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

void
LoadConfigFile(void)
{
  
  preferences.camprefs.display_keep_ratio   = gnome_config_get_int("coriander/display/keep_ratio=0");
  preferences.camprefs.display_period       = gnome_config_get_int("coriander/display/period=1");
  preferences.camprefs.display_redraw       = gnome_config_get_int("coriander/display/redraw=1");
  preferences.camprefs.overlay_color_r      = gnome_config_get_int("coriander/display/overlay_color_r=128");
  preferences.camprefs.overlay_color_g      = gnome_config_get_int("coriander/display/overlay_color_g=128");
  preferences.camprefs.overlay_color_b      = gnome_config_get_int("coriander/display/overlay_color_b=128");
  preferences.camprefs.overlay_type         = gnome_config_get_int("coriander/display/overlay_type=0");
  preferences.camprefs.overlay_pattern      = gnome_config_get_int("coriander/display/overlay_pattern=0");
  preferences.camprefs.display_redraw_rate  = gnome_config_get_float("coriander/display/redraw_rate=4.0");
  preferences.camprefs.dma_buffer_size      = gnome_config_get_int("coriander/receive/dma_buffer_size=10");
  preferences.camprefs.iso_nodrop           = gnome_config_get_int("coriander/receive/iso_nodrop=0");
  preferences.camprefs.save_format          = gnome_config_get_int("coriander/save/format=0");
  preferences.camprefs.save_period          = gnome_config_get_int("coriander/save/period=1");
  preferences.camprefs.save_append          = gnome_config_get_int("coriander/save/append=0");
  preferences.camprefs.save_to_dir          = gnome_config_get_int("coriander/save/save_to_dir=0");
  preferences.camprefs.save_to_stdout       = gnome_config_get_int("coriander/save/save_to_stdout=0");
  preferences.camprefs.use_ram_buffer       = gnome_config_get_int("coriander/save/use_ram_buffer=0");
  preferences.camprefs.ram_buffer_size      = gnome_config_get_int("coriander/save/ram_buffer_size=100");
  preferences.camprefs.ftp_mode             = gnome_config_get_int("coriander/ftp/mode=0");
  preferences.camprefs.ftp_period           = gnome_config_get_int("coriander/ftp/period=1");
  preferences.camprefs.ftp_datenum          = gnome_config_get_int("coriander/ftp/datenum=1");
  preferences.camprefs.v4l_period           = gnome_config_get_int("coriander/v4l/period=1");
  preferences.camprefs.broadcast            = gnome_config_get_int("coriander/global/broadcast=0");
    
  preferences.camprefs.save_filename    = gnome_config_get_string("coriander/save/filename=test");
  preferences.camprefs.overlay_filename = gnome_config_get_string("coriander/display/overlay_filename=test.png");
  preferences.camprefs.ftp_filename     = gnome_config_get_string("coriander/ftp/filename=");
  preferences.camprefs.ftp_path         = gnome_config_get_string("coriander/ftp/path=");
  preferences.camprefs.ftp_address      = gnome_config_get_string("coriander/ftp/address=");
  preferences.camprefs.ftp_user         = gnome_config_get_string("coriander/ftp/user=username");
  preferences.camprefs.v4l_dev_name     = gnome_config_get_string("coriander/v4l/v4l_dev_name=/dev/video0");
  preferences.camprefs.ftp_password     = "";

  preferences.op_timeout                = gnome_config_get_float("coriander/global/one_push_timeout=10.0");
  preferences.auto_update               = gnome_config_get_int("coriander/global/auto_update=1");
  preferences.auto_update_frequency     = gnome_config_get_float("coriander/global/auto_update_frequency=2.0");
  preferences.overlay_byte_order        = gnome_config_get_int("coriander/global/overlay_byte_order=800");
  // for compatibilty with older libdc
  if (preferences.overlay_byte_order==0)
    preferences.overlay_byte_order=DC1394_BYTE_ORDER_UYVY;
  if (preferences.overlay_byte_order==1)
    preferences.overlay_byte_order=DC1394_BYTE_ORDER_YUYV;
  preferences.no_overwrite              = gnome_config_get_int("coriander/global/no_overwrite=1");
  preferences.warning_in_popup          = gnome_config_get_int("coriander/global/warning_in_popup=0");
  preferences.error_in_popup            = gnome_config_get_int("coriander/global/error_in_popup=1");
  preferences.automate_receive          = gnome_config_get_int("coriander/global/automate_receive=1");
  preferences.automate_iso              = gnome_config_get_int("coriander/global/automate_iso=1");
  //fprintf(stderr,"Buffer size read as: %d\n",preferences.camprefs.ram_buffer_size);

}

void
CopyCameraPrefs(camera_t* cam) {

  char *tmp, *tmp_ptr;

  cam->prefs.display_keep_ratio     = preferences.camprefs.display_keep_ratio;
  cam->prefs.display_period         = preferences.camprefs.display_period;
  cam->prefs.display_redraw         = preferences.camprefs.display_redraw;
  cam->prefs.display_redraw_rate    = preferences.camprefs.display_redraw_rate;
  cam->prefs.overlay_color_r        = preferences.camprefs.overlay_color_r;
  cam->prefs.overlay_color_g        = preferences.camprefs.overlay_color_g;
  cam->prefs.overlay_color_b        = preferences.camprefs.overlay_color_b;
  cam->prefs.overlay_type           = preferences.camprefs.overlay_type;
  cam->prefs.overlay_pattern        = preferences.camprefs.overlay_pattern;
  cam->prefs.iso_nodrop             = preferences.camprefs.iso_nodrop;
  cam->prefs.dma_buffer_size        = preferences.camprefs.dma_buffer_size;
  cam->prefs.save_format            = preferences.camprefs.save_format;
  cam->prefs.save_period            = preferences.camprefs.save_period;
  cam->prefs.save_append            = preferences.camprefs.save_append;
  cam->prefs.save_to_dir            = preferences.camprefs.save_to_dir;
  cam->prefs.save_to_stdout         = preferences.camprefs.save_to_stdout;
  cam->prefs.use_ram_buffer         = preferences.camprefs.use_ram_buffer;
  cam->prefs.ram_buffer_size        = preferences.camprefs.ram_buffer_size;
  cam->prefs.ftp_mode               = preferences.camprefs.ftp_mode;
  cam->prefs.ftp_period             = preferences.camprefs.ftp_period;
  cam->prefs.ftp_datenum            = preferences.camprefs.ftp_datenum;
  cam->prefs.v4l_period             = preferences.camprefs.v4l_period;
  cam->prefs.broadcast              = preferences.camprefs.broadcast;
  strcpy(cam->prefs.save_filename   , preferences.camprefs.save_filename);
  strcpy(cam->prefs.overlay_filename, preferences.camprefs.overlay_filename);
  strcpy(cam->prefs.ftp_filename    , preferences.camprefs.ftp_filename);
  strcpy(cam->prefs.ftp_path        , preferences.camprefs.ftp_path);
  strcpy(cam->prefs.ftp_address     , preferences.camprefs.ftp_address);
  strcpy(cam->prefs.ftp_user        , preferences.camprefs.ftp_user);
  strcpy(cam->prefs.v4l_dev_name    , preferences.camprefs.v4l_dev_name);
  preferences.camprefs.ftp_password = "";
  //fprintf(stderr,"Buffer size copied to: %d\n",cam->prefs.ram_buffer_size);
  

  //FIXME: update for multi-unit cameras
  tmp=(char*)malloc(STRING_SIZE*sizeof(char));
  sprintf(tmp,"coriander/camera_names/%llx=%s %s",cam->camera_info->guid,
	  cam->camera_info->vendor, cam->camera_info->model);
  tmp_ptr=gnome_config_get_string(tmp);
  strcpy(cam->prefs.name,tmp_ptr);
  free(tmp);

}
