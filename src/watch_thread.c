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

#ifdef HAVE_SDLLIB

int
WatchStartThread(watchthread_info_t* info)
{
  // init threads, mutexes and data
  pthread_mutex_init(&info->mutex_area, NULL);
  pthread_mutex_init(&info->mutex_cancel_watch, NULL);
  pthread_mutex_lock(&info->mutex_area);
  info->draw=0;
  info->mouse_down=0;
  info->crop=0;
  info->pos[0]=0;
  info->pos[1]=0;
  info->size[0]=0;
  info->size[1]=0;
  info->upper_left[0]=0;
  info->upper_left[1]=0;
  info->lower_right[0]=0;
  info->lower_right[1]=0;
  pthread_mutex_unlock(&info->mutex_area);
  pthread_mutex_lock(&info->mutex_cancel_watch);
  info->cancel_watch_req=0;
  pthread_mutex_unlock(&info->mutex_cancel_watch);

  if (pthread_create(&info->thread, NULL, WatchThread, (void*)info))
    return(1);
  else
    return(0);
  
}


void*
WatchThread(void *arg)
{
  watchthread_info_t *info;
  info=(watchthread_info_t*)arg;
  dc1394video_mode_t video_mode;
  
  pthread_mutex_lock(&info->mutex_area);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&info->mutex_area);

  dc1394_video_get_mode(camera->camera_info, &video_mode);
  while (1) {
    pthread_mutex_lock(&info->mutex_cancel_watch);
    if (info->cancel_watch_req>0) {
      pthread_mutex_unlock(&info->mutex_cancel_watch);
      return ((void*)1);
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel_watch);
      pthread_mutex_lock(&info->mutex_area);
      
      if (info->crop>0) {
	if ((video_mode >= DC1394_VIDEO_MODE_FORMAT7_MIN) &&
	    (video_mode <= DC1394_VIDEO_MODE_FORMAT7_MAX)) {
	  if (dc1394_format7_get_image_position(camera->camera_info, video_mode,
						  &camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].pos_x,
						  &camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].pos_y)!=DC1394_SUCCESS)
	    Error("Could not get format7 image position");
	  // if we did reset to max size, don't do the addition:
	  if ((info->size[0]==camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].max_size_x)&&
	      (info->size[1]==camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].max_size_y))
	    SetFormat7Crop(info->size[0],info->size[1],info->pos[0],info->pos[1],video_mode);
	  else {
	    SetFormat7Crop(info->size[0],info->size[1],
			   info->pos[0]+camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].pos_x,
			   info->pos[1]+camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].pos_y,
			   video_mode);
	  }
	  //UpdateFormat7Ranges();
	  // no need to rebuild at all: setformat7crop includes rebuilding 
	}
	else {
	  SetFormat7Crop(info->size[0],info->size[1],info->pos[0],info->pos[1],video_mode);
	}
	info->crop=0;
      }
      
      pthread_mutex_unlock(&info->mutex_area);
      usleep(1000);
    }
    
  }
  
}


int
WatchStopThread(watchthread_info_t* info)
{
  // send request for cancellation:
  pthread_mutex_lock(&info->mutex_cancel_watch);
  info->cancel_watch_req=1;
  pthread_mutex_unlock(&info->mutex_cancel_watch);

  // when cancellation occured, join:
  pthread_join(info->thread, NULL);

  return (1);
}


void
GetValidF7Crop(watchthread_info_t *wtinfo, chain_t* display_service) {

  dc1394format7mode_t* f7info;
  dc1394video_mode_t video_mode;
  int mov[2];
  dc1394_video_get_mode(camera->camera_info, &video_mode);
  
  if ((video_mode >= DC1394_VIDEO_MODE_FORMAT7_MIN) &&
      (video_mode <= DC1394_VIDEO_MODE_FORMAT7_MAX)) {
    f7info=&camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN];

    // step_pos=step if no step_pos is supported.
    wtinfo->pos[0]=wtinfo->upper_left[0]-wtinfo->upper_left[0]%f7info->unit_pos_x;
    wtinfo->pos[1]=wtinfo->upper_left[1]-wtinfo->upper_left[1]%f7info->unit_pos_y;

    wtinfo->size[0]=wtinfo->lower_right[0]-wtinfo->pos[0];
    if (wtinfo->size[0]%f7info->unit_size_x>0) {
      wtinfo->size[0]=wtinfo->size[0]+f7info->unit_size_x;
    }
    wtinfo->size[0]=wtinfo->size[0]-wtinfo->size[0]%f7info->unit_size_x;
    if (wtinfo->size[0]<f7info->unit_size_x)
      wtinfo->size[0]=f7info->unit_size_x;

    wtinfo->size[1]=wtinfo->lower_right[1]-wtinfo->pos[1];
    if (wtinfo->size[1]%f7info->unit_size_y>0) {
      wtinfo->size[1]=wtinfo->size[1]+f7info->unit_size_y;
    }
    wtinfo->size[1]=wtinfo->size[1]-wtinfo->size[1]%f7info->unit_size_y;
    if (wtinfo->size[1]<f7info->unit_size_y)
      wtinfo->size[1]=f7info->unit_size_y;

    //fprintf(stderr,"[%d %d] [%d %d]  ",wtinfo->upper_left[0],wtinfo->upper_left[1],wtinfo->lower_right[0],wtinfo->lower_right[1]);
    //fprintf(stderr,"[%d %d] [%d %d]\n",wtinfo->pos[0],wtinfo->pos[1],wtinfo->size[0],wtinfo->size[1]);
    // optional recentering:
    if ((f7info->unit_pos_x<f7info->unit_size_x)||(f7info->unit_pos_y<f7info->unit_size_y)) {
      mov[0]=(wtinfo->lower_right[0]-wtinfo->upper_left[0])/2+wtinfo->upper_left[0]-(wtinfo->pos[0]+wtinfo->size[0]/2);
      mov[1]=(wtinfo->lower_right[1]-wtinfo->upper_left[1])/2+wtinfo->upper_left[1]-(wtinfo->pos[1]+wtinfo->size[1]/2);
      
      mov[0]=(mov[0]/f7info->unit_pos_x)*f7info->unit_pos_x;
      mov[1]=(mov[1]/f7info->unit_pos_y)*f7info->unit_pos_y;

      wtinfo->pos[0]+=mov[0];
      wtinfo->pos[1]+=mov[1];
      //fprintf(stderr," [%d %d] [%d %d]\n",wtinfo->pos[0],wtinfo->pos[1],wtinfo->size[0],wtinfo->size[1]);
    }
    
    // check boundaries:
    if (wtinfo->pos[0]<0)
      wtinfo->pos[0]=0;
    if (wtinfo->pos[1]<0)
      wtinfo->pos[1]=0;
    if (wtinfo->pos[0]+wtinfo->size[0]-1>display_service->current_buffer->frame.size[0])
      wtinfo->pos[0]-=wtinfo->pos[0]+wtinfo->size[0]-display_service->current_buffer->frame.size[0]; // there was a +1 here
    if (wtinfo->pos[1]+wtinfo->size[1]-1>display_service->current_buffer->frame.size[1])
      wtinfo->pos[1]-=wtinfo->pos[1]+wtinfo->size[1]-display_service->current_buffer->frame.size[1]; // there was a +1 here

  }
  else {
    wtinfo->pos[0]=wtinfo->upper_left[0];
    wtinfo->pos[1]=wtinfo->upper_left[1];
    wtinfo->size[0]=wtinfo->lower_right[0]-wtinfo->upper_left[0];
    wtinfo->size[1]=wtinfo->lower_right[1]-wtinfo->upper_left[1];
  }

  //fprintf(stderr," [%d %d] [%d %d]\n",wtinfo->pos[0],wtinfo->pos[1],wtinfo->size[0],wtinfo->size[1]);
}

#endif
