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

#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#define  BU                  1000     // definitions for distinguishing the BU and RV scales of the FEATURE_WHITE_BALANCE feature
#define  RV                  2000    
#define  SHADINGR            1000     // definitions for distinguishing the R, G and B scales of the FEATURE_WHITE_SHADING feature
#define  SHADINGG            2000     
#define  SHADINGB            3000     
#define  NO                 FALSE
#define  YES                 TRUE
#define  STRING_SIZE         1024
#define  KEY_BINDINGS_NUM      10
#define  DELAY              50000

enum  {
  RANGE_MENU_OFF = 0,
  RANGE_MENU_MAN,
  RANGE_MENU_AUTO,
  RANGE_MENU_SINGLE,
  RANGE_MENU_ABSOLUTE
};

enum {
  FORMAT7_SIZE_X = 0,
  FORMAT7_SIZE_Y,
  FORMAT7_POS_X,
  FORMAT7_POS_Y,
  FORMAT7_PACKET
};

enum
{
  REGISTER_OFFSET_CRB=0,
  REGISTER_OFFSET_UD,
  REGISTER_OFFSET_UDD,
  REGISTER_OFFSET_BASE,
  REGISTER_OFFSET_PIO,
  REGISTER_OFFSET_SIO,
  REGISTER_OFFSET_STROBE
};

typedef struct _mainthread_info {
  pthread_t thread;
  pthread_mutex_t do_mutex;
  void *(*do_function)(void *);
  void *do_arg;
  void *do_result;
  int dialog_clicked;
  pthread_mutex_t dialog_mutex;
  char message[1024];
} mainthread_info_t;


#define NO_BAYER_DECODING -1

typedef struct _CamPrefs
{
  int display_keep_ratio;
  int display_period;
  int display_redraw;
  float display_redraw_rate;
  int iso_nodrop;
  int dma_buffer_size;
  char *save_filename;
  char *save_filename_ext;
  int save_format;
  int save_append;
  int save_period;
  int save_to_dir;
  int save_to_stdout;
  int use_ram_buffer;
  unsigned int ram_buffer_size;
  char *ftp_address;
  char *ftp_user;
  char *ftp_password;
  char *ftp_filename;
  char *ftp_filename_ext;
  char *ftp_filename_base;
  char *ftp_path;
  int ftp_mode;
  int ftp_period;
  int ftp_datenum;

  int v4l_period;
  char *v4l_dev_name;

  char* name;

  char *overlay_filename;
  int overlay_type;
  int overlay_color_r;
  int overlay_color_g;
  int overlay_color_b;
  int overlay_pattern;

  int broadcast;

  // internal data:
  int receive_method2index[2];

} PrefsCam_t;

typedef struct _Prefs
{
  float op_timeout;
  int auto_update;
  float auto_update_frequency;
  int no_overwrite;
  int warning_in_popup;
  int error_in_popup;
  int automate_receive;
  int automate_iso;
  int overlay_byte_order;

  struct _CamPrefs camprefs;

} Prefs_t;

typedef enum _Service_T
{
  SERVICE_ISO=0,
  SERVICE_DISPLAY,
  SERVICE_SAVE,
  SERVICE_V4L,
  SERVICE_FTP
} service_t;

typedef struct _Format7Info
{
  dc1394format7modeset_t modeset;
  int edit_mode;
  
  int scale_posx_handle;
  int scale_posy_handle;
  int scale_sizex_handle;
  int scale_sizey_handle;
  
} Format7Info_t;

typedef struct _CameraInfo_T {
  dc1394camera_t *camera_info;
  dc1394featureset_t feature_set;
  struct _Chain_T* image_pipe;
  Format7Info_t format7_info;

  // old uiinfo
  pthread_mutex_t uimutex;
  int want_to_display;
  int bayer;
  int bayer_pattern;
  int stereo;
  unsigned int bpp;
  int register_offset;
  int memory_channel;

  // structure information
  struct _CameraInfo_T* prev;
  struct _CameraInfo_T* next;

  PrefsCam_t prefs;

} camera_t;

typedef struct _Buffer_T
{
  dc1394video_frame_t frame;
  unsigned int bayer;
  unsigned int stereo;
  dc1394stereo_method_t stereo_method;
  dc1394bayer_method_t  bayer_method;

  char captime_string[19];

  //int used; // set to 1 if the buffer has already been used by a service. Only the last thread can do
            // this change. The ISO thread can be set to take that flag into account or not. EFFECT:
            // avoids framedrop in association with a sufficiently large DMA buffer
} buffer_t;

typedef struct _Chain_T
{
  pthread_mutex_t mutex_struct; // below is protected by mutex_struct
  struct _Chain_T* next_chain;
  struct _Chain_T* prev_chain;
  int             updated;
  service_t       service;

  pthread_mutex_t mutex_data; // below is protected by mutex_data
  pthread_t       thread;
  buffer_t*       next_buffer;
  buffer_t*       current_buffer;
  void*           data;
  buffer_t        local_param_copy; // not pointer: it remains in the chain.

  // timing data:

  float fps;

  uint64_t prev_time;
  float prev_period;
  uint32_t drop_warning;

  unsigned long long int processed_frames;

  camera_t*       camera; // the camera that uses this thread    

  int ready;// set to 1 if the ISO service can grab another picture from the camera. Only the last thread can do
            // this change. The ISO thread can be set to take that flag into account or not. EFFECT:
            // avoids framedrop in association with a sufficiently large DMA buffer
} chain_t;

/*
typedef struct _UIInfo
{
  pthread_mutex_t mutex;
  int want_to_display;
  int bayer;
  int bayer_pattern;
  int stereo;
  int bpp;

} uiinfo_t;
*/

typedef struct _StatusInfo
{
  char name[STRING_SIZE];
  quadlet_t guid;
  
} StatusInfo_t;

typedef struct _CtxtInfo
{
    int model_ctxt;
    int port_node_ctxt;
    int guid_ctxt;
    //int handle_ctxt;
    int vendor_ctxt;
    int max_iso_ctxt;
    //int delay_ctxt;
    int dc_ctxt;
    //int pwclass_ctxt;
    int iso_channel_ctxt;
    int iso_status_ctxt;
    int main_ctxt;
    
    int model_id;
    int port_node_id;
    int guid_id;
    //int handle_id;
    int vendor_id;
    int max_iso_id;
    //int delay_id;
    int dc_id;
    //int pwclass_id;
    int iso_channel_id;
    int iso_status_id;
    int main_id;
    
    int cursor_pos_ctxt;
    int cursor_yuv_ctxt;
    int cursor_rgb_ctxt;
    
    int cursor_pos_id;
    int cursor_rgb_id;
    int cursor_yuv_id;
    
    int fps_receive_ctxt;
    int fps_receive_id;
    int fps_display_ctxt;
    int fps_display_id;
    int fps_save_ctxt;
    int fps_save_id;
    int fps_ftp_ctxt;
    int fps_ftp_id;
    int fps_v4l_ctxt;
    int fps_v4l_id;
    
    int save_filename_ctxt;
    int save_filename_id;
    
    int format7_imagebytes_id;
    int format7_totalbytes_id;
    int format7_imagepixels_id;
    int format7_padding_id;
    int format7_imagebytes_ctxt;
    int format7_totalbytes_ctxt;
    int format7_imagepixels_ctxt;
    int format7_padding_ctxt;

} CtxtInfo_t;


#endif // __DEFINITIONS_H__

