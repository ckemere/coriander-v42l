/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#ifndef __THREAD_SAVE_H__
#define __THREAD_SAVE_H__

typedef enum
{
  SAVE_FORMAT_RAW=0,
  SAVE_FORMAT_PPM,
#ifdef HAVE_FFMPEG
  SAVE_FORMAT_JPEG,
#endif
  SAVE_FORMAT_RAW_VIDEO,
#ifdef HAVE_FFMPEG
  SAVE_FORMAT_PVN,
  SAVE_FORMAT_MPEG
#else
  SAVE_FORMAT_PVN
#endif

} save_format_t;

#define SAVE_FORMAT_FIRST_VIDEO SAVE_FORMAT_RAW_VIDEO

typedef enum
{
  SAVE_APPEND_NUMBER=0,
  SAVE_APPEND_DATE_TIME,
  SAVE_APPEND_NONE
} save_append_t;

typedef struct
{ 
  /* Define thread variables here.
     This data will only de available to the thread. */

  pthread_mutex_t    mutex_cancel;
  int                cancel_req;
  char               destdir[STRING_SIZE];
  dc1394video_frame_t frame;
  unsigned char*     bigbuffer;
  unsigned long int  bigbuffer_position;
  int                mpeg_color_mode;

#ifdef HAVE_FFMPEG
  //MPEG encoding data
  AVOutputFormat *fmt;
  AVFormatContext *oc;
  AVStream *video_st;
  AVFrame *picture;
  AVFrame *tmp_picture;
  char subtitle[256];
  int fdts;
#endif

} savethread_info_t;

gint
SaveStartThread(camera_t* cam);

void*
SaveThread(void* arg);

gint
SaveStopThread(camera_t* cam);

void
SaveThreadCheckParams(chain_t *save_service);

int
isColor(int buffer_color_mode);

int
needsConversionForPVN(int buffer_color_mode);

int
getConvertedBytesPerChannel(int buffer_color_mode);

int
getBytesPerChannel(int buffer_color_mode);

double
framerateAsDouble(int framerate_enum);

float
getAvgBytesPerPixel(int buffer_color_mode);

unsigned int
getDepth(unsigned long bufsize, int mode, unsigned int height, unsigned int width);

int
InitVideoFile(chain_t *save_service, FILE *fd, char *filename_out);

void
writePVNHeader(FILE *fd, unsigned int mode, unsigned int width, unsigned int height,
	       unsigned int depth, unsigned int bpp, double framerate);

int
GetSaveFD(chain_t *save_service, FILE **fd, char *filename_out);

void
FillRamBuffer(chain_t *save_service);


#endif // __THREAD_SAVE_H__
