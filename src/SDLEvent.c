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

// This should be probed automatically.
// This limitation might come from the video RAM.
#define MAX_DISPLAY_WIDTH  8192
#define MAX_DISPLAY_HEIGHT 8192

int
SDLEventStartThread(chain_t *display_service)
{
  displaythread_info_t *info;

  info=(displaythread_info_t*)display_service->data;
  pthread_mutex_init(&info->mutex_cancel_event, NULL);
  pthread_mutex_init(&info->mutex_event, NULL);

  pthread_mutex_lock(&info->mutex_cancel_event);
  info->cancel_event_req=0;
  pthread_mutex_unlock(&info->mutex_cancel_event);
  if (pthread_create(&info->event_thread, NULL, SDLEventThread, (void*)display_service))
    return(1);
  else
    return(0);
}

void*
SDLEventThread(void *arg)
{
  chain_t* display_service;
  displaythread_info_t *info;
 
  display_service=(chain_t*)arg;
  pthread_mutex_lock(&display_service->mutex_data);
  info=(displaythread_info_t*)display_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&display_service->mutex_data);

  while (1) {
    pthread_mutex_lock(&info->mutex_cancel_event);
    if (info->cancel_event_req>0) {
      pthread_mutex_unlock(&info->mutex_cancel_event);
      return ((void*)1);
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel_event);

      pthread_mutex_lock(&info->mutex_event);
      if (!SDLHandleEvent(display_service)) {
	pthread_mutex_unlock(&info->mutex_event);
	// SDL_QUIT called, close display thread
	DisplayStopThread(camera);
	// the following is only for the display_service button to be updated.
	WM_cancel_display=1;
	break;
      }
      else {
	pthread_mutex_unlock(&info->mutex_event);
	usleep(DELAY);
      }
    }
  }
  return ((void*)0);
}

void
SDLEventStopThread(chain_t *display_service)
{
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;
  // send request for cancellation:
  pthread_mutex_lock(&info->mutex_cancel_event);
  info->cancel_event_req=1;
  pthread_mutex_unlock(&info->mutex_cancel_event);
  
  // when cancellation occured, join:
  pthread_join(info->event_thread, NULL);
}


int
SDLHandleEvent(chain_t *display_service)
{
  displaythread_info_t *info;
  SDL_Event event;

  info=(displaythread_info_t*)display_service->data;

  while ( SDL_PollEvent(&event) ) {
    pthread_mutex_lock(&display_service->mutex_data);
    switch(event.type) {
    case SDL_VIDEORESIZE:
      SDLResizeDisplay(display_service, event.resize.w, event.resize.h);
      break;
    case SDL_KEYDOWN:
      OnKeyPressed(display_service,event.key.keysym.sym, event.key.keysym.mod);
      break;
    case SDL_KEYUP:
      OnKeyReleased(display_service,event.key.keysym.sym, event.key.keysym.mod);
      break;
    case SDL_MOUSEBUTTONDOWN:
      OnMouseDown(display_service,event.button.button, event.button.x, event.button.y);
      break;
    case SDL_MOUSEBUTTONUP:
      OnMouseUp(display_service,event.button.button, event.button.x, event.button.y);
      break;
    case SDL_MOUSEMOTION:
      OnMouseMotion(display_service, event.motion.x, event.motion.y);
      break;
    case SDL_QUIT:
      pthread_mutex_unlock(&display_service->mutex_data);
      return(0);
      break;
    }
    pthread_mutex_unlock(&display_service->mutex_data);
  }
  return(1);
}


void
OnKeyPressed(chain_t *display_service, int key, int mod)
{
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  //printf ("%s: 0x%02x\n", __FUNCTION__, key);
  switch (key) {
  case SDLK_n:
    // set display to normal size
    //fprintf(stderr,"Gaa! resize called!\n");
    SDLResizeDisplay(display_service, display_service->current_buffer->frame.size[0], display_service->current_buffer->frame.size[1]);
    break;
  case SDLK_f:
    // toggle fullscreen mode
    SDL_WM_ToggleFullScreen(info->sdlvideo);
    break;
  case SDLK_c:
    // crop image
    SDLCropImage(display_service);
    break;
  case SDLK_m:
    // set F7 image to max size
    SDLSetMaxSize(display_service);
    break;
  case SDLK_PAGEUP:
    // image size +10%
    SDLResizeDisplay(display_service, info->sdlvideorect.w*10/9, info->sdlvideorect.h*10/9);
    break;
  case SDLK_PAGEDOWN:
    // image size -10%
    SDLResizeDisplay(display_service, info->sdlvideorect.w*9/10, info->sdlvideorect.h*9/10);
    break;
#if 1
    /* This should leave. SDL supplies the raw key with modifiers, so e.g.
       'a' = SDLK_a,0 'A' = SDLK_a,KMOD_SHIFT, etc. On my keyboard, '<' is
       SDLK_COMMA,KMOD_SHIFT and SDLK_GREATER will never arrive here.
    */
  case SDLK_RIGHTBRACKET:
  case SDLK_LESS:
    // image size *2
    if (mod&(KMOD_SHIFT)) {
      SDLResizeDisplay(display_service, info->sdlvideorect.w/2, info->sdlvideorect.h/2);
    } else {
      SDLResizeDisplay(display_service, info->sdlvideorect.w*2, info->sdlvideorect.h*2);
    }
    break;
  case SDLK_LEFTBRACKET:
  case SDLK_GREATER:
    // image size /2
    if (mod&(KMOD_SHIFT)) {
      SDLResizeDisplay(display_service, info->sdlvideorect.w*2, info->sdlvideorect.h*2);
    } else {
      SDLResizeDisplay(display_service, info->sdlvideorect.w/2, info->sdlvideorect.h/2);
    }
    break;
#endif
  }
      
}

void
OnKeyReleased(chain_t *display_service, int key, int mod)
{

}

void
OnMouseDown(chain_t *display_service, int button, int x, int y)
{
  displaythread_info_t *info;
  dc1394format7mode_t* f7info;
  info=(displaythread_info_t*)display_service->data;

  dc1394video_mode_t video_mode;
  dc1394_video_get_mode(camera->camera_info, &video_mode);

  if (dc1394_is_video_mode_scalable(video_mode)) {
    f7info=&camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN];
  }

  switch (button) {
  case SDL_BUTTON_LEFT:
    pthread_mutex_lock(&watchthread_info.mutex_area);
    watchthread_info.draw=1;
    watchthread_info.mouse_down=1;
    // there is some adaptation because the display size can be different
    // from the real image size. (i.e. the image can be resized)
    watchthread_info.first[0]= ((x*display_service->current_buffer->frame.size[0] /info->sdlvideorect.w));
    watchthread_info.first[1]= ((y*display_service->current_buffer->frame.size[1]/info->sdlvideorect.h));

    // zero size at first click, -> copy click coordinate to second corner
    watchthread_info.second[0]=watchthread_info.first[0];
    watchthread_info.second[1]=watchthread_info.first[1];
    watchthread_info.upper_left[0]=watchthread_info.first[0];
    watchthread_info.upper_left[1]=watchthread_info.first[1];
    watchthread_info.lower_right[0]=watchthread_info.first[0];
    watchthread_info.lower_right[1]=watchthread_info.first[1];

    GetValidF7Crop(&watchthread_info, display_service);

    pthread_mutex_unlock(&watchthread_info.mutex_area);
    break;
  case SDL_BUTTON_MIDDLE:
    x=x*display_service->current_buffer->frame.size[0]/info->sdlvideorect.w; //rescaling
    y=y*display_service->current_buffer->frame.size[1]/info->sdlvideorect.h;
    switch(preferences.overlay_byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      cursor_info.col_y=info->sdloverlay->pixels[0][(y*display_service->current_buffer->frame.size[0]+x)*2];
      cursor_info.col_u=info->sdloverlay->pixels[0][(((y*display_service->current_buffer->frame.size[0]+x)>>1)<<2)+1]-127;
      cursor_info.col_v=info->sdloverlay->pixels[0][(((y*display_service->current_buffer->frame.size[0]+x)>>1)<<2)+3]-127;
      break;
    case DC1394_BYTE_ORDER_UYVY:
      cursor_info.col_u=info->sdloverlay->pixels[0][(y*display_service->current_buffer->frame.size[0]+x)*2]-127;
      cursor_info.col_y=info->sdloverlay->pixels[0][(((y*display_service->current_buffer->frame.size[0]+x)>>1)<<2)+1];
      cursor_info.col_v=info->sdloverlay->pixels[0][(((y*display_service->current_buffer->frame.size[0]+x)>>1)<<2)+2]-127;
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order\n");
      break;
    }
    YUV2RGB(cursor_info.col_y, cursor_info.col_u, cursor_info.col_v,
	    cursor_info.col_r, cursor_info.col_g, cursor_info.col_b);
    cursor_info.x=x;
    cursor_info.y=y;
    cursor_info.update_req=1;
    break;
  case SDL_BUTTON_RIGHT:
    //whitebal_data->x=x*display_service->current_buffer->frame.size[0]/info->sdlvideorect.w; //rescaling
    //whitebal_data->y=y*display_service->current_buffer->frame.size[1]/info->sdlvideorect.h;
    //whitebal_data->service=display_service;
    //pthread_create(&whitebal_data->thread, NULL, AutoWhiteBalance, (void*)&whitebal_data);
    break;
  default:
    //fprintf(stderr,"Bad button ID in SDL!\n");
    break;
  }
}

void
OnMouseUp(chain_t *display_service, int button, int x, int y)
{

  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

  switch (button) {
  case SDL_BUTTON_LEFT:
    pthread_mutex_lock(&watchthread_info.mutex_area);
    watchthread_info.mouse_down=0;
    // there is some adaptation because the display size can be different
    // from the real image size. (i.e. the image can be resized)
    //info->lower_right[0]=x*display_service->current_buffer->frame.size[0]/info->sdlvideorect.w;
    //info->lower_right[1]=y*display_service->current_buffer->frame.size[1]/info->sdlvideorect.h;
    pthread_mutex_unlock(&watchthread_info.mutex_area);
    break;
  case SDL_BUTTON_MIDDLE:
    break;
  case SDL_BUTTON_RIGHT:
    break;
  default:
    //fprintf(stderr,"Bad button ID in SDL!\n");
    break;
  }
}


void
OnMouseMotion(chain_t *display_service, int x, int y)
{
  displaythread_info_t *info;

  info=(displaythread_info_t*)display_service->data;
  pthread_mutex_lock(&watchthread_info.mutex_area);

  if (watchthread_info.mouse_down==1) {
    watchthread_info.second[0]=x*display_service->current_buffer->frame.size[0]/info->sdlvideorect.w;
    watchthread_info.second[1]=y*display_service->current_buffer->frame.size[1]/info->sdlvideorect.h;

    // we need to flip the corners if the pointer moves backwards
    if (watchthread_info.second[0]<watchthread_info.first[0])
      watchthread_info.upper_left[0]=watchthread_info.second[0];
    else
      watchthread_info.upper_left[0]=watchthread_info.first[0];

    if (watchthread_info.second[1]<watchthread_info.first[1])
      watchthread_info.upper_left[1]=watchthread_info.second[1];
    else
      watchthread_info.upper_left[1]=watchthread_info.first[1];

    if (watchthread_info.second[0]>=watchthread_info.first[0])
      watchthread_info.lower_right[0]=watchthread_info.second[0];
    else
      watchthread_info.lower_right[0]=watchthread_info.first[0];

    if (watchthread_info.second[1]>=watchthread_info.first[1])
      watchthread_info.lower_right[1]=watchthread_info.second[1];
    else
      watchthread_info.lower_right[1]=watchthread_info.first[1];
   
    GetValidF7Crop(&watchthread_info,display_service);
  }
  pthread_mutex_unlock(&watchthread_info.mutex_area);
  
}

void
SDLResizeDisplay(chain_t *display_service, int width, int height)
{    
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;
  int prev_height, prev_width;

  prev_width=info->sdlvideorect.w;
  prev_height=info->sdlvideorect.h;

  // check that the size is not too small or too large.
  if ((width<1)||(height<1)||(width>MAX_DISPLAY_WIDTH)||(height>MAX_DISPLAY_HEIGHT))
    return;

  if (display_service->camera->prefs.display_keep_ratio>0) {
    // keep aspect ratio and resize following which dimension we change
    if (abs(width-info->sdlvideorect.w) >= (abs(height-info->sdlvideorect.h))) {
      // we changed the width, set height accordingly
      info->sdlvideorect.w = width;
      info->sdlvideorect.h = (width * display_service->current_buffer->frame.size[1]) / display_service->current_buffer->frame.size[0];
    }
    else {
      // we changed the height, set width accordingly
      info->sdlvideorect.w = (height * display_service->current_buffer->frame.size[0]) / display_service->current_buffer->frame.size[1];
      info->sdlvideorect.h = height;
    }
  }
  else {
    // bypass aspect keep:
    info->sdlvideorect.w = width;
    info->sdlvideorect.h = height;
  }

  info->sdlvideorect.x = 0;
  info->sdlvideorect.y = 0;

  // maximize display size to XV size if necessary
  if ((xvinfo.max_width!=-1)&&(xvinfo.max_height!=-1)) {
    if (info->sdlvideorect.w>xvinfo.max_width) {
      info->sdlvideorect.w=xvinfo.max_width;
    }
    if (info->sdlvideorect.h>xvinfo.max_height) {
      info->sdlvideorect.h=xvinfo.max_height;
    }
  }

  //fprintf(stderr,"SDLResize: prev size: [%d %d], videorect size [%d %d]\n",prev_width, prev_height, info->sdlvideorect.w, info->sdlvideorect.h);

  // if size change is effective, re-set SDL stuff
  if ((prev_width!=info->sdlvideorect.w)||(prev_height!=info->sdlvideorect.h)) {

    // Free overlay & video surface
    //fprintf(stderr,"Freeing overlay...");
    SDL_FreeYUVOverlay(info->sdloverlay);
    //fprintf(stderr,"and video...");
    SDL_FreeSurface(info->sdlvideo);
    //fprintf(stderr,"done\n");

    // new video mode
    info->sdlbpp = SDL_VideoModeOK(info->sdlvideorect.w, info->sdlvideorect.h, info->sdlbpp, info->sdlflags); 
    //fprintf(stderr,"create new video mode with size [%d %d], bpp %d and flags 0x%lx...\n",info->sdlvideorect.w, info->sdlvideorect.h, info->sdlbpp, info->sdlflags);
    info->sdlvideo = SDL_SetVideoMode(info->sdlvideorect.w, info->sdlvideorect.h, info->sdlbpp, info->sdlflags); // THIS LINE SOMETIMES SUCKS WHEN CHANGING FORMAT W/H
    if (info->sdlvideo == NULL) {
      Error(SDL_GetError());
      SDL_Quit();
      return;
    }
    
    //if (SDL_SetColorKey(info->sdlvideo, SDL_SRCCOLORKEY, 0x0) < 0 ) {
    //  Error(SDL_GetError());
    //}
    
    //fprintf(stderr,"create overlay with size [%d %d]...\n",display_service->current_buffer->frame.size[0],display_service->current_buffer->frame.size[1]);
    
    // Create YUV Overlay  
    switch(preferences.overlay_byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      info->sdloverlay = SDL_CreateYUVOverlay(display_service->current_buffer->frame.size[0],
					      display_service->current_buffer->frame.size[1],
					      SDL_YUY2_OVERLAY,info->sdlvideo);
      break;
    case DC1394_BYTE_ORDER_UYVY:
      info->sdloverlay = SDL_CreateYUVOverlay(display_service->current_buffer->frame.size[0],
					      display_service->current_buffer->frame.size[1],
					      SDL_UYVY_OVERLAY,info->sdlvideo);
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order\n");
      break;
    }
    if (info->sdloverlay==NULL) {
      Error(SDL_GetError());
      SDL_Quit();
      return;
    }
    
    //fprintf(stderr,"Update completed\n");

  }
  
}


void
SDLCropImage(chain_t *display_service)
{
  pthread_mutex_lock(&watchthread_info.mutex_area);

  dc1394video_mode_t video_mode;
  dc1394_video_get_mode(camera->camera_info, &video_mode);

  watchthread_info.draw=0;
  if (dc1394_is_video_mode_scalable(video_mode))
    watchthread_info.crop=1;

  pthread_mutex_unlock(&watchthread_info.mutex_area);
    
}

void
SDLSetMaxSize(chain_t *display_service)
{
  pthread_mutex_lock(&watchthread_info.mutex_area);

  dc1394video_mode_t video_mode;
  dc1394_video_get_mode(camera->camera_info, &video_mode);

  watchthread_info.draw=0;
  if (dc1394_is_video_mode_scalable(video_mode)) {
    watchthread_info.crop=1;
    watchthread_info.pos[0]=0;
    watchthread_info.pos[1]=0;
    watchthread_info.size[0]=camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].max_size_x;
    watchthread_info.size[1]=camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].max_size_y;
  }
  pthread_mutex_unlock(&watchthread_info.mutex_area);

}

#endif
