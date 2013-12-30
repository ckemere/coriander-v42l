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

#ifndef __CORIANDER_H__
#define __CORIANDER_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <gnome.h>
#include <libraw1394/raw1394.h>
#include <libraw1394/csr.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <dc1394/dc1394.h>
#ifdef HAVE_GET_PORT
#include <dc1394/linux/control.h>
#endif

// FTP lib
#ifdef HAVE_FTPLIB
#include <ftplib.h>
#endif

// this is for the nice icon display
#ifdef HAVE_GDK_PIXBUF
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#endif

#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/times.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>

#include "definitions.h"
#include "videodev.h"

// SDL lib
#ifdef HAVE_SDLLIB
#include "SDL.h"
#include "SDLEvent.h"
#endif

// X11 includes for the simplified XVinfo
#ifdef HAVE_XV
#include <X11/Xlib.h>
#include <X11/extensions/Xvlib.h>
#include <ctype.h>
#endif

#ifdef HAVE_FFMPEG
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
#endif

#include "video_encode.h"

#include "error.h"
#include "build_frames.h"
#include "build_menus.h"
#include "build_ranges.h"
#include "build_windows.h"
#include "callbacks.h"
#include "camera.h"
#include "conversions.h"
#include "interface.h"
#include "preferences.h"
#include "support.h"
#include "video_encode.h"
#include "subtitles.h"
#include "thread_base.h"
#include "thread_display.h"
#include "thread_iso.h"
#include "thread_save.h"
#include "thread_ftp.h"
#include "thread_v4l.h"
#include "tools.h"
#include "update_frames.h"
#include "update_ranges.h"
#include "update_windows.h"
#include "watch_thread.h"

GtkWidget *main_window;
GtkWidget *about_window;
GtkWidget *help_window;
GtkWidget *preferences_window;
GtkWidget *waiting_camera_window;
CtxtInfo_t ctxt;
Prefs_t preferences;
int silent_ui_update;
camera_t* camera;
camera_t* cameras;
dc1394_t *dc1394;

xvinfo_t xvinfo;
int port_num;
raw1394handle_t *handles;

unsigned int format7_tab_presence;
unsigned int main_timeout_ticker;
unsigned int WM_cancel_display;
/*
unsigned int message_request;
pthread_mutex_t message_mutex;
char message_string[STRING_SIZE];
unsigned int message_type;
unsigned int message_clicked;
*/
#ifdef HAVE_SDLLIB
cursor_info_t cursor_info;
watchthread_info_t watchthread_info;
#endif

mainthread_info_t mainthread_info;

extern const char *feature_name_list[DC1394_FEATURE_NUM];
extern const char *feature_abs_switch_list[DC1394_FEATURE_NUM];
extern const char *feature_abs_label_list[DC1394_FEATURE_NUM];
extern const char *trigger_mode_list[DC1394_TRIGGER_MODE_NUM];
extern const char *trigger_source_list[DC1394_TRIGGER_SOURCE_NUM];
extern const char *channel_num_list[16];
extern const char *fps_label_list[DC1394_FRAMERATE_NUM];
extern const char *format7_mode_list[DC1394_VIDEO_MODE_FORMAT7_NUM];
extern const char *format7_color_list[DC1394_COLOR_CODING_NUM];
extern const char *phy_speed_list[7];
extern const char *power_class_list[8];
extern const char *phy_delay_list[4];
extern const char *video_mode_list[DC1394_VIDEO_MODE_NUM];
extern const char *feature_menu_items_list[5];
extern const char *help_key_bindings_keys[KEY_BINDINGS_NUM];
extern const char *help_key_bindings_functions[KEY_BINDINGS_NUM];

#endif // __CORIANDER_H__
