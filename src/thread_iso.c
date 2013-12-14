/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

gint IsoStartThread(camera_t* cam)
{
  int err;
  //int channel, speed;
  chain_t* iso_service=NULL;
  isothread_info_t *info=NULL;
  dc1394switch_t iso_state;
  dc1394video_mode_t video_mode;
  dc1394framerate_t framerate;

  iso_service=GetService(cam, SERVICE_ISO);

  if (iso_service==NULL) { // if no ISO service running...
    iso_service=(chain_t*)malloc(sizeof(chain_t));
    iso_service->current_buffer=NULL;
    iso_service->next_buffer=NULL;
    iso_service->data=(void*)malloc(sizeof(isothread_info_t));
    pthread_mutex_init(&iso_service->mutex_struct, NULL);
    pthread_mutex_init(&iso_service->mutex_data, NULL);
    
    info=(isothread_info_t*)iso_service->data;

    /* if you want a clean-interrupt thread:*/
    pthread_mutex_init(&info->mutex_cancel, NULL);
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=0;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    dc1394_video_get_mode(cam->camera_info, &video_mode);
    
    /* currently FORMAT_STILL_IMAGE is not supported*/
    if (dc1394_is_video_mode_still_image(video_mode)==DC1394_TRUE) {
      FreeChain(iso_service);
      iso_service=NULL;
      return(-1);
    }

    // IF ISO IS ACTIVE, DON'T SET ANYTHING BUT CAPTURE
    err=dc1394_video_get_transmission(cam->camera_info, &iso_state);
    if (err!=DC1394_SUCCESS){
      eprint("Failed to get ISO state. Error code %d\n",err);
      FreeChain(iso_service);
      iso_service=NULL;
      return(-1);
    }

    
    if (iso_state!=DC1394_ON) {
      /*
    int maxspeed;
    // ONLY IF LEGACY. OTHERWISE S800.
    switch (cam->camera_info->phy_speed) {
    case 0: maxspeed=DC1394_ISO_SPEED_100;break;
    case 1: maxspeed=DC1394_ISO_SPEED_200;break;
    case 2: maxspeed=DC1394_ISO_SPEED_400;break;
    case 3: maxspeed=DC1394_ISO_SPEED_800;break;
#if 0
    case 4: maxspeed=DC1394_ISO_SPEED_1600;break;
    case 5: maxspeed=DC1394_ISO_SPEED_3200;break;
#endif
    default:
      fprintf(stderr, "%s: unhandled phy speed %d\n", __FUNCTION__, cam->camera_info->phy_speed);
      maxspeed=DC1394_ISO_SPEED_100;
      break;
    }

    if (maxspeed >= DC1394_ISO_SPEED_800) {
      if (dc1394_video_set_operation_mode(cam->camera_info, DC1394_OPERATION_MODE_1394B)!=DC1394_SUCCESS) {
	fprintf(stderr,"Can't set 1394B mode. Reverting to 400Mbps\n");
	maxspeed=DC1394_ISO_SPEED_400;
      }
    }

    // set ISO speed:
    err=dc1394_video_set_iso_speed(cam->camera_info, maxspeed);
    if (err!=DC1394_SUCCESS){
      eprint("Failed to set ISO speed. Error code %d\n",err);
      FreeChain(iso_service);
      iso_service=NULL;
      return(-1);
    }
      */

    // set format and other stuff
    err=dc1394_video_set_mode(cam->camera_info, video_mode);
    if (err!=DC1394_SUCCESS){
      eprint("Failed to set current video mode. Error code %d\n",err);
      FreeChain(iso_service);
      iso_service=NULL;
      return(-1);
    }
    
    // set framerate or ROI:
    if (dc1394_is_video_mode_scalable(video_mode)==DC1394_TRUE) {
      err=dc1394_format7_set_roi(cam->camera_info, video_mode,
				 DC1394_QUERY_FROM_CAMERA, DC1394_QUERY_FROM_CAMERA, 
				 DC1394_QUERY_FROM_CAMERA, DC1394_QUERY_FROM_CAMERA, 
				 DC1394_QUERY_FROM_CAMERA, DC1394_QUERY_FROM_CAMERA);
      if (err!=DC1394_SUCCESS){
	eprint("Failed to set format7 mode. Error code %d\n",err);
	FreeChain(iso_service);
	iso_service=NULL;
	return(-1);
      }
    }
    else {
      dc1394_video_get_framerate(cam->camera_info, &framerate);
      err=dc1394_video_set_framerate(cam->camera_info, framerate);
      if (err!=DC1394_SUCCESS){
	eprint("Failed to set framerate. Error code %d\n",err);
	FreeChain(iso_service);
	iso_service=NULL;
	return(-1);
      }
    }

    } // end if iso_state is on.

    err=dc1394_capture_setup(cam->camera_info, cam->prefs.dma_buffer_size, DC1394_CAPTURE_FLAGS_DEFAULT);
    if (err!=DC1394_SUCCESS){
      eprint("Failed to setup capture. Error code %d\n",err);
      FreeChain(iso_service);
      iso_service=NULL;
      return(-1);
    }
    
    pthread_mutex_lock(&iso_service->mutex_data);
    CommonChainSetup(cam, iso_service, SERVICE_ISO);
    // init image buffers structs
    info->tempframe.allocated_image_bytes=0;
    info->tempframe.image=NULL;

    pthread_mutex_lock(&iso_service->mutex_struct);
    InsertChain(cam,iso_service);

    //iso_service->timeout_func_id=-1;
    if (pthread_create(&iso_service->thread, NULL, IsoThread,(void*) iso_service)) {
      RemoveChain(cam, iso_service);
      pthread_mutex_unlock(&iso_service->mutex_struct);
      pthread_mutex_unlock(&iso_service->mutex_data);
      FreeChain(iso_service);
      iso_service=NULL;
      return(-1);
    }
    else {
      pthread_mutex_unlock(&iso_service->mutex_struct);
      pthread_mutex_unlock(&iso_service->mutex_data);
    }
  }

  return (1);
}

void*
IsoThread(void* arg)
{
  chain_t *iso_service;
  isothread_info_t *info;
  float tmp;
  dc1394camera_t *camptr;
  struct tm captime;
  dc1394error_t err;
  struct timeval rawtime;

  iso_service=(chain_t*)arg;

  pthread_mutex_lock(&iso_service->mutex_data);
  info=(isothread_info_t*)iso_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&iso_service->mutex_data);

  camera_t *cam=iso_service->camera;

  dc1394video_frame_t *frame=NULL;
  uint8_t *backup_ptr;
  uint64_t backup_alloc;

  // time inits:
  iso_service->prev_time = 0;
  iso_service->prev_period = 0;
  iso_service->drop_warning = 0;
  iso_service->processed_frames=0;

  while (1) {
    /* Clean cancel handlers */
    pthread_mutex_lock(&info->mutex_cancel);
    if (info->cancel_req>0) {
      //fprintf(stderr,"Cancel request found, breaking...\n");
      break;
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel);
      pthread_mutex_lock(&iso_service->mutex_data);

      // IF FRAME USED (and we use no-drop mode)
      if (((iso_service->ready>0)&&(cam->prefs.iso_nodrop>0))||
	  (cam->prefs.iso_nodrop==0)) {
	
	camptr=iso_service->camera->camera_info;
	
	err=dc1394_capture_dequeue(camptr, DC1394_CAPTURE_POLICY_POLL, &frame);

	if ((err==DC1394_SUCCESS)&&(frame!=NULL)) { // should check for more errors here  
	  //fprintf(stderr,"size: %d %d\n",frame->size[0],frame->size[1]);
	
	  rawtime.tv_sec=frame->timestamp/1000000;
	  rawtime.tv_usec=frame->timestamp%1000000;
	  localtime_r(&rawtime.tv_sec, &captime);
	  sprintf(iso_service->current_buffer->captime_string,"%04d%02d%02d-%02d%02d%02d-%03d",
		  captime.tm_year+1900,
		  captime.tm_mon+1,
		  captime.tm_mday,
		  captime.tm_hour,
		  captime.tm_min,
		  captime.tm_sec,
		  (int)rawtime.tv_usec/1000);
	  
	  // check current buffer status
	  // IsoThreadCheckParams(iso_service); // THIS IS AUTOMATIC NOW!!
	  
	  // if the manually set bpp is different from the bpp found in the frame, set the frame bpp to the manual bpp:
	  if (frame->data_depth!=iso_service->camera->bpp)
	    frame->data_depth=iso_service->camera->bpp;

	  // copy the buffer data, but only copy the pointer to the image
	  memcpy(&info->tempframe,frame,sizeof(dc1394video_frame_t));
	  // we don't allocate the image buffer of tempframe (pointer copy from the DMA ring buffer)
	  // so the alloc should be zero
	  info->tempframe.allocated_image_bytes=0;
	  
	  // stereo decoding -> tempframe
	  switch (iso_service->camera->stereo) {
	  case -1:
	    // nothing to do at this time, the picture is already in tempframe.
	    break;
	  default:
		// tempframe.image currently points to the DMA ring buffer. However, it
		// will be reallocated by the deinterlacing function. In order not to
		// free the DMA buffer, set it to NULL now. We will have to free it
		// later, or else we will leak memory.
		info->tempframe.image=NULL;
	    dc1394_deinterlace_stereo_frames(frame,&info->tempframe,(dc1394stereo_method_t)iso_service->camera->stereo);
	    break;
	  }
	  
	  // bayer decoding

	  // we want to keep the UI at the same setting, but we ignore the UI value if the color mode is clearly not bayer type
	  int bayer=iso_service->camera->bayer;
	  if ((info->tempframe.color_coding!=DC1394_COLOR_CODING_RAW8) &&
	      (info->tempframe.color_coding!=DC1394_COLOR_CODING_RAW16) &&
	      (info->tempframe.color_coding!=DC1394_COLOR_CODING_MONO8) &&
	      (info->tempframe.color_coding!=DC1394_COLOR_CODING_MONO16)) {
	    bayer=-1;
	  }

	  // If a bayer pattern is used, the debayer function will copy the data to
	  // iso_service->current_buffer. If not, it is copied using memcpy. We
	  // do that regardless of whether stereo deinterlacing was applied.
	  switch (bayer) {
	  case -1:
	      if (iso_service->current_buffer->frame.allocated_image_bytes<info->tempframe.total_bytes) {
		if (iso_service->current_buffer->frame.allocated_image_bytes!=0)
		  free(iso_service->current_buffer->frame.image);
		iso_service->current_buffer->frame.image=(uint8_t*)malloc(info->tempframe.total_bytes*sizeof(uint8_t));
		iso_service->current_buffer->frame.allocated_image_bytes=info->tempframe.total_bytes;
	      }
	      // copy all info, but don't overwrite the (newly allocated) image field pointer and the allocated size
	      backup_ptr=iso_service->current_buffer->frame.image;
	      backup_alloc=iso_service->current_buffer->frame.allocated_image_bytes;
	      memcpy(&iso_service->current_buffer->frame,&info->tempframe,sizeof(dc1394video_frame_t));
	      iso_service->current_buffer->frame.image=backup_ptr;
	      iso_service->current_buffer->frame.allocated_image_bytes=backup_alloc;
	      memcpy(iso_service->current_buffer->frame.image,info->tempframe.image,info->tempframe.total_bytes*sizeof(uint8_t));
	    break;
	  default:
	    if (info->tempframe.color_filter==0)
	      info->tempframe.color_filter=iso_service->camera->bayer_pattern;
	    err=dc1394_debayer_frames(&info->tempframe, &iso_service->current_buffer->frame,iso_service->camera->bayer);
	    DC1394_WRN(err,"oops\n");
	    break;
	  }

	  // Free tempframe.image if stereo interlacing was performed.
	  if (iso_service->camera->stereo!=-1)
	  {
		  // Stereo deinterlacing was performed. During stereo interlacing,
		  // tempframe.image is allocated new memory by the dc1394 library.
		  // The tempframe was copied to current_buffer->frame above, either by
		  // dc1394_debayer_frames or by memcpy if no debayering is performed.
		  // So we can free tempframe.image at this point.
		  free (info->tempframe.image);
		  info->tempframe.image=NULL;
		  info->tempframe.allocated_image_bytes=0;
	  }
	  
	  // FPS computation:
	  iso_service->processed_frames++;
	  
	  tmp=((float)(frame->timestamp-iso_service->prev_time))/1000000.0;
	  if (iso_service->prev_time==0) {
	    iso_service->fps=fabs(0.0);
	  }
	  else {
	    if (tmp==0)
	      iso_service->fps=fabs(0.0);
	    else
	      iso_service->fps=fabs(1/tmp);
	  }
	  if (iso_service->prev_time!=0) {
	    iso_service->prev_period=tmp;
	  }
	  // produce a drop warning if the period difference is over 50%
	  if (iso_service->prev_period!=0) {
	    if (fabs(iso_service->prev_period-tmp)/(iso_service->prev_period/2+tmp/2)>=.5)
	      iso_service->drop_warning++;
	  }
	  iso_service->prev_time=frame->timestamp;

	  // return the frame to the DMA ring buffer
	  dc1394_capture_enqueue(camptr,frame);

	  PublishBufferForNext(iso_service);
	  pthread_mutex_unlock(&iso_service->mutex_data);
	}
	else {
	  pthread_mutex_unlock(&iso_service->mutex_data); // NOTE: mutex unlocks should be done BEFORE usleep()!!!
	  usleep(100);
	}
      }
      else {
	pthread_mutex_unlock(&iso_service->mutex_data);
	usleep(100);
      }
      //fprintf(stderr,"got frame %.7f %d\n",iso_service->fps, rand());
    }
  }
  return ((void*)1);
  
}


gint IsoStopThread(camera_t* cam)
{
  isothread_info_t *info;
  chain_t *iso_service;
  iso_service=GetService(cam,SERVICE_ISO);  

  //fprintf(stderr,"Cancel requested\n");

  if (iso_service!=NULL) { // if ISO service running...
    //fprintf(stderr,"Preforming thread removal\n");
    info=(isothread_info_t*)iso_service->data;
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=1;
    //fprintf(stderr,"done\n");
    pthread_mutex_unlock(&info->mutex_cancel);
    pthread_join(iso_service->thread, NULL);
    pthread_mutex_lock(&iso_service->mutex_data);
    pthread_mutex_lock(&iso_service->mutex_struct);

    //eprint("test1\n");

    RemoveChain(cam,iso_service);
    
    if ((info->tempframe.image!=NULL)&&(info->tempframe.allocated_image_bytes>0)) {
      free(info->tempframe.image);
      info->tempframe.image=NULL;
      info->tempframe.allocated_image_bytes=0;
    }
    //eprint("test2\n");

    dc1394_capture_stop(iso_service->camera->camera_info);
    
    //eprint("test3\n");
    pthread_mutex_unlock(&iso_service->mutex_struct);
    pthread_mutex_unlock(&iso_service->mutex_data);
    
    FreeChain(iso_service);
    iso_service=NULL;
    
  }
  //eprint("test final\n");
  
  return (1);
}
