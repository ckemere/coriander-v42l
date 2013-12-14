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

#ifndef __THREAD_ISO_H__
#define __THREAD_ISO_H__

typedef enum
{
  RECEIVE_METHOD_RAW1394=0, 
  RECEIVE_METHOD_VIDEO1394
} receive_method_t;

typedef struct
{ 
  pthread_mutex_t    mutex_cancel;
  int                cancel_req;

  dc1394video_frame_t tempframe;
  int orig_sizex;
  int orig_sizey;
  int cond16bit;

} isothread_info_t;

gint
IsoStartThread(camera_t* cam);

void*
IsoThread(void* arg);

gint
IsoStopThread(camera_t* camera);

void
IsoThreadCheckParams(chain_t *iso_service);

void
AllocTempBuffer(long long unsigned int requested_size, isothread_info_t* info);

void
AllocImageBuffer(chain_t* iso_service);

void
SetColorMode(int mode, buffer_t *buffer, int f7_color);

#endif // __THREAD_ISO_H__


