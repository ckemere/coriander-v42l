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

#ifndef __WATCH_THREAD_H__
#define __WATCH_THREAD_H__

#ifdef HAVE_SDLLIB

typedef struct
{
  pthread_t       thread;
  pthread_mutex_t mutex_cancel_watch;
  int             cancel_watch_req;

  // area drawing:
  pthread_mutex_t mutex_area;
  int             upper_left[2];
  int             lower_right[2];
  int             first[2];
  int             second[2];
  int             pos[2];
  int             size[2];
  int             draw;
  int             mouse_down;
  int             crop;
  int             abs_pos[2];

} watchthread_info_t;


int
WatchStartThread(watchthread_info_t* info);

void*
WatchThread(void *arg);

int
WatchStopThread(watchthread_info_t* info);

void
GetValidF7Crop(watchthread_info_t *wtinfo, chain_t *display_service);

#endif

#endif // __WATCH_THREAD_H__
