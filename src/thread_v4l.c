/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *                         Dan Dennedy <dan@dennedy.org>
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
V4lStartThread(camera_t* cam)
{
  chain_t* v4l_service=NULL;
  v4lthread_info_t *info=NULL;
  char *stemp;

  stemp=(char*)malloc(STRING_SIZE*sizeof(char));

  v4l_service=GetService(cam, SERVICE_V4L);

  if (v4l_service==NULL) { // if no V4L service running...
    v4l_service=(chain_t*)malloc(sizeof(chain_t));
    v4l_service->current_buffer=NULL;
    v4l_service->next_buffer=NULL;
    v4l_service->data=(void*)malloc(sizeof(v4lthread_info_t));
    info=(v4lthread_info_t*)v4l_service->data;
    pthread_mutex_init(&v4l_service->mutex_data, NULL);
    pthread_mutex_init(&v4l_service->mutex_struct, NULL);
    pthread_mutex_init(&info->mutex_cancel, NULL);
    
    /* if you want a clean-interrupt thread:*/
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=0;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    /* setup v4l_thread: handles, ...*/
    pthread_mutex_lock(&v4l_service->mutex_data);
    
    CommonChainSetup(cam, v4l_service,SERVICE_V4L);
    
    info->frame.image=NULL;
    info->frame.allocated_image_bytes=0;

    // open V4L device
    info->v4l_dev=-1;
    info->v4l_dev = open(cam->prefs.v4l_dev_name, O_RDWR);
    if (info->v4l_dev < 0) {
      sprintf(stemp,"Failed to open V4L device %s",cam->prefs.v4l_dev_name);
      Error(stemp);
      free(stemp);
      stemp=NULL;
      FreeChain(v4l_service);
      v4l_service=NULL;
      return(-1);
    }

    /* Insert chain and start service*/
    pthread_mutex_lock(&v4l_service->mutex_struct);
    InsertChain(cam, v4l_service);
    if (pthread_create(&v4l_service->thread, NULL, V4lThread,(void*) v4l_service)) {
      /* error starting thread. You should cleanup here
	 (free, unset global vars,...):*/
      
      /* Mendatory cleanups:*/
      RemoveChain(cam, v4l_service);
      pthread_mutex_unlock(&v4l_service->mutex_struct);
      pthread_mutex_unlock(&v4l_service->mutex_data);
      if ((info->frame.image!=NULL)&&(info->frame.allocated_image_bytes>0)) {
	free(info->frame.image);
	info->frame.allocated_image_bytes=0;
      }
      free(stemp);
      stemp=NULL;
      FreeChain(v4l_service);
      v4l_service=NULL;
      return(-1);
    }

    pthread_mutex_unlock(&v4l_service->mutex_struct);
    pthread_mutex_unlock(&v4l_service->mutex_data);
    
  }
  free(stemp);
  stemp=NULL;
  return (1);
}


void*
V4lThread(void* arg)
{
  chain_t* v4l_service=NULL;
  v4lthread_info_t *info=NULL;
  long int skip_counter;
  float tmp;
  struct video_picture p;
  
  v4l_service=(chain_t*)arg;
  pthread_mutex_lock(&v4l_service->mutex_data);
  info=(v4lthread_info_t*)v4l_service->data;
  camera_t *cam=v4l_service->camera;

  skip_counter=0;
  v4l_service->processed_frames=0;

  /* These settings depend on the thread. For 100% safe deferred-cancel
   threads, I advise you use a custom thread cancel flag. See display thread.*/
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&v4l_service->mutex_data);
  
  // time inits:
  v4l_service->prev_time = 0;
  v4l_service->prev_period = 0;
  v4l_service->drop_warning = 0;
  v4l_service->processed_frames=0;

  while (1) { 
    /* Clean cancel handlers */
    pthread_mutex_lock(&info->mutex_cancel);
    if (info->cancel_req>0) {
      break;
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel);
      pthread_mutex_lock(&v4l_service->mutex_data);
      if(GetBufferFromPrevious(v4l_service)) { // have buffers been rolled?
	// check params
	V4lThreadCheckParams(v4l_service);
	
	/* IF we have mono data then set V4L for mono(grey) output */
	/* Only do this ONCE before writing the first frame */
	if (((v4l_service->current_buffer->frame.color_coding == DC1394_COLOR_CODING_MONO8) ||
	     (v4l_service->current_buffer->frame.color_coding == DC1394_COLOR_CODING_RAW8)) && v4l_service->processed_frames==0) {
	  Warning("Setting V4L device to GREY palette");
	  if (ioctl(info->v4l_dev,VIDIOCGPICT,&p) < 0) 
	    Error("ioctl(VIDIOCGPICT) error");
	  else {
	    p.palette = VIDEO_PALETTE_GREY;
	    if (ioctl(info->v4l_dev,VIDIOCSPICT,&p) < 0) 
	      Error("ioctl(VIDIOCSPICT) Error");
	  }
	}

	// Convert to RGB unless we are using direct GREY palette
	if ((v4l_service->current_buffer->frame.color_coding != DC1394_COLOR_CODING_MONO8) &&
	    (v4l_service->current_buffer->frame.color_coding != DC1394_COLOR_CODING_RAW8)) {
	  convert_to_rgb(&v4l_service->current_buffer->frame, &info->frame);
	  swap_rb(info->frame.image, v4l_service->current_buffer->frame.size[0]*v4l_service->current_buffer->frame.size[1]*3);
	}

	if (v4l_service->current_buffer->frame.size[0]!=-1) {
	  if (skip_counter>=(cam->prefs.v4l_period-1)) {
	    skip_counter=0;
	    if ((v4l_service->current_buffer->frame.color_coding != DC1394_COLOR_CODING_MONO8) &&
		(v4l_service->current_buffer->frame.color_coding != DC1394_COLOR_CODING_RAW8))
	      write(info->v4l_dev,info->frame.image,v4l_service->current_buffer->frame.size[0]*v4l_service->current_buffer->frame.size[1]*3);
	    else
	      write(info->v4l_dev,v4l_service->current_buffer->frame.image,v4l_service->current_buffer->frame.size[0]*v4l_service->current_buffer->frame.size[1]);
	    v4l_service->processed_frames++;
	  }
	  else
	    skip_counter++;
	  
	  // FPS computation:
	  tmp=((float)(v4l_service->current_buffer->frame.timestamp-v4l_service->prev_time))/1000000.0;
	  if (v4l_service->prev_time==0) {
	    v4l_service->fps=fabs(0.0);
	  }
	  else {
	    if (tmp==0)
	      v4l_service->fps=fabs(0.0);
	    else
	      v4l_service->fps=fabs(1/tmp);
	  }
	  if (v4l_service->prev_time!=0) {
	    v4l_service->prev_period=tmp;
	  }
	  // produce a drop warning if the period difference is over 50%
	  if (v4l_service->prev_period!=0) {
	    if (fabs(v4l_service->prev_period-tmp)/(v4l_service->prev_period/2+tmp/2)>=.5)
	      v4l_service->drop_warning++;
	  }
	  v4l_service->prev_time=v4l_service->current_buffer->frame.timestamp;

	}
	PublishBufferForNext(v4l_service);
	pthread_mutex_unlock(&v4l_service->mutex_data);
      }
      else {
	pthread_mutex_unlock(&v4l_service->mutex_data);
      }
    }
    usleep(100);
  }

  pthread_mutex_unlock(&info->mutex_cancel);
  return ((void*)1);

}


gint
V4lStopThread(camera_t* cam)
{
  v4lthread_info_t *info;
  chain_t *v4l_service;
  v4l_service=GetService(cam,SERVICE_V4L);

  if (v4l_service!=NULL) { // if V4L service running...
    info=(v4lthread_info_t*)v4l_service->data;
    /* Clean cancel handler: */
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=1;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    /* common handlers...*/
    pthread_join(v4l_service->thread, NULL);
    
    pthread_mutex_lock(&v4l_service->mutex_data);
    pthread_mutex_lock(&v4l_service->mutex_struct);
    RemoveChain(cam,v4l_service);
    
    /* Do custom cleanups here...*/
    if ((info->frame.image!=NULL)&&(info->frame.allocated_image_bytes>0)) {
      free(info->frame.image);
      info->frame.allocated_image_bytes=0;
    }

    close(info->v4l_dev);

    /* Mendatory cleanups: */
    pthread_mutex_unlock(&v4l_service->mutex_struct);
    pthread_mutex_unlock(&v4l_service->mutex_data);
    FreeChain(v4l_service);
    v4l_service=NULL;
    
  }
  
  return (1);
}

void
V4lThreadCheckParams(chain_t *v4l_service)
{

  v4lthread_info_t *info;
  info=(v4lthread_info_t*)v4l_service->data;

  // if some parameters changed, we need to re-allocate the local buffers and restart the v4l
  if ((v4l_service->current_buffer->frame.size[0]!=v4l_service->local_param_copy.frame.size[0])||
      (v4l_service->current_buffer->frame.size[1]!=v4l_service->local_param_copy.frame.size[1]) ) {

    // STOPING THE PIPE MIGHT BE NECESSARY HERE
    
    // "start pipe"      
    if (ioctl (info->v4l_dev, VIDIOCGCAP, &info->vid_caps) == -1) {
      perror ("ioctl (VIDIOCGCAP)");
    }
    if (ioctl (info->v4l_dev, VIDIOCGPICT, &info->vid_pic)== -1) {
      perror ("ioctl VIDIOCGPICT");
    }
    info->vid_pic.palette = VIDEO_PALETTE_RGB24;
    if (ioctl (info->v4l_dev, VIDIOCSPICT, &info->vid_pic)== -1) {
      perror ("ioctl VIDIOCSPICT");
    }
    if (ioctl (info->v4l_dev, VIDIOCGWIN, &info->vid_win)== -1) {
      perror ("ioctl VIDIOCGWIN");
    }
    info->vid_win.width=v4l_service->current_buffer->frame.size[0];
    info->vid_win.height=v4l_service->current_buffer->frame.size[1];
    if (ioctl (info->v4l_dev, VIDIOCSWIN, &info->vid_win)== -1) {
      perror ("ioctl VIDIOCSWIN");
    }
  }

  // copy all new parameters:
  memcpy(&v4l_service->local_param_copy, v4l_service->current_buffer,sizeof(buffer_t));
  v4l_service->local_param_copy.frame.allocated_image_bytes=0; // to avoid bad free...
  
}

void
swap_rb(unsigned char *image, int i) {

  unsigned char t;
  i--;

  while (i>0) {
    t=image[i];
    image[i]=image[i-2];
    image[i-2]=t;
    i-=3;
  }

}
