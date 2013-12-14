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

#ifndef __THREAD_FTP_H__
#define __THREAD_FTP_H__

typedef enum
{
  FTP_MODE_SEQUENTIAL=0,
  FTP_MODE_OVERWRITE
} ftp_mode_t;
 
typedef enum
{
  FTP_TAG_DATE=0,
  FTP_TAG_NUMBER
} ftp_tag_t;
 
typedef struct
{ 
  /* Define thread variables here.
     This data will only de available to the thread.*/

  pthread_mutex_t    mutex_cancel;
  int                cancel_req;
  //char               filename[STRING_SIZE];
  //char               filename_ext[STRING_SIZE];
  //char               address[STRING_SIZE];
  //char               password[STRING_SIZE];
  //char               user[STRING_SIZE];
  //char               path[STRING_SIZE];
  //long int           period;
  //long int           imlib_buffer_size;
  dc1394video_frame_t frame;
  //int                mode;
  //int                datenum;
#ifdef HAVE_FTPLIB
  netbuf             *ftp_handle;
#endif

} ftpthread_info_t;

gint
FtpStartThread(camera_t* cam);

void*
FtpThread(void* arg);

gint
FtpStopThread(camera_t* cam);

void
FtpThreadCheckParams(chain_t *ftp_service);

#ifdef HAVE_FTPLIB

gboolean
OpenFtpConnection(chain_t *ftp_service);

// V20***
//gboolean
//FtpPutFrame(char *filename, GdkImlibImage *im, ftpthread_info_t* info);

void
CloseFtpConnection(netbuf *ftp_handle);

gboolean
CheckFtpConnection(chain_t *ftp_service);

#endif

#endif // __THREAD_FTP_H__
