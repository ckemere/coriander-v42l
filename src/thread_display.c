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

gint
DisplayStartThread(camera_t* cam)
{
  chain_t *display_service=NULL;
  displaythread_info_t *info=NULL;

  display_service=GetService(cam,SERVICE_DISPLAY);

  if (display_service==NULL) {// if no display service running...
    display_service=(chain_t*)malloc(sizeof(chain_t));
    display_service->current_buffer=NULL;
    display_service->next_buffer=NULL;
    display_service->data=(void*)malloc(sizeof(displaythread_info_t));
    info=(displaythread_info_t*)display_service->data;
    pthread_mutex_init(&display_service->mutex_struct, NULL);
    pthread_mutex_init(&display_service->mutex_data, NULL);
    pthread_mutex_init(&info->mutex_cancel, NULL);
    
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=0;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    pthread_mutex_lock(&display_service->mutex_data);
    CommonChainSetup(cam, display_service,SERVICE_DISPLAY);
    pthread_mutex_lock(&display_service->mutex_struct);

    InsertChain(cam, display_service);
    
    if (pthread_create(&display_service->thread, NULL, DisplayThread, (void*)display_service)) {
      RemoveChain(cam, display_service);
      pthread_mutex_unlock(&display_service->mutex_struct);
      pthread_mutex_unlock(&display_service->mutex_data);
      FreeChain(display_service);
      return(-1);
    }

    pthread_mutex_unlock(&display_service->mutex_struct);
    pthread_mutex_unlock(&display_service->mutex_data);
    
  }
  return (1);
}


void*
DisplayThread(void* arg)
{
  chain_t* display_service=NULL;
  displaythread_info_t *info=NULL;
  long int skip_counter;
  float tmp;

  // we should only use mutex_data in this function

  display_service=(chain_t*)arg;
  pthread_mutex_lock(&display_service->mutex_data);
  info=(displaythread_info_t*)display_service->data;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&display_service->mutex_data);
  skip_counter=0;
  camera_t *cam=display_service->camera;

  // time inits:
  display_service->prev_time = 0;
  display_service->prev_period = 0;
  display_service->drop_warning = 0;
  display_service->processed_frames=0;

  while (1) {
    pthread_mutex_lock(&info->mutex_cancel);
    
    if (info->cancel_req>0) {
      break;
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel);
      pthread_mutex_lock(&display_service->mutex_data);
      if(GetBufferFromPrevious(display_service)) { // have buffers been rolled?
#ifdef HAVE_SDLLIB
	// check params
	DisplayThreadCheckParams(display_service);
#endif
	if (display_service->current_buffer->frame.size[0]!=-1) {
	  if (skip_counter>=(cam->prefs.display_period-1)) {
	    skip_counter=0;
#ifdef HAVE_SDLLIB
	    if (info->sdloverlay!=NULL) {
	      if (SDL_LockYUVOverlay(info->sdloverlay) == 0) {
		convert_to_yuv_for_SDL(&display_service->current_buffer->frame, info->sdloverlay, preferences.overlay_byte_order);
		// informative overlays
		SDLDisplayPattern(display_service);
		SDLDisplayArea(display_service);
		SDL_UnlockYUVOverlay(info->sdloverlay);
		SDL_DisplayYUVOverlay(info->sdloverlay, &info->sdlvideorect);
		
		info->redraw_prev_time=times(&info->redraw_tms_buf);
	      }
	    }
#endif
	    display_service->processed_frames++;
	  }
	  else { //
	    if (cam->prefs.display_redraw==DISPLAY_REDRAW_ON) {
	      ConditionalTimeoutRedraw(display_service);
	    }
	    skip_counter++;
	  }
	  // FPS computation:
	  tmp=((float)(display_service->current_buffer->frame.timestamp-display_service->prev_time))/1000000.0;
	  if (display_service->prev_time==0) {
	    display_service->fps=fabs(0.0);
	  }
	  else {
	    if (tmp==0)
	      display_service->fps=fabs(0.0);
	    else
	      display_service->fps=fabs(1/tmp);
	  }
	  if (display_service->prev_time!=0) {
	    display_service->prev_period=tmp;
	  }
	  // produce a drop warning if the period difference is over 50%
	  if (display_service->prev_period!=0) {
	    if (fabs(display_service->prev_period-tmp)/(display_service->prev_period/2+tmp/2)>=.5)
	      display_service->drop_warning++;
	  }
	  display_service->prev_time=display_service->current_buffer->frame.timestamp;
	}

	PublishBufferForNext(display_service);
	pthread_mutex_unlock(&display_service->mutex_data);
      }
      else { //
	if (cam->prefs.display_redraw==DISPLAY_REDRAW_ON) {
	  ConditionalTimeoutRedraw(display_service);
	}
	pthread_mutex_unlock(&display_service->mutex_data);
      }
    }
    usleep(100);
  }
  
  pthread_mutex_unlock(&info->mutex_cancel);
  return ((void*)1);
}

// the goal of the following function is to redraw the SDL display twice a second so that the image follows the screen
// during window movement or if another window comes momentarily on top of the display while no images are coming. 
void
ConditionalTimeoutRedraw(chain_t* service)
{
  displaythread_info_t *info=NULL;
  float interval;
  info=(displaythread_info_t*)service->data;

  if (service->current_buffer->frame.size[0]!=-1) {
    info->redraw_current_time=times(&info->redraw_tms_buf);
    interval=fabs((float)(info->redraw_current_time-info->redraw_prev_time)/sysconf(_SC_CLK_TCK));
    if (interval>(1.0/service->camera->prefs.display_redraw_rate)) { // redraw e.g. 4 times per second
#ifdef HAVE_SDLLIB
      if (SDL_LockYUVOverlay(info->sdloverlay) == 0) {
	//MainWarning("Conditional display redraw");
	convert_to_yuv_for_SDL(&service->current_buffer->frame, info->sdloverlay, preferences.overlay_byte_order);
	SDLDisplayArea(service);
	SDL_UnlockYUVOverlay(info->sdloverlay);
	SDL_DisplayYUVOverlay(info->sdloverlay, &info->sdlvideorect);
      }
#endif
      info->redraw_prev_time=times(&info->redraw_tms_buf);
    }
  }
}

gint
DisplayStopThread(camera_t* cam)
{
  displaythread_info_t *info;
  chain_t *display_service;
  display_service=GetService(cam,SERVICE_DISPLAY);
  
  if (display_service!=NULL) { // if display service running... 
    info=(displaythread_info_t*)display_service->data;
    
    // send request for cancellation:
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=1;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    // when cancellation occured, join:
    pthread_join(display_service->thread, NULL);
    
    pthread_mutex_lock(&display_service->mutex_data);
    pthread_mutex_lock(&display_service->mutex_struct);
    RemoveChain(cam,display_service);
#ifdef HAVE_SDLLIB
    SDLQuit(display_service);
#endif
    
    pthread_mutex_unlock(&display_service->mutex_struct);
    pthread_mutex_unlock(&display_service->mutex_data);
    FreeChain(display_service);
  }
  return (1);
}

#ifdef HAVE_SDLLIB

int
SDLInit(chain_t *display_service)
{
  displaythread_info_t *info;
  const SDL_VideoInfo *sdl_videoinfo;
  SDL_Rect** modes;
  info=(displaythread_info_t*)display_service->data;
  //SDL_Surface *icon_surface;

  info->sdlbpp=16;
  info->sdlflags=SDL_ANYFORMAT | SDL_RESIZABLE;

  // Initialize the SDL library (video subsystem)
  if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1) {
    Error("Couldn't initialize SDL video subsystem");
    return(0);
  }
  
  sdl_videoinfo = SDL_GetVideoInfo();
  
  if ((xvinfo.max_width!=-1)&&(xvinfo.max_height!=-1)) {
    // if the XV area is too small, we use software accelleration
    if ((xvinfo.max_width<display_service->current_buffer->frame.size[0])||
	(xvinfo.max_height<display_service->current_buffer->frame.size[1])) {
      //fprintf(stderr,"Using SW surface\n");
      info->sdlflags|= SDL_SWSURFACE;
      info->sdlflags&= ~SDL_HWSURFACE;
      info->sdlflags&= ~SDL_HWACCEL;
    }
    else {
      //fprintf(stderr,"Using HW surface\n");
      info->sdlflags|= SDL_HWSURFACE;
      info->sdlflags|= SDL_HWACCEL;
      info->sdlflags&= ~SDL_SWSURFACE;
    }
  }
  else {
    // try HW accel and pray...
    info->sdlflags|= SDL_SWSURFACE;
    info->sdlflags&= ~SDL_HWSURFACE;
    info->sdlflags&= ~SDL_HWACCEL;
  }

  modes=SDL_ListModes(NULL,info->sdlflags);
  if (modes!=(SDL_Rect**)-1) {
    // not all resolutions are OK for this video card. For safety we switch to software accel
    Warning("No SDL mode available with hardware accel. Trying without HWSURFACE");
    info->sdlflags&= ~SDL_HWSURFACE;
    info->sdlflags&= ~SDL_HWACCEL;
    modes=SDL_ListModes(NULL,info->sdlflags);
    if (modes!=(SDL_Rect**)-1) {
      Error("No video modes available, even without hardware acceleration. Can't start SDL!");
      SDL_Quit();
      return(0);
    }
  }

  
  // set coriander icon
  //icon_surface=SDL_CreateRGBSurfaceFrom((void*)coriander_logo_xpm)
  //SDL_WM_SetIcon(icon_surface,NULL);
  

  info->sdlvideorect.x=0;
  info->sdlvideorect.y=0;
  info->sdlvideorect.w=display_service->current_buffer->frame.size[0];
  info->sdlvideorect.h=display_service->current_buffer->frame.size[1];

  // maximize display size to XV size if necessary
  if ((xvinfo.max_width!=-1)&&(xvinfo.max_height!=-1)) {
    if (info->sdlvideorect.w>xvinfo.max_width) {
      info->sdlvideorect.w=xvinfo.max_width;
    }
    if (info->sdlvideorect.h>xvinfo.max_height) {
      info->sdlvideorect.h=xvinfo.max_height;
    }
  }

  // Set requested video mode
  info->sdlbpp = SDL_VideoModeOK(info->sdlvideorect.w, info->sdlvideorect.h, info->sdlbpp, info->sdlflags);
  info->sdlvideo = SDL_SetVideoMode(info->sdlvideorect.w, info->sdlvideorect.h, info->sdlbpp, info->sdlflags);

  if (info->sdlvideo == NULL) {
    Error(SDL_GetError());
    SDL_Quit();
    return(0);
  }

  if (SDL_SetColorKey( info->sdlvideo, SDL_SRCCOLORKEY, 0x0) < 0 ) {
    Error(SDL_GetError());
  }
  
  // Show cursor
  SDL_ShowCursor(1);
  
  // set window title:
  SDL_WM_SetCaption(display_service->camera->prefs.name,display_service->camera->prefs.name);

  // this line broke everything for unknown reasons so I just remove it.
  //info->sdlvideo->format->BytesPerPixel=2;

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
    return(0);
  }

  SDLEventStartThread(display_service);

  return(1);
}


void
SDLDisplayArea(chain_t *display_service)
{
  displaythread_info_t *info=(displaythread_info_t*)display_service->data;
  unsigned char *pimage;
  int upper_left[2];
  int lower_right[2];
  int width;
  int tmp;
  register int i;
  register int j;

  pthread_mutex_lock(&watchthread_info.mutex_area);
  if (watchthread_info.draw==1) {
    //fprintf(stderr,"drawing\n");
    upper_left[0]=watchthread_info.pos[0];
    upper_left[1]=watchthread_info.pos[1];
    lower_right[0]=watchthread_info.pos[0]+watchthread_info.size[0]-1;
    lower_right[1]=watchthread_info.pos[1]+watchthread_info.size[1]-1;
    width=display_service->current_buffer->frame.size[0];
    
    if (lower_right[0]<upper_left[0]) {
      tmp=lower_right[0];
      lower_right[0]=upper_left[0];
      upper_left[0]=tmp;
    }
    if (lower_right[1]<upper_left[1]) {
      tmp=lower_right[1];
      lower_right[1]=upper_left[1];
      upper_left[1]=tmp;
    }

    //fprintf(stderr,"boundaries: %d %d %d %d\n",upper_left[0],upper_left[1],lower_right[0],lower_right[1]);

    pimage=info->sdloverlay->pixels[0];//+i*info->sdloverlay->pitches[0];
    for (i=upper_left[1];i<=lower_right[1];i++) {
      for (j=upper_left[0];j<=lower_right[0];j++) {
	pimage[((i*width+j)<<1)]=(unsigned char)(255-pimage[((i*width+j)<<1)]);
	pimage[((i*width+j)<<1)+1]=(unsigned char)(255-pimage[((i*width+j)<<1)+1]);
      }
    }
  } 
  pthread_mutex_unlock(&watchthread_info.mutex_area);
}

void
SDLDisplayPattern(chain_t *display_service)
{
  displaythread_info_t *info=(displaythread_info_t*)display_service->data;
  unsigned char *pimage;
  int sx = display_service->current_buffer->frame.size[0];
  int sy = display_service->current_buffer->frame.size[1];
  int y,u,v,is,ie,js,je;
  unsigned char block[4];
  register int i;
  register int j;

  is = (7*sx)/16; is = is-is%2;
  ie = (9*sx)/16; ie = ie-ie%2;
  js = (7*sy)/16;
  je = (9*sy)/16;

  RGB2YUV(display_service->camera->prefs.overlay_color_r,
	  display_service->camera->prefs.overlay_color_g,
	  display_service->camera->prefs.overlay_color_b,y,u,v);

  switch(display_service->camera->prefs.overlay_type) {
  case OVERLAY_TYPE_REPLACE:
    switch(preferences.overlay_byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      block[0]=y;
      block[1]=u;
      block[2]=y;
      block[3]=v;
      break;
    case DC1394_BYTE_ORDER_UYVY:
      block[0]=u;
      block[1]=y;
      block[2]=v;
      block[3]=y;
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order coding: %d\n",preferences.overlay_byte_order);
      break;
    }
    break;
  case OVERLAY_TYPE_RANDOM:
    switch(preferences.overlay_byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      block[0]=random()/(RAND_MAX/255);
      block[1]=random()/(RAND_MAX/255);
      block[2]=block[0];
      block[3]=random()/(RAND_MAX/255);
      break;
    case DC1394_BYTE_ORDER_UYVY:
      block[0]=random()/(RAND_MAX/255);
      block[1]=random()/(RAND_MAX/255);
      block[2]=random()/(RAND_MAX/255);
      block[3]=block[1];
      break;
    }
    break;
  case OVERLAY_TYPE_INVERT:
    memset(block, 0, sizeof(block));
    break;
  default:
    fprintf(stderr,"Invalid display overlay type: %d\n",display_service->camera->prefs.overlay_type);
    memset(block, 0, sizeof(block));
    break;
  }

  pthread_mutex_lock(&watchthread_info.mutex_area);
  pimage=info->sdloverlay->pixels[0];
  switch(display_service->camera->prefs.overlay_pattern) {
  case OVERLAY_PATTERN_OFF:
    break;
  case OVERLAY_PATTERN_RECTANGLE:
    if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
      j=js;
      for (i=is;i<=ie;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      j=je;
      for (i=is;i<=ie;i+=2) {
	INVPIX(pimage,  ((j*sx+i)<<1));
	INVPIX(pimage, (((j+1)*sx+i)<<1));
      }
      i=is;
      for (j=js;j<=je;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
      i=ie;
      for (j=js;j<=je;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
    }
    else {
      j=js;
      for (i=is;i<=ie;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      j=je;
      for (i=is;i<=ie;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j+1)*sx+i)<<1));
      }
      i=is;
      for (j=js;j<=je;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
      i=ie;
      for (j=js;j<=je;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
    }
    break;
  case OVERLAY_PATTERN_SMALL_CROSS:
    if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
      j=sy/2; 
      for (i=is;i<=ie;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      i=sx/2; i=i-i%2;
      for (j=js;j<=je;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
    }
    else {
      j=sy/2;
      for (i=is;i<=ie;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      i=sx/2; i=i-i%2;
      for (j=js;j<=je;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
    }
    break;
  case OVERLAY_PATTERN_LARGE_CROSS:
    if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
      j=sy/2;
      for (i=0;i<sx;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      i=sx/2; i=i-i%2;
      for (j=0;j<sy;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
    }
    else {
      j=sy/2;
      for (i=0;i<sx;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      i=sx/2; i=i-i%2;
      for (j=0;j<sy;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
    }
    break;
  case OVERLAY_PATTERN_GOLDEN_MEAN:
    if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
      j=sy/3;
      for (i=0;i<sx;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      j=(2*sy)/3;
      for (i=0;i<sx;i+=2) {
	INVPIX(pimage, ((j*sx+i)<<1));
	INVPIX(pimage, (((j-1)*sx+i)<<1));
      }
      i=sx/3;
      i=i-i%2;
      for (j=0;j<sy;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
      i=(2*sx)/3;
      i=i-i%2;
      for (j=0;j<sy;j++) {
	INVPIX(pimage, ((j*sx+i)<<1));
      }
    }
    else {
      j=sy/3;
      for (i=0;i<sx;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      j=(2*sy)/3;
      for (i=0;i<sx;i+=2) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
	REPLPIX(pimage, block, (((j-1)*sx+i)<<1));
      }
      i=sx/3;
      i=i-i%2;
      for (j=0;j<sy;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
      i=(2*sx)/3;
      i=i-i%2;
      for (j=0;j<sy;j++) {
	REPLPIX(pimage, block, ((j*sx+i)<<1));
      }
    }
    break;
  case OVERLAY_PATTERN_IMAGE:
    break;
  case OVERLAY_PATTERN_OVER_UNDEREXPOSED:
    switch(preferences.overlay_byte_order) {
    case DC1394_BYTE_ORDER_YUYV:
      if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
	for (i=0;i<sx;i+=2) {
	  for (j=0;j<sy;j++) {
	    if ((pimage[(j*sx+i)<<1]>254)||(pimage[(j*sx+i)<<1]<1)||
		(pimage[(j*sx+i+1)<<1]>254)||(pimage[(j*sx+i+1)<<1]<1)) {
	      INVPIX(pimage, ((j*sx+i)<<1));
	    }
	  }
	}
      }
      else {
	for (i=0;i<sx;i+=2) {
	  for (j=0;j<sy;j++) {
	    if ((pimage[(j*sx+i)<<1]>254)||(pimage[(j*sx+i)<<1]<1)||
		(pimage[(j*sx+i+1)<<1]>254)||(pimage[(j*sx+i+1)<<1]<1)) {
	      REPLPIX(pimage, block, ((j*sx+i)<<1));
	    }
	  }
	}
      }
      break;
    case DC1394_BYTE_ORDER_UYVY:
      if (display_service->camera->prefs.overlay_type==OVERLAY_TYPE_INVERT) {
	for (i=0;i<sx;i+=2) {
	  for (j=0;j<sy;j++) {
	    if ((pimage[((j*sx+i)<<1)+1]>254)||(pimage[((j*sx+i)<<1)+1]<1)||
		(pimage[((j*sx+i+1)<<1)+1]>254)||(pimage[((j*sx+i+1)<<1)+1]<1)) {
	      INVPIX(pimage, ((j*sx+i)<<1));
	    }
	  }
	}
      }
      else {
	for (i=0;i<sx;i+=2) {
	  for (j=0;j<sy;j++) {
	    if ((pimage[((j*sx+i)<<1)+1]>254)||(pimage[((j*sx+i)<<1)+1]<1)||
		(pimage[((j*sx+i+1)<<1)+1]>254)||(pimage[((j*sx+i+1)<<1)+1]<1)) {
	      REPLPIX(pimage, block, ((j*sx+i)<<1));
	    }
	  }
	}
      }
      break;
    default:
      fprintf(stderr,"Invalid overlay byte order\n");
    }
    break;
  default:
    fprintf(stderr,"Wrong overlay pattern ID\n");
    break;
  }
  pthread_mutex_unlock(&watchthread_info.mutex_area);
}

void
SDLQuit(chain_t *display_service)
{
  displaythread_info_t *info;
  info=(displaythread_info_t*)display_service->data;

#ifdef HAVE_SDLLIB
  // if width==-1, SDL was never initialized so we do nothing
  if (display_service->current_buffer->frame.size[0]!=-1) {
    SDLEventStopThread(display_service);
    SDL_FreeYUVOverlay(info->sdloverlay);
    SDL_FreeSurface(info->sdlvideo);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
  }
#endif
}

void
DisplayThreadCheckParams(chain_t *display_service)
{

  displaythread_info_t *info;
  int first_time=0;
  //int size_change;
  int prev_image_size[2];
  int prev_overlay_size[2];
  info=(displaythread_info_t*)display_service->data;
  
  if (display_service->current_buffer->frame.size[0]==-1)
    fprintf(stderr,"Error: display size: %dx%d\n",
	    display_service->current_buffer->frame.size[0],
	    display_service->current_buffer->frame.size[1]);

  //fprintf(stderr,"check params\n");
  // if some parameters changed, we need to restart the display
  //fprintf(stderr,"Before: [%d %d] After: [%d %d]\n",
  //	  display_service->local_param_copy.frame.size[0],display_service->local_param_copy.frame.size[1],
  //	  display_service->current_buffer->frame.size[0],display_service->current_buffer->frame.size[1]);
  if ((display_service->current_buffer->frame.size[0]!=display_service->local_param_copy.frame.size[0])||
      (display_service->current_buffer->frame.size[1]!=display_service->local_param_copy.frame.size[1])   ) {

    first_time=((display_service->local_param_copy.frame.size[0]==-1)&&(display_service->current_buffer->frame.size[0]!=-1));
    //if (first_time>0)
    //fprintf(stderr,"  first frame...\n");
      
    prev_image_size[0]=display_service->local_param_copy.frame.size[0];
    prev_image_size[1]=display_service->local_param_copy.frame.size[1];

    // alternating between thos two methods may give a clue (don't forget deal with the 'else memcpy' below) ///////////////////////////////////////////
    memcpy(&display_service->local_param_copy,display_service->current_buffer,sizeof(buffer_t));
    //display_service->local_param_copy.frame.size[0]=display_service->current_buffer->frame.size[0];
    //display_service->local_param_copy.frame.size[1]=display_service->current_buffer->frame.size[1];

    // DO SOMETHING
    //fprintf(stderr,"  fs: %d\n",display_service->local_param_copy.frame.size[0]);
    // if the width is not -1, that is if some image has already reached the thread and the size has changed
    if (display_service->local_param_copy.frame.size[0]!=-1) { //&&(size_change!=0)) {
      //fprintf(stderr,"   do something...\n");
      if (first_time) {
	SDLInit(display_service);
      } else {

	//usleep(1000000);

	// note: in order to preserve the previous scaling and ratio, the previous parameters are used to
	// determine the new size of the display area
	prev_overlay_size[0]=info->sdlvideorect.w;
	prev_overlay_size[1]=info->sdlvideorect.h;
	watchthread_info.draw=0;
	//SDLEventStopThread(display_service);
	SDLResizeDisplay(display_service,
			 display_service->current_buffer->frame.size[0]*prev_overlay_size[0]/prev_image_size[0],
			 display_service->current_buffer->frame.size[1]*prev_overlay_size[1]/prev_image_size[1]);
	//SDLEventStartThread(display_service);
      }
    }
    //fprintf(stderr,"SDL updated.\n");
  }

  memcpy(&display_service->local_param_copy,display_service->current_buffer,sizeof(buffer_t));
  display_service->local_param_copy.frame.allocated_image_bytes=0;
}

#endif
