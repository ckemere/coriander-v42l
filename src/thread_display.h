/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#ifndef __THREAD_DISPLAY_H__
#define __THREAD_DISPLAY_H__

typedef enum
{
  DISPLAY_REDRAW_OFF=0,
  DISPLAY_REDRAW_ON
} display_redraw_t;

enum
{
  OVERLAY_PATTERN_OFF=0,
  OVERLAY_PATTERN_RECTANGLE,
  OVERLAY_PATTERN_SMALL_CROSS,
  OVERLAY_PATTERN_LARGE_CROSS,
  OVERLAY_PATTERN_GOLDEN_MEAN,
  OVERLAY_PATTERN_OVER_UNDEREXPOSED,
  OVERLAY_PATTERN_IMAGE
};

enum
{
  OVERLAY_TYPE_REPLACE=0,
  OVERLAY_TYPE_RANDOM,
  OVERLAY_TYPE_INVERT,
  OVERLAY_TYPE_AVERAGE
};

#define REPLPIX(im, pix, index)\
  im[index]=pix[0];\
  im[index+1]=pix[1];\
  im[index+2]=pix[2];\
  im[index+3]=pix[3]

#define INVPIX(im, index)\
  im[index]=255-im[index];\
  im[index+1]=255-im[index+1];\
  im[index+2]=255-im[index+2];\
  im[index+3]=255-im[index+3]

typedef struct
{
  pthread_mutex_t         mutex_cancel;
  int                     cancel_req;

  struct tms redraw_tms_buf;
  clock_t redraw_prev_time;
  clock_t redraw_current_time;

#ifdef HAVE_SDLLIB

  // general SDL stuff
  long unsigned int sdlflags;
  int sdlbpp;
  SDL_Surface *sdlvideo;   // video surface
  SDL_Overlay *sdloverlay; // video overlay surface
  SDL_Rect sdlvideorect;   // video rectangle for overlay surface

  // events
  pthread_mutex_t         mutex_cancel_event;
  int                     cancel_event_req;
  pthread_mutex_t         mutex_event;
  pthread_t               event_thread;

#endif

} displaythread_info_t;

gint
DisplayStartThread(camera_t* cam);

void*
DisplayThread(void* arg);

gint
DisplayStopThread(camera_t* cam);

void
ConditionalTimeoutRedraw(chain_t* service);

#ifdef HAVE_SDLLIB

int
SDLInit(chain_t *display_service);

void
SDLDisplayArea(chain_t *display_service);

void
SDLDisplayPattern(chain_t *display_service);

void
SDLQuit(chain_t *display_service);

void
DisplayThreadCheckParams(chain_t *display_service);

#endif

#endif // __THREAD_DISPLAY_H__
