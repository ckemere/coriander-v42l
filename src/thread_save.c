/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *
 * PVN saving capability by Jacob (Jack) Gryn and Konstantinos G. Derpanis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "coriander.h"

gint
SaveStartThread(camera_t* cam)
{
  chain_t* save_service=NULL;
  savethread_info_t *info=NULL;

  save_service=GetService(cam, SERVICE_SAVE);

  if (save_service==NULL) { // if no SAVE service running...
    save_service=(chain_t*)malloc(sizeof(chain_t));
    save_service->current_buffer=NULL;
    save_service->next_buffer=NULL;
    save_service->data=(void*)malloc(sizeof(savethread_info_t));
    info=(savethread_info_t*)save_service->data;
    pthread_mutex_init(&save_service->mutex_data, NULL);
    pthread_mutex_init(&save_service->mutex_struct, NULL);
    pthread_mutex_init(&info->mutex_cancel, NULL);
    
    /* if you want a clean-interrupt thread:*/
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=0;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    /* setup save_thread: handles, ...*/
    pthread_mutex_lock(&save_service->mutex_data);
    CommonChainSetup(cam, save_service,SERVICE_SAVE);
    
    info->frame.image=NULL;
    info->frame.allocated_image_bytes=0;
    info->bigbuffer=NULL;

    if ((cam->prefs.use_ram_buffer==TRUE)&&
	((cam->prefs.save_format==SAVE_FORMAT_RAW_VIDEO)||
#ifdef HAVE_FFMPEG
	 (cam->prefs.save_format==SAVE_FORMAT_MPEG)||
#endif
	 (cam->prefs.save_format==SAVE_FORMAT_PVN))) {
      info->bigbuffer_position=0;
      info->bigbuffer=(unsigned char*)malloc(cam->prefs.ram_buffer_size*1024*1024*sizeof(unsigned char));
      if (info->bigbuffer==NULL) {
        Error("Could not allocate memory for RAM buffer save service");
        pthread_mutex_unlock(&save_service->mutex_data);
        FreeChain(save_service);
        save_service=NULL;
        return(-1);
      }
    }

    /* Insert chain and start service*/
    pthread_mutex_lock(&save_service->mutex_struct);
    InsertChain(cam, save_service);
    if (pthread_create(&save_service->thread, NULL, SaveThread,(void*) save_service)) {
      /* error starting thread. You should cleanup here
	 (free, unset global vars,...):*/
      
      /* Mendatory cleanups:*/
      RemoveChain(cam,save_service);
      pthread_mutex_unlock(&save_service->mutex_struct);
      pthread_mutex_unlock(&save_service->mutex_data);
      if ((info->frame.image!=NULL)&&(info->frame.allocated_image_bytes>0)) {
        free(info->frame.image);
        info->frame.allocated_image_bytes=0;
      }
      FreeChain(save_service);
      save_service=NULL;
      return(-1);
    }

    pthread_mutex_unlock(&save_service->mutex_struct);
    pthread_mutex_unlock(&save_service->mutex_data);
    
  }
  
  return (1);
}


void
ProtectFilename(char *filename)
{
  if (preferences.no_overwrite>0) {
    char temp[1024];
    struct stat buf;
    // try to read the file
    int retry=1;
    strcpy(temp,filename);
    while (stat(temp, &buf)==0) {
      // file exists, append -1
      sprintf(temp,"%s-%d",filename,retry);
      retry++;
    }
    strcpy(filename,temp);
  }
}


// return TRUE if format is not in PVN native RGB or GREYSCALE mode
int
needsConversionForPVN(int color_mode)
{
  switch(color_mode) {
  case DC1394_COLOR_CODING_YUV411:
  case DC1394_COLOR_CODING_YUV422:
  case DC1394_COLOR_CODING_YUV444:
    return(TRUE);
    break;
  case DC1394_COLOR_CODING_MONO8:
  case DC1394_COLOR_CODING_RGB8:
  case DC1394_COLOR_CODING_MONO16:
  case DC1394_COLOR_CODING_MONO16S:
  case DC1394_COLOR_CODING_RGB16:
  case DC1394_COLOR_CODING_RAW8:
  case DC1394_COLOR_CODING_RAW16:
  case DC1394_COLOR_CODING_RGB16S:
    return(FALSE);
    break;
  default:
    fprintf(stderr, "Unknown buffer format!\n");
    return(FALSE);
    break;
  }
  return(FALSE);
}

// return Bytes in each channel after converted to RGB or MONO (YUV422 will convert to RGB8 thats why we need this extra function)
int
getConvertedBytesPerChannel(int color_mode)
{
  switch(color_mode) {
  case DC1394_COLOR_CODING_MONO8:
  case DC1394_COLOR_CODING_RAW8:
  case DC1394_COLOR_CODING_YUV411:
  case DC1394_COLOR_CODING_YUV422:
  case DC1394_COLOR_CODING_YUV444:
  case DC1394_COLOR_CODING_RGB8:
    return(1);
    break;
  case DC1394_COLOR_CODING_MONO16:
  case DC1394_COLOR_CODING_RAW16:
  case DC1394_COLOR_CODING_MONO16S:
  case DC1394_COLOR_CODING_RGB16:
    return(2);
    break;
  default:
    fprintf(stderr, "Unknown buffer format!\n");
    return(0);
    break;
  }
  return(0);
}

unsigned int
getDepth(unsigned long bufsize, int mode, unsigned int height, unsigned int width)
{
  unsigned int bits_per_pixel;
  dc1394_get_color_coding_bit_size(mode, &bits_per_pixel);
  return 8*bufsize/(bits_per_pixel*height*width);
}

// (JG) note: depth/# of pages is calculated from bufsize by dividing by # of bytes per image
void
writePVNHeader(FILE *fd, unsigned int mode, unsigned int width, unsigned int height,
	       unsigned int depth, unsigned int bpp, double framerate)
{
  char magic[6];
  unsigned int is_color;

  dc1394_is_color(mode,&is_color);

  if(is_color==FALSE) {//greyscale
    if(mode == DC1394_COLOR_CODING_MONO16S)
      strcpy(magic,"PV5b");
    else
      strcpy(magic,"PV5a");
  }
  else // colour
    strcpy(magic,"PV6a");

  fprintf(fd,"%s\n%d %d %d\n%d %f\n", magic, width, height, depth, bpp, framerate);
}

static gint
CreateSettingsFile(camera_t *cam, char *destdir, dc1394framerate_t framerate)
{
  char *fname = NULL;
  FILE *fd = NULL;
  int i;
  extern Prefs_t preferences;
  
  fname = (char*)malloc(STRING_SIZE*sizeof(char));
  sprintf(fname,"%s/camera_setup.txt",destdir);

  ProtectFilename(fname);
  fd = fopen(fname,"w");
  if (fd==NULL) {
    Error("Cannot create settings file");
    return(0);
  }
  
  fprintf(fd,"fps=%s\n", fps_label_list[framerate-DC1394_FRAMERATE_MIN]);
  fprintf(fd,"sync_control=%d\n",cam->prefs.broadcast);
  
  for(i=DC1394_FEATURE_MIN; i<=DC1394_FEATURE_MAX; ++i) {
    if (cam->feature_set.feature[i-DC1394_FEATURE_MIN].available)
      fprintf(fd,"%s=%d\n", feature_name_list[i-DC1394_FEATURE_MIN],
	      cam->feature_set.feature[i-DC1394_FEATURE_MIN].value);
  }

  fclose(fd);
  return(1);
}


int
GetSaveFD(chain_t *save_service, FILE **fd, char *filename_out)
{
  savethread_info_t *info=save_service->data;
  camera_t *cam=save_service->camera;

  if (cam->prefs.save_to_stdout>0) {
    *fd=stdout;
    return DC1394_SUCCESS;
  }

  // get filename
  switch (cam->prefs.save_format) {
  case SAVE_FORMAT_PPM:
  case SAVE_FORMAT_RAW:
#ifdef HAVE_FFMPEG
  case SAVE_FORMAT_JPEG:
#endif
    // first handle the case of save-to-dir
    if (cam->prefs.save_to_dir==0) {
      switch (cam->prefs.save_append) {
      case SAVE_APPEND_NONE:
	sprintf(filename_out, "%s.%s", cam->prefs.save_filename, cam->prefs.save_filename_ext);
	break;
      case SAVE_APPEND_DATE_TIME:
	sprintf(filename_out, "%s-%s.%s", cam->prefs.save_filename, save_service->current_buffer->captime_string, cam->prefs.save_filename_ext);
	break;
      case SAVE_APPEND_NUMBER:
	sprintf(filename_out,"%s-%10.10lli.%s", cam->prefs.save_filename, save_service->processed_frames, cam->prefs.save_filename_ext);
	break;
      }
    }
    else { // we save to a directory...
      // 1. create the directory and write a setup file if it's the first frame
      if (save_service->processed_frames==0) {
	// note that we append a time tag to allow safe re-launch of the thread (it will prevent overwriting
	// previous results)
	sprintf(info->destdir,"%s-%s",cam->prefs.save_filename, save_service->current_buffer->captime_string);

	// Optional: get rid of "-mmm" ms 
	//if (strlen(destdir) > 4)
	//destdir[strlen(destdir)-4]=0;
	
	if (mkdir(info->destdir,0755)) {
	  Error("Could not create directory");
	  return DC1394_FAILURE;
	}
	// Create a file with camera settings
	dc1394framerate_t framerate;
	dc1394_video_get_framerate(cam->camera_info,&framerate);
	CreateSettingsFile(cam,info->destdir, framerate);
      }

      // 2. build the filename
      switch (cam->prefs.save_append) {
      case SAVE_APPEND_NONE: 
	fprintf(stderr,"time or number should have been selected\n");
	return DC1394_FAILURE;
	break;
      case SAVE_APPEND_DATE_TIME:
	sprintf(filename_out, "%s/%s.%s", info->destdir, save_service->current_buffer->captime_string, cam->prefs.save_filename_ext);
	break;
      case SAVE_APPEND_NUMBER:
	sprintf(filename_out,"%s/%10.10lli.%s", info->destdir, save_service->processed_frames, cam->prefs.save_filename_ext);
	break;
      }
      // 3. done!
    }
    break;
  case SAVE_FORMAT_RAW_VIDEO:
  case SAVE_FORMAT_PVN:
#ifdef HAVE_FFMPEG
  case SAVE_FORMAT_MPEG:
#endif
    switch (cam->prefs.save_append) {
    case SAVE_APPEND_NONE:
      sprintf(filename_out, "%s.%s", cam->prefs.save_filename, cam->prefs.save_filename_ext);
      break;
    case SAVE_APPEND_DATE_TIME:
      sprintf(filename_out, "%s-%s.%s", cam->prefs.save_filename, save_service->current_buffer->captime_string, cam->prefs.save_filename_ext);
      break;
    case SAVE_APPEND_NUMBER:
      sprintf(filename_out,"%s-%10.10lli.%s", cam->prefs.save_filename, save_service->processed_frames, cam->prefs.save_filename_ext);
      break;
    }
    break;
  default:
    fprintf(stderr,"unsupported format!\n");
    break;
  }

  // open FD if it's the first frame of a video
  // OR it's in picture saving mode AND it's not using imlib (which creates the fd from the filename itself)
  switch (cam->prefs.save_format) {
  case SAVE_FORMAT_RAW_VIDEO:
  case SAVE_FORMAT_PVN:
    if (save_service->processed_frames==0) {
      ProtectFilename(filename_out);
      *fd=fopen(filename_out,"w");
      if (*fd==NULL) {
	fprintf(stderr,"Can't create sequence file for saving\n");
	return DC1394_FAILURE;
      }
    }
    break;
  case SAVE_FORMAT_PPM:
  case SAVE_FORMAT_RAW:
    ProtectFilename(filename_out);
    *fd=fopen(filename_out,"w");
    if (*fd==NULL) {
      fprintf(stderr,"Can't create sequence file for saving\n");
      return DC1394_FAILURE;
    }
    break;
#ifdef HAVE_FFMPEG
  case SAVE_FORMAT_MPEG:
  case SAVE_FORMAT_JPEG:
    // do nothing
    break;
#endif
  default:
    fprintf(stderr,"unsupported format!\n");
    break;
  }

  return DC1394_SUCCESS;
}


int
InitVideoFile(chain_t *save_service, FILE *fd, char *filename_out)
{
  savethread_info_t *info;
  info=(savethread_info_t*)save_service->data;
  float fps;
  camera_t *cam=save_service->camera;

  dc1394framerate_t framerate;
  dc1394_video_get_framerate(cam->camera_info,&framerate);

#ifdef HAVE_FFMPEG
  info->fmt = NULL;
  info->oc = NULL;
  info->codec = NULL;
  info->video_st = NULL;
  info->picture = NULL;
  info->tmp_picture = NULL;
  
  info->fdts = 0;
#endif

  // (JG) if extension is PVN, write PVN header here
  if ((cam->prefs.save_format==SAVE_FORMAT_PVN) && (cam->prefs.use_ram_buffer==FALSE)) {//-----------------------------------
    //fprintf(stderr,"pvn header write\n");
    dc1394_framerate_as_float(framerate, &fps);
    writePVNHeader(fd, save_service->current_buffer->frame.color_coding,
		   save_service->current_buffer->frame.size[0],
		   save_service->current_buffer->frame.size[1],
		   0, getConvertedBytesPerChannel(save_service->current_buffer->frame.color_coding)*8,
		   fps);
  }

#ifdef HAVE_FFMPEG
  if ((cam->prefs.save_format==SAVE_FORMAT_MPEG) && (cam->prefs.use_ram_buffer==FALSE)) {//-----------------------------------
    // MPEG
    /* auto detect the output format from the name. default is mp4/mpeg1. */
    info->fmt = av_guess_format(NULL, filename_out, NULL);
    if (!info->fmt) {
      fprintf(stderr,"Could not deduce output format from file extension: using MP4.\n");
      info->fmt = av_guess_format("mp4", NULL, NULL);
    }
    info->fmt->video_codec = CODEC_ID_MPEG2VIDEO; // force MPEG vidoe otherwise, default for mp4 is h264
    
    if (!info->fmt) {
      fprintf(stderr, "Could not find suitable output format\n");
      return DC1394_FAILURE;
    }

    /* allocate the output media context */
    info->oc = avformat_alloc_context();
    if (!info->oc) {
      fprintf(stderr, "Memory error\n");
    }
    info->oc->oformat = info->fmt;
    snprintf(info->oc->filename, sizeof(info->oc->filename), "%s", filename_out);
    
    /* add the video stream to the container and initialize the codecs */
    info->video_st = NULL;
    if (info->fmt->video_codec != CODEC_ID_NONE) {
      info->video_st = add_video_stream(info->oc, info->fmt->video_codec, 
					save_service->current_buffer->frame.size[0],
					save_service->current_buffer->frame.size[1]);
    }
    
    /* set the output parameters (must be done even if no
       parameters). */
    if (av_set_parameters(info->oc, NULL) < 0) {
      fprintf(stderr, "Invalid output format parameters\n");
    }
    
    av_dump_format(info->oc, 0, filename_out, 1);
    
    /* now that all the parameters are set, we can open the
       video codec and allocate the necessary encode buffers */
    if (info->video_st)
      open_video(info->oc, info->video_st);

    info->picture = alloc_pframe(info->video_st->codec->pix_fmt, 
        info->video_st->codec->width, info->video_st->codec->height);

    if (info->picture == NULL)
      fprintf(stderr, "Could not allocate picture memory\n");
    
    /* open the output file, if needed */
    if (!(info->fmt->flags & AVFMT_NOFILE)) {
      ProtectFilename(filename_out);
      if (url_fopen(&info->oc->pb, filename_out, URL_WRONLY) < 0) {
        fprintf(stderr, "Could not open '%s'\n", filename_out);
      }
    }
  
    /* write the stream header, if any */
    avformat_write_header(info->oc, NULL); 
    
    strcpy(info->subtitle, filename_out);
    strcpy(strrchr(info->subtitle,'.'), ".sub");
    fprintf(stderr, "Recording frame timestamps to: %s\n", info->subtitle);
    info->fdts = open(info->subtitle, O_CREAT | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);


    subtitle_header(info->subtitle);
    write(info->fdts, info->subtitle, strlen(info->subtitle));

  }
#endif    // END MPEG

  // other inits for other video formats come here...          ----------------------------------
  // ...
  

  return DC1394_SUCCESS;
}

void
FillRamBuffer(chain_t *save_service)
{
  savethread_info_t *info;

  info=(savethread_info_t*)save_service->data;
  camera_t *cam=save_service->camera;


  if ((cam->prefs.use_ram_buffer==TRUE)&&
      ((cam->prefs.save_format==SAVE_FORMAT_RAW_VIDEO)||
#ifdef HAVE_FFMPEG
       (cam->prefs.save_format==SAVE_FORMAT_MPEG)||
#endif
       (cam->prefs.save_format==SAVE_FORMAT_PVN))) {
    if (cam->prefs.ram_buffer_size*1024*1024-info->bigbuffer_position>=save_service->current_buffer->frame.image_bytes) {
      memcpy(&info->bigbuffer[info->bigbuffer_position], save_service->current_buffer->frame.image, save_service->current_buffer->frame.image_bytes);
      info->bigbuffer_position+=save_service->current_buffer->frame.image_bytes;
    }
    else { // buffer is full, exit thread
      info->cancel_req=1;
    }
  }
}

void
SavePPM(chain_t *save_service, FILE *fd)
{
  unsigned int maxlevels, P_value;
  long long unsigned int bytes;
  savethread_info_t *info;
  unsigned char *src;
  info=(savethread_info_t*)save_service->data;
  
  switch (save_service->current_buffer->frame.color_coding) {
  case DC1394_COLOR_CODING_MONO8:
  case DC1394_COLOR_CODING_RAW8:
    P_value=5;
    bytes=save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1];
    maxlevels=255;
    src=save_service->current_buffer->frame.image;
    break;
  case DC1394_COLOR_CODING_MONO16:
  case DC1394_COLOR_CODING_RAW16:
  case DC1394_COLOR_CODING_MONO16S:
    P_value=5;
    bytes=save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1]*2;
    maxlevels=65535;
    src=save_service->current_buffer->frame.image;
    break;
  case DC1394_COLOR_CODING_YUV411:
  case DC1394_COLOR_CODING_YUV422:
  case DC1394_COLOR_CODING_YUV444:
    convert_to_rgb(&save_service->current_buffer->frame, &info->frame);
    P_value=6;
    bytes=save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1]*3;
    maxlevels=255;
    src=info->frame.image;
    break;
  case DC1394_COLOR_CODING_RGB8:
    P_value=6;
    bytes=save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1]*3;
    maxlevels=255;
    src=save_service->current_buffer->frame.image;
    break;
  case DC1394_COLOR_CODING_RGB16:
    P_value=6;
    bytes=save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1]*6;
    maxlevels=65535;
    src=save_service->current_buffer->frame.image;
    break;
  default:
    fprintf(stderr, "Unknown buffer format!\n");
    return;
  }
  fprintf(fd,"P%u\n#Created by Coriander\n%d %d\n%u\n",P_value,save_service->current_buffer->frame.size[0],save_service->current_buffer->frame.size[1],maxlevels);
  fwrite(src, bytes, 1, fd);
}

#ifdef HAVE_FFMPEG
void
SaveMPEGFrame(chain_t *save_service)
{
  savethread_info_t *info;
  //unsigned char *tmp_buf;
  int err=0;
  static int sws_flags = SWS_FAST_BILINEAR | SWS_CPU_CAPS_MMX2;
  // static int sws_flags = SWS_BICUBIC;
  static struct SwsContext *sws_ctx;
  int linesize[4];
  info=(savethread_info_t*)save_service->data;

  int i;

  AVCodecContext *c = info->video_st->codec;
  static dc1394color_coding_t color_coding = DC1394_COLOR_CODING_MONO8; // initial

  AVPicture *tmp_picture;

  if ((!sws_ctx) || (save_service->current_buffer->frame.color_coding != color_coding)) {
    fprintf(stderr, "Setting color coding\n");
    color_coding = save_service->current_buffer->frame.color_coding;
    info->mpeg_color_mode=0;
    switch(color_coding) {
    case DC1394_COLOR_CODING_MONO8:
    case DC1394_COLOR_CODING_RAW8:
      info->mpeg_color_mode=PIX_FMT_GRAY8;
      fprintf(stderr, "This format is not supported for MPEG save\n");
      err=1;
      break;
    case DC1394_COLOR_CODING_YUV411:
      info->mpeg_color_mode=PIX_FMT_UYYVYY411;
      //info->mpeg_color_mode=PIX_FMT_YUV411P;
      break;
    case DC1394_COLOR_CODING_YUV422:
      info->mpeg_color_mode=PIX_FMT_UYVY422;
      //info->mpeg_color_mode=PIX_FMT_YUV422P;
      break;
    case DC1394_COLOR_CODING_YUV444:
      info->mpeg_color_mode=PIX_FMT_YUV444P;
      fprintf(stderr, "This format is not supported for MPEG save\n");
      err=1;
      break;
    case DC1394_COLOR_CODING_RGB8:
      info->mpeg_color_mode=PIX_FMT_RGB24;
      break;
    default:
      fprintf(stderr, "This format is not supported for MPEG save\n");
      err=1;
      break;
    }

    if (info->tmp_picture != NULL) {
      av_free(info->tmp_picture);
      info->tmp_picture=NULL;
    }
    info->tmp_picture = avcodec_alloc_frame();
    sws_ctx = sws_getContext(c->width, c->height, info->mpeg_color_mode,
                           c->width, c->height, c->pix_fmt,
                           sws_flags, NULL, NULL, NULL);
  }

  switch(color_coding) {
  case DC1394_COLOR_CODING_YUV411:
    // uyvy411_yuv411p(save_service->current_buffer->frame.image, info->tmp_picture, 
		    // save_service->current_buffer->frame.size[0], save_service->current_buffer->frame.size[1]);
    avpicture_fill(info->tmp_picture, save_service->current_buffer->frame.image,
        PIX_FMT_UYYVYY411, c->width, c->height);
    sws_scale(sws_ctx,
        (const uint8_t * const *)info->tmp_picture->data, info->tmp_picture->linesize,
        0, c->height, info->picture->data, info->picture->linesize);
    break;
  case DC1394_COLOR_CODING_YUV422:
    // uyvy422_yuv422p(save_service->current_buffer->frame.image, info->tmp_picture, 
		    // save_service->current_buffer->frame.size[0], save_service->current_buffer->frame.size[1]);
    avpicture_fill(info->tmp_picture, save_service->current_buffer->frame.image,
        PIX_FMT_UYVY422, c->width, c->height);
    sws_scale(sws_ctx,
        (const uint8_t * const *)info->tmp_picture->data, info->tmp_picture->linesize,
        0, c->height, info->picture->data, info->picture->linesize);
    break;
  case DC1394_COLOR_CODING_RGB8:
    avpicture_fill(info->tmp_picture, save_service->current_buffer->frame.image,
        PIX_FMT_RGB24, c->width, c->height);
    sws_scale(sws_ctx,
        info->tmp_picture->data, info->tmp_picture->linesize,
        0, c->height, info->picture->data, info->picture->linesize);
        // save_service->current_buffer->frame.image, info->tmp_picture->linesize,
        // 0, c->height, info->picture->data, info->picture->linesize);
    break;
  default:
    fprintf(stderr,"unsupported color format!!\n");
    err=1;
    break;
  }

  if (err==0) {
    write_video_frame(info->oc, info->video_st, info->picture);
    
    /* Save time stamp */
    write(info->fdts, &(save_service->current_buffer->frame_sec), sizeof(uint64_t));
    write(info->fdts, &(save_service->current_buffer->frame_nsec), sizeof(uint64_t));
    //new_subtitle(save_service->processed_frames, save_service->fps,
		 //save_service->current_buffer->captime_string, info->subtitle);
    //printf("%s", info->subtitle);
    //write(info->fdts, info->subtitle, strlen(info->subtitle));
  }
}

void
SaveJPEGFrame(chain_t *save_service, char *filename_out)
{
  savethread_info_t *info;
  static dc1394color_coding_t color_coding = DC1394_COLOR_CODING_MONO8; // initial
  info=(savethread_info_t*)save_service->data;

  static int sws_flags = SWS_BICUBIC;
  static struct SwsContext *sws_ctx;
  info=(savethread_info_t*)save_service->data;

  int width = save_service->current_buffer->frame.size[0];
  int height = save_service->current_buffer->frame.size[1];

  AVPicture tmp_picture;

  info->picture = alloc_pframe(PIX_FMT_YUVJ420P, width, height);
  if (info->picture == NULL) {
    fprintf(stderr, "Could not allocate picture\n");
  }

  if ((!sws_ctx) || (save_service->current_buffer->frame.color_coding != color_coding)) {
    color_coding = save_service->current_buffer->frame.color_coding;
    info->mpeg_color_mode=0;
    switch(color_coding) {
    case DC1394_COLOR_CODING_MONO8:
    case DC1394_COLOR_CODING_RAW8:
      info->mpeg_color_mode=PIX_FMT_GRAY8;
      fprintf(stderr, "This format is not supported for MPEG save\n");
      break;
    case DC1394_COLOR_CODING_YUV411:
      info->mpeg_color_mode=PIX_FMT_UYYVYY411;
      //info->mpeg_color_mode=PIX_FMT_YUV411P;
      break;
    case DC1394_COLOR_CODING_YUV422:
      info->mpeg_color_mode=PIX_FMT_UYVY422;
      //info->mpeg_color_mode=PIX_FMT_YUV422P;
      break;
    case DC1394_COLOR_CODING_YUV444:
      info->mpeg_color_mode=PIX_FMT_YUV444P;
      fprintf(stderr, "This format is not supported for MPEG save\n");
      break;
    case DC1394_COLOR_CODING_RGB8:
      info->mpeg_color_mode=PIX_FMT_RGB24;
      break;
    default:
      fprintf(stderr, "This format is not supported for MPEG save\n");
      break;
    }
    fprintf(stderr, "Set colorformat to %d\n",info->mpeg_color_mode);

    if (info->tmp_picture != NULL) {
      av_free(info->tmp_picture);
      info->tmp_picture=NULL;
    }
    info->tmp_picture = avcodec_alloc_frame();
    sws_ctx = sws_getContext(width, height, info->mpeg_color_mode,
                           width, height, PIX_FMT_YUVJ420P,
                           sws_flags, NULL, NULL, NULL);
  }

  switch(color_coding) {
  case DC1394_COLOR_CODING_YUV411:
    avpicture_fill(info->tmp_picture, save_service->current_buffer->frame.image,
        PIX_FMT_UYYVYY411, width, height);
    sws_scale(sws_ctx,
        (const uint8_t * const *)info->tmp_picture->data, info->tmp_picture->linesize,
        0, height, info->picture->data, info->picture->linesize);
    jpeg_write(info->picture, width, height,
	       PIX_FMT_YUV420P, filename_out, 90.0, "Created using Coriander and FFMPEG");
    break;
  case DC1394_COLOR_CODING_YUV422:
    avpicture_fill(info->tmp_picture, save_service->current_buffer->frame.image,
        PIX_FMT_UYVY422, width, height);
    sws_scale(sws_ctx,
        (const uint8_t * const *)info->tmp_picture->data, info->tmp_picture->linesize,
        0, height, info->picture->data, info->picture->linesize);
    jpeg_write(info->picture, width, height,
	       PIX_FMT_YUV420P, filename_out, 90.0, "Created using Coriander and FFMPEG");
    break;
  case DC1394_COLOR_CODING_RGB8:
    avpicture_fill(info->tmp_picture, save_service->current_buffer->frame.image,
        PIX_FMT_RGB24, width, height);
    sws_scale(sws_ctx,
        info->tmp_picture->data, info->tmp_picture->linesize,
        0, height, info->picture->data, info->picture->linesize);
    jpeg_write(info->picture, width, height,
	       PIX_FMT_YUV420P, filename_out, 90.0, "Created using Coriander and FFMPEG");
    break;
  default:
    fprintf(stderr,"unsupported format: jpeg only works with RGB24, YUV411, YUV422 at this time!\n");
    break;
  }


  if (info->picture) {
    av_free(info->picture->data[0]);
    av_free(info->picture);
    info->picture=NULL;
  }
  if (info->tmp_picture) {
    av_free(info->tmp_picture);
    info->tmp_picture=NULL;
  }
}
#endif


void*
SaveThread(void* arg)
{
  char *filename_out;
  chain_t* save_service=NULL;
  savethread_info_t *info=NULL;
  //GdkImlibImage *im=NULL; // V20***
  long int skip_counter;
  FILE *fd=NULL;
  float tmp, fps;
  int i;
  unsigned char* tmp_buf=NULL;

  filename_out=(char*)malloc(STRING_SIZE*sizeof(char));

  save_service=(chain_t*)arg;
  pthread_mutex_lock(&save_service->mutex_data);
  info=(savethread_info_t*)save_service->data;
  skip_counter=0;
  camera_t *cam=save_service->camera;


  /* These settings depend on the thread. For 100% safe deferred-cancel
   threads, I advise you use a custom thread cancel flag. See display thread.*/
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&save_service->mutex_data);

  // time inits:
  save_service->prev_time = 0;
  save_service->prev_period = 0;
  save_service->drop_warning = 0;
  save_service->processed_frames=0;

  while (1) { 
    /* Clean cancel handlers */
    pthread_mutex_lock(&info->mutex_cancel);
    if (info->cancel_req>0) {
      //fprintf(stderr,"Cancel request found, breaking...\n");
      break;
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel);
      pthread_mutex_lock(&save_service->mutex_data);

      //fprintf(stderr,"About to roll buffers in SAVE thread\n");
      if(GetBufferFromPrevious(save_service)) { // have buffers been rolled?
        // check params
        //printf("New frame arrived\n");
        SaveThreadCheckParams(save_service);
        if (save_service->current_buffer->frame.size[0]!=-1) {
          if (skip_counter>=(cam->prefs.save_period-1)) {
            skip_counter=0;

            // get file descriptor
            if (GetSaveFD(save_service, &fd, filename_out)!=DC1394_SUCCESS)
              break;

#ifdef HAVE_FFMPEG
            if ((save_service->processed_frames==0)&&
          ((cam->prefs.save_format==SAVE_FORMAT_MPEG)||
           (cam->prefs.save_format==SAVE_FORMAT_JPEG))) {
              av_register_all();
            }
#endif

            // write initial data for video (header,...)
            if ((save_service->processed_frames==0)&&
          ((cam->prefs.save_format==SAVE_FORMAT_RAW_VIDEO)||
#ifdef HAVE_FFMPEG
           (cam->prefs.save_format==SAVE_FORMAT_MPEG)||
#endif
           (cam->prefs.save_format==SAVE_FORMAT_PVN))) {
              if (InitVideoFile(save_service, fd, filename_out)!=DC1394_SUCCESS) {
          break;
              }
            }

            // rambuffer operation
            if ((cam->prefs.use_ram_buffer==TRUE)&&
          ((cam->prefs.save_format==SAVE_FORMAT_RAW_VIDEO)||
#ifdef HAVE_FFMPEG
           (cam->prefs.save_format==SAVE_FORMAT_MPEG)||
#endif
           (cam->prefs.save_format==SAVE_FORMAT_PVN))) {
              FillRamBuffer(save_service);
            }
            else { // normal operation (no RAM buffer)
              switch (cam->prefs.save_format) {
              case SAVE_FORMAT_RAW:
                //fprintf(stderr,"writing raw...");
                fwrite(save_service->current_buffer->frame.image, save_service->current_buffer->frame.image_bytes, 1, fd);
                //fprintf(stderr,"done. closing fd...");
                fclose(fd);
                fd=NULL;
                //fprintf(stderr,"done\n");
                break;
              case SAVE_FORMAT_RAW_VIDEO:
                fwrite(save_service->current_buffer->frame.image, save_service->current_buffer->frame.image_bytes, 1, fd);
                break;
              case SAVE_FORMAT_PVN:
                if (needsConversionForPVN(save_service->current_buffer->frame.color_coding)>0) {
                  // we assume that if it needs conversion, the output of the conversion is an 8bpp RGB
                  tmp_buf = (unsigned char*)malloc(3*save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1]*sizeof(unsigned char));
                  convert_for_pvn(save_service->current_buffer->frame.image, save_service->current_buffer->frame.size[0],
                      save_service->current_buffer->frame.size[1], 0, save_service->current_buffer->frame.color_coding, tmp_buf);
                  fwrite(tmp_buf, 3*save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1], 1, fd);
                  free(tmp_buf);
                  tmp_buf=NULL;
                }
                else {
                  // no conversion, we can dump the data
                  fwrite(save_service->current_buffer->frame.image, save_service->current_buffer->frame.image_bytes, 1, fd);
                }
                break;
#ifdef HAVE_FFMPEG
              case SAVE_FORMAT_MPEG:
                // video saving mode
                // fprintf(stderr,"entering MPEG save and convert section\n");
                SaveMPEGFrame(save_service);
                break;
                    case SAVE_FORMAT_JPEG:
                /* Save JPEG file using FFMPEG... Much, much faster... There is no need for YUV->RGB color space conversions */
                SaveJPEGFrame(save_service, filename_out);
                break;
#endif
              case SAVE_FORMAT_PPM:
                SavePPM(save_service,fd);
                fclose(fd);
                fd=NULL;
                break;
              default:
                fprintf(stderr,"Unsupported file format\n");
              } // end save format switch
            } // end ram buffer if
            save_service->processed_frames++;
          }
          else
            skip_counter++;
          
          // FPS computation:
          tmp=((float)(save_service->current_buffer->frame.timestamp-save_service->prev_time))/1000000.0;
          if (save_service->prev_time==0) {
            save_service->fps=fabs(0.0);
          }
          else {
            if (tmp==0)
              save_service->fps=fabs(0.0);
            else
              save_service->fps=fabs(1/tmp);
          }
          if (save_service->prev_time!=0) {
            save_service->prev_period=tmp;
          }
          // produce a drop warning if the period difference is over 50%
          if (save_service->prev_period!=0) {
            if (fabs(save_service->prev_period-tmp)/(save_service->prev_period/2+tmp/2)>=.5)
              save_service->drop_warning++;
          }
          save_service->prev_time=save_service->current_buffer->frame.timestamp;
        }
        //usleep(200000);////////////////////////////////////////////
        PublishBufferForNext(save_service);
        pthread_mutex_unlock(&save_service->mutex_data);
        //fprintf(stderr,"saved frame %.7f\n",save_service->fps);
      }
      else {
        //fprintf(stderr,"No new frame, unlocking mutex\n");
        pthread_mutex_unlock(&save_service->mutex_data);
      }
      //fprintf(stderr,"??");
    }
    //fprintf(stderr,"??");
    usleep(100);
  }

  // we now have to close the video file properly and handle ram buffer operation

  //fprintf(stderr,"Break completed\n");

  dc1394framerate_t framerate;
  if (cam->prefs.use_ram_buffer==TRUE) {
    switch(cam->prefs.save_format) {
    case SAVE_FORMAT_RAW_VIDEO:
#ifdef HAVE_FFMPEG
    case SAVE_FORMAT_MPEG:
      fwrite(info->bigbuffer, info->bigbuffer_position, 1, fd);
      break;
#endif
    case SAVE_FORMAT_PVN:
      dc1394_video_get_framerate(cam->camera_info,&framerate);
      dc1394_framerate_as_float(framerate, &fps);
      writePVNHeader(fd, save_service->current_buffer->frame.color_coding,
		     save_service->current_buffer->frame.size[0],
		     save_service->current_buffer->frame.size[1],
		     getDepth(info->bigbuffer_position, save_service->current_buffer->frame.color_coding, 
			      save_service->current_buffer->frame.size[1], save_service->current_buffer->frame.size[0]),
		     getConvertedBytesPerChannel(save_service->current_buffer->frame.color_coding)*8,
		     fps);
      
      if(needsConversionForPVN(save_service->current_buffer->frame.color_coding)==FALSE) {
	fwrite(info->bigbuffer, info->bigbuffer_position, 1, fd);
      }
      else {
	// we assume that if it needs conversion, the output of the conversion is an 8bpp RGB
	tmp_buf = (unsigned char*)malloc(3*save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1]*sizeof(unsigned char));
	
	for (i = 0; i < getDepth(info->bigbuffer_position, save_service->current_buffer->frame.color_coding,
				 save_service->current_buffer->frame.size[1], save_service->current_buffer->frame.size[0]); i++) {
	  convert_for_pvn(info->bigbuffer, save_service->current_buffer->frame.size[0],
			  save_service->current_buffer->frame.size[1], i, save_service->current_buffer->frame.color_coding, tmp_buf);
	  fwrite(tmp_buf, 3*save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1], 1, fd);
	}
	free(tmp_buf);
	tmp_buf=NULL;
      }
      break;
    default:
      fprintf(stderr,"Error: Ram buffer should be accessible only in video mode\n");
      break;
    }
  }
  //fprintf(stderr,"handling FD\n");
  
  if (fd!=NULL) {
    fclose(fd);
    fd=NULL;
  }
#ifdef HAVE_FFMPEG
  if (cam->prefs.save_format==SAVE_FORMAT_MPEG) {
    //video_encode_finish();
    /* write the trailer, if any */
    av_write_trailer(info->oc);

    /* close each codec */
    if (info->video_st)
      close_video(info->oc, info->video_st, info->picture);
    if (info->tmp_picture) {
      av_free(info->tmp_picture);
      info->tmp_picture=NULL;
    }

    if (!(info->fmt->flags & AVFMT_NOFILE)) {
      /* close the output file */
      url_fclose(info->oc->pb);
    }
    /* free the stream and context */
    avformat_free_context(info->oc);
    info->oc=NULL;

    close(info->fdts);

  }
#endif

  if (info->bigbuffer!=NULL) {
    free(info->bigbuffer);
    info->bigbuffer=NULL;
  }

  pthread_mutex_unlock(&info->mutex_cancel);

  free(filename_out);
  filename_out=NULL;

  return ((void*)1);
}


gint
SaveStopThread(camera_t* cam)
{
  savethread_info_t *info;
  chain_t *save_service;
  save_service=GetService(cam,SERVICE_SAVE);

  if (save_service!=NULL) { // if SAVE service running...
    info=(savethread_info_t*)save_service->data;
    /* Clean cancel handler: */
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=1;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    /* common handlers...*/
    pthread_join(save_service->thread, NULL);
    
    pthread_mutex_lock(&save_service->mutex_data);
    pthread_mutex_lock(&save_service->mutex_struct);
    
    RemoveChain(cam,save_service);
    /* Do custom cleanups here...*/
    if ((info->frame.image!=NULL)&&(info->frame.allocated_image_bytes>0)) {
      free(info->frame.image);
      info->frame.allocated_image_bytes=0;
    }
    
    /* Mendatory cleanups: */
    pthread_mutex_unlock(&save_service->mutex_struct);
    pthread_mutex_unlock(&save_service->mutex_data);
    FreeChain(save_service);
    save_service=NULL;
    
  }
  
  return (1);
}

void
SaveThreadCheckParams(chain_t *save_service)
{

  savethread_info_t *info;
  info=(savethread_info_t*)save_service->data;

  // THIS IS ALL AUTOMATIC NOW!!
  /*
  // if some parameters changed, we need to re-allocate the local buffers and restart the save
  if ((save_service->current_buffer->frame.size[0]!=save_service->local_param_copy.frame.size[0])||
      (save_service->current_buffer->frame.size[1]!=save_service->local_param_copy.frame.size[1])  ) {
    
    // DO SOMETHING
    if (info->buffer!=NULL) {
      free(info->buffer);
      info->buffer=NULL;
    }
    info->buffer=(unsigned char*)malloc(save_service->current_buffer->frame.size[0]*save_service->current_buffer->frame.size[1]*3
					*sizeof(unsigned char));
    if (info->buffer==NULL)
      fprintf(stderr,"Can't allocate buffer! Aiiieee!\n");
  }
  */
  // copy all new parameters:
  memcpy(&save_service->local_param_copy, save_service->current_buffer,sizeof(buffer_t));
  save_service->local_param_copy.frame.allocated_image_bytes=0;
 
}
