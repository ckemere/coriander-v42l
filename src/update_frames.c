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
extern cursor_info_t cursor_info;
#endif

void
UpdatePrefsGeneralFrame(void)
{
  // nothing yet. should update ranges
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(preferences_window,"prefs_update_power")),
			       preferences.auto_update);
}

void
UpdatePrefsDisplayFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(main_window,"display_redraw_rate"),
			   camera->prefs.display_redraw==DISPLAY_REDRAW_ON);
  gtk_widget_set_sensitive(lookup_widget(main_window,"label155a"),
			   camera->prefs.display_redraw==DISPLAY_REDRAW_ON);

  UpdatePrefsDisplayOverlayFrame();
}

void
UpdatePrefsReceiveFrame(void)
{
  // thread presence blanking: default some to ON
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_receive_frame"), TRUE);

  // thread presence blanking:
  if (GetService(camera,SERVICE_ISO)!=NULL)
    gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_receive_frame"), FALSE);
}

void
UpdatePrefsSaveFrame(void)
{
  int is_video=((camera->prefs.save_format==SAVE_FORMAT_PVN)||
#ifdef HAVE_FFMPEG
		(camera->prefs.save_format==SAVE_FORMAT_MPEG)||
#endif
		(camera->prefs.save_format==SAVE_FORMAT_RAW_VIDEO));

  // thread presence blanking: default some to ON
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_filename_frame"), TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"ram_buffer_frame"), TRUE);

  // normal:
  gtk_widget_set_sensitive(lookup_widget(main_window,"save_to_dir"), (!is_video)&&(!camera->prefs.save_to_stdout));
  gtk_widget_set_sensitive(lookup_widget(main_window,"ram_buffer_frame"), is_video);
  gtk_widget_set_sensitive(lookup_widget(main_window,"use_ram_buffer"), is_video);

  // thread presence blanking:
  if (GetService(camera,SERVICE_SAVE)!=NULL) {
    gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_save_filename_frame"), FALSE);
    gtk_widget_set_sensitive(lookup_widget(main_window,"ram_buffer_frame"), FALSE);
  }
  UpdateSaveFilenameFrame();

  // save to stdout
  gtk_widget_set_sensitive(lookup_widget(main_window,"label177"), !camera->prefs.save_to_stdout);
  gtk_widget_set_sensitive(lookup_widget(main_window,"label175"), !camera->prefs.save_to_stdout);
  gtk_widget_set_sensitive(lookup_widget(main_window,"label174"), !camera->prefs.save_to_stdout);
  gtk_widget_set_sensitive(lookup_widget(main_window,"save_filename_status"), !camera->prefs.save_to_stdout);
  gtk_widget_set_sensitive(lookup_widget(main_window,"save_append_menu"), !camera->prefs.save_to_stdout);
  gtk_widget_set_sensitive(lookup_widget(main_window,"save_filename_entry"), !camera->prefs.save_to_stdout);
  //gtk_widget_set_sensitive(lookup_widget(main_window,"save_to_dir"), !camera->prefs.save_to_stdout);

}


void
UpdatePrefsFtpFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_date_tag"),
			   camera->prefs.ftp_mode==FTP_MODE_SEQUENTIAL);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_num_tag"),
			   camera->prefs.ftp_mode==FTP_MODE_SEQUENTIAL);

  // thread presence blanking:
  if (GetService(camera,SERVICE_FTP)!=NULL) {
    gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_server_frame"),FALSE);
  }
  else {
    gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_server_frame"),TRUE);
  }
    
}

void
UpdatePrefsV4lFrame(void)
{
  // thread presence blanking:
  if (GetService(camera,SERVICE_V4L)!=NULL) {
    gtk_widget_set_sensitive(lookup_widget(main_window,"v4l_output_device_frame"),FALSE);
  }
  else {
    gtk_widget_set_sensitive(lookup_widget(main_window,"v4l_output_device_frame"),TRUE);
  }
}


void
UpdateCameraFrame(void)
{
  // should reprobe the bus for new cameras here??
}

void
UpdateTriggerFrame(void)
{
  dc1394video_mode_t video_mode;
  dc1394_video_get_mode(camera->camera_info,&video_mode);

  // always set the trigger frame on (because it contains the fps menu):
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_frame"),TRUE);

  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_external"),
			   camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].available);
  gtk_widget_set_sensitive(lookup_widget(main_window,"fps_menu"),
			   !(camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].is_on) &&
			   !((video_mode >= DC1394_VIDEO_MODE_FORMAT7_MIN) && (video_mode <= DC1394_VIDEO_MODE_FORMAT7_MAX)));
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_mode"),
			   camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].is_on && 
			   camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].available);
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_source"),
			   camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].is_on && 
			   camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].available &&
			   (camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_sources.num>0 ));
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_polarity"),
			   camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].available &&
			   camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].is_on &&
			   camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].polarity_capable);
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_count"),
			   (camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].available) &&
			   (camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].is_on) && 
			   ((camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_mode == DC1394_TRIGGER_MODE_2)||
			     camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_mode == DC1394_TRIGGER_MODE_3));

  gtk_widget_set_sensitive(lookup_widget(main_window, "label16"),
			   (camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].available) &&
			   (camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].is_on) && 
			   ((camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_mode == DC1394_TRIGGER_MODE_2)||
			     camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_mode == DC1394_TRIGGER_MODE_3));
}

void
UpdatePowerFrame(void)
{
  // nothing to update
}

void
UpdateMemoryFrame(void)
{
  gtk_widget_set_sensitive(lookup_widget(main_window,"memory_channel"),TRUE);

  // save not activated by default (it is not avail. for factory defaults channel):
  gtk_widget_set_sensitive(GTK_WIDGET (lookup_widget(main_window,"save_mem")),
			   ((camera->camera_info->max_mem_channel>0)&&(camera->memory_channel>0)));
 

  // load always present, so we can activate it:
  gtk_widget_set_sensitive(lookup_widget(main_window,"memory_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"load_mem"),TRUE);
}


void
UpdateIsoFrame(void)
{
  dc1394switch_t is_iso_on;
  dc1394_video_get_transmission(camera->camera_info,&is_iso_on);

  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_start"),!is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_restart"),is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_stop"),is_iso_on);
  //fprintf(stderr,"bmode_capable: %d\n",camera->camera_info->bmode_capable);
  gtk_widget_set_sensitive(lookup_widget(main_window,"bmode_button"),
			   camera->camera_info->bmode_capable &&
			   !is_iso_on );//    &&
                           // FIXME: capture_is_set not available anymore!
                           // !camera->camera_info->capture_is_set);
  gtk_widget_set_sensitive(lookup_widget(main_window,"isospeed_menu"),
			   !is_iso_on);//    &&
                           // FIXME: capture_is_set not available anymore!
			   // !camera->camera_info->capture_is_set);
}

void
UpdateFormat7ModeFrame(void)
{
  BuildFormat7ColorMenu();
  BuildFormat7ModeMenu();
  BuildFormat7Ranges();
}

void
UpdateCameraStatusFrame(void)
{
  char *temp;
  quadlet_t value[3];

  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  value[0]= camera->camera_info->guid & 0xffffffff;
  value[1]= (camera->camera_info->guid >>32) & 0x000000ff;
  value[2]= (camera->camera_info->guid >>40) & 0xfffff;

  // vendor:
  sprintf(temp," %s",camera->camera_info->vendor);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_vendor_status"), ctxt.vendor_ctxt, ctxt.vendor_id);
  ctxt.vendor_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"camera_vendor_status"), ctxt.vendor_ctxt, temp);

  // camera model:
  sprintf(temp," %s",camera->camera_info->model);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_model_status"), ctxt.model_ctxt, ctxt.model_id);
  ctxt.model_id=gtk_statusbar_push( (GtkStatusbar*)lookup_widget(main_window,"camera_model_status"), ctxt.model_ctxt, temp);

  // camera node/bus:
  unsigned int node, gen;
  dc1394_camera_get_node(camera->camera_info, &node, &gen);
#ifdef HAVE_GET_PORT
  unsigned int port;
  if (dc1394_camera_get_linux_port(camera->camera_info, &port)==DC1394_SUCCESS && port<port_num)
      sprintf(temp," %d  /  %d /  %d ", port, node, gen); 
  else
      sprintf(temp," - /  %d /  %d ", node, gen); 

#else
  sprintf(temp," - /  %d /  %d ", node, gen); 
#endif
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_port_node_status"), ctxt.port_node_ctxt, ctxt.port_node_id);
  ctxt.port_node_id=gtk_statusbar_push( (GtkStatusbar*)lookup_widget(main_window,"camera_port_node_status"), ctxt.port_node_ctxt, temp);

  // camera handle:
  // TODO TODO TODO
  // sprintf(temp," %p",camera->camera_info.handle);
  //sprintf(temp," N/A");
  //gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_handle_status"), ctxt.handle_ctxt, ctxt.handle_id);
  //ctxt.handle_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_handle_status"), ctxt.handle_ctxt, temp);

  // camera GUID / unit:
  sprintf(temp," 0x%06x-%02x%08x  /  %d", value[2], value[1], value[0], camera->camera_info->unit);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_guid_status"), ctxt.guid_ctxt, ctxt.guid_id);
  ctxt.guid_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_guid_status"), ctxt.guid_ctxt, temp);

  // camera maximal PHY speed:
  //sprintf(temp," N/A");
  //sprintf(temp," %s",phy_speed_list[camera->camera_info->phy_speed-DC1394_ISO_SPEED_MIN]);
  //gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_maxiso_status"), ctxt.max_iso_ctxt, ctxt.max_iso_id);
  //ctxt.max_iso_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_maxiso_status"), ctxt.max_iso_ctxt, temp);

  // camera maximal PHY delay:
  // FIXME: not available with latest API
  //sprintf(temp," %s",phy_delay_list[camera->camera_info->phy_delay-DC1394_PHY_DELAY_MIN]);
  //sprintf(temp," N/A");
  //gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_delay_status"), ctxt.delay_ctxt, ctxt.delay_id);
  //ctxt.delay_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_delay_status"), ctxt.delay_ctxt, temp);

  // IIDC software revision:
  switch (camera->camera_info->iidc_version) {
  case DC1394_IIDC_VERSION_1_04: sprintf(temp," 1.04 ");break;
  case DC1394_IIDC_VERSION_1_20: sprintf(temp," 1.20 ");break;
  case DC1394_IIDC_VERSION_1_30: sprintf(temp," 1.30 ");break;
  case DC1394_IIDC_VERSION_PTGREY: sprintf(temp," Pt Grey 114 ");break;
  case DC1394_IIDC_VERSION_1_31: sprintf(temp," 1.31 ");break;
  case DC1394_IIDC_VERSION_1_32: sprintf(temp," 1.32 ");break;
  case DC1394_IIDC_VERSION_1_33: sprintf(temp," 1.33 ");break;
  case DC1394_IIDC_VERSION_1_34: sprintf(temp," 1.34 ");break;
  case DC1394_IIDC_VERSION_1_35: sprintf(temp," 1.35 ");break;
  case DC1394_IIDC_VERSION_1_36: sprintf(temp," 1.36 ");break;
  case DC1394_IIDC_VERSION_1_37: sprintf(temp," 1.37 ");break;
  case DC1394_IIDC_VERSION_1_38: sprintf(temp," 1.38 ");break;
  case DC1394_IIDC_VERSION_1_39: sprintf(temp," 1.39 ");break;
  default: sprintf(temp," ?? %d ",camera->camera_info->unit_sw_version);
  }
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_dc_status"), ctxt.dc_ctxt, ctxt.dc_id);
  ctxt.dc_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_dc_status"), ctxt.dc_ctxt, temp);

  // power class:
  // FIXME: not available with latest API
  //sprintf(temp," %s",power_class_list[camera->camera_info->power_class-DC1394_POWER_CLASS_MIN]);
  //sprintf(temp," N/A");
  //gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"camera_pwclass_status"), ctxt.pwclass_ctxt, ctxt.pwclass_id);
  //ctxt.pwclass_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"camera_pwclass_status"), ctxt.pwclass_ctxt,temp);

  // camera name:
  //eprint("UpdateFrame: Entry was '%s', ",gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"camera_name_text"))));
  //eprint("Setting to '%s'\n",camera->prefs.name);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window,"camera_name_text")), camera->prefs.name);

  free(temp);

}


void
UpdateTransferStatusFrame(void)
{
  char *temp;
  uint32_t channel;
  dc1394switch_t iso;
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  dc1394_video_get_iso_channel(camera->camera_info, &channel);
  dc1394_video_get_transmission(camera->camera_info, &iso);
  if ((channel>=0)&&(iso==DC1394_ON)) {
      sprintf(temp," %d",channel);
  }
  else {
      sprintf(temp," N/A");
  }
  gtk_statusbar_remove( (GtkStatusbar*) lookup_widget(main_window,"iso_channel_status"), ctxt.iso_channel_ctxt, ctxt.iso_channel_id);
  ctxt.iso_channel_id=gtk_statusbar_push( (GtkStatusbar*) lookup_widget(main_window,"iso_channel_status"), ctxt.iso_channel_ctxt, temp);

  free(temp);
}

void
UpdateCursorFrame(void)
{
#ifdef HAVE_SDLLIB
  char *temp;
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  // position: 
  sprintf(temp," %d,%d",cursor_info.x,cursor_info.y);
  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"cursor_pos"), ctxt.cursor_pos_ctxt, ctxt.cursor_pos_id);
  ctxt.cursor_pos_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"cursor_pos"), ctxt.cursor_pos_ctxt, temp);

  // color:
  if (cursor_info.col_r>-255) {
    sprintf(temp," %03d,%03d,%03d",cursor_info.col_r,cursor_info.col_g,cursor_info.col_b);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"cursor_rgb"), ctxt.cursor_rgb_ctxt, ctxt.cursor_rgb_id);
    ctxt.cursor_rgb_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"cursor_rgb"), ctxt.cursor_rgb_ctxt, temp);
  }
  
  if (cursor_info.col_y>-255) {
    sprintf(temp," %03d,%03d,%03d",cursor_info.col_y,cursor_info.col_u,cursor_info.col_v);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"cursor_yuv"), ctxt.cursor_yuv_ctxt, ctxt.cursor_yuv_id);
      ctxt.cursor_yuv_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"cursor_yuv"), ctxt.cursor_yuv_ctxt, temp);
  }

  free(temp);
#endif
}

void
UpdateOptionFrame(void)
{
  int bayer_ok, stereo_ok, bpp16_ok;

  dc1394video_mode_t video_mode;
  dc1394_video_get_mode(camera->camera_info,&video_mode);

  pthread_mutex_lock(&camera->uimutex);
  gtk_widget_set_sensitive(lookup_widget(main_window,"pattern_menu"),
			   camera->bayer!=NO_BAYER_DECODING);
  pthread_mutex_unlock(&camera->uimutex);
  IsOptionAvailableWithFormat(camera,&bayer_ok, &stereo_ok, &bpp16_ok);

  gtk_widget_set_sensitive(lookup_widget(main_window,"pattern_menu"), bayer_ok);
  gtk_widget_set_sensitive(lookup_widget(main_window,"bayer_menu"), bayer_ok);
  gtk_widget_set_sensitive(lookup_widget(main_window,"stereo_menu"), stereo_ok);

  dc1394color_filter_t filter;
  // if we have a valid color filter, use that:
  if (dc1394_is_video_mode_scalable(video_mode)==DC1394_TRUE) {
    if (dc1394_format7_get_color_filter(camera->camera_info, video_mode, &filter)==DC1394_SUCCESS) {
      if (filter!=0) { // we have a valid filter: use it.
	//fprintf(stderr,"Valid filter!\n");
	camera->format7_info.modeset.mode[video_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].color_filter=filter;
	camera->bayer_pattern=filter;
	pthread_mutex_lock(&camera->uimutex);
	gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "pattern_menu")),
				    camera->bayer_pattern-DC1394_COLOR_FILTER_MIN);
	gtk_widget_set_sensitive(lookup_widget(main_window,"pattern_menu"),0);
	pthread_mutex_unlock(&camera->uimutex);
      }
      //else
	//fprintf(stderr,"Filter is zero!\n");
    }
  }

  dc1394_video_get_data_depth(camera->camera_info, &camera->bpp);

  int cond= ((bpp16_ok) && 
	     (camera->stereo==-1) && 
	     (camera->bayer==-1) &&
	     (camera->bpp==16));

  pthread_mutex_lock(&camera->uimutex);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window, "mono16_bpp"),camera->bpp);
  gtk_widget_set_sensitive(lookup_widget(main_window,"mono16_bpp"),cond);
  gtk_widget_set_sensitive(lookup_widget(main_window,"label114"), cond);
  pthread_mutex_unlock(&camera->uimutex);
  
}


void
UpdateServiceFrame(void)
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_iso")),
			       GetService(camera,SERVICE_ISO)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_display")),
			       GetService(camera,SERVICE_DISPLAY)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_save")),
			       GetService(camera,SERVICE_SAVE)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_ftp")),
			       GetService(camera,SERVICE_FTP)!=NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (lookup_widget(main_window,"service_v4l")),
			       GetService(camera,SERVICE_V4L)!=NULL);

}


void
UpdateFormat7InfoFrame(void)
{

  char *temp;
  dc1394format7mode_t *mode;
  unsigned int packet_size;
  int bytesize, grandtotal;
  
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  if (camera->format7_info.edit_mode!=-1) {

    mode = &camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN];

    dc1394_get_color_coding_bit_size(mode->color_coding, &packet_size);

    bytesize=(mode->size_x*mode->size_y*packet_size)/8;
    /*
      // this appears to be meaningless as some cameras take padding into account
      if (bytesize!=mode->total_bytes) {
      fprintf(stderr,"bytesize: %d, total_bytes: %d\n",bytesize, (int)mode->total_bytes);
      Warning("The camera has a strange TOTAL_BYTES value.");
      }
    */
    //fprintf(stderr,"total bytes: %d\n",mode->total_bytes);
    if (mode->packet_size==0) {
      //Error("PACKET_SIZE is zero! This should not happen.");

      // do something about it:
      // - first wait a bit and try to get the PACKET_SIZE again
      // (what follows only if ISO is not running)
      // - then, if it's not ok, do a set_roi with all current values, wait and get PACKET_SIZE again
      // - at last, do a set_roi with all max values, wait and get PACKET_SIZE again
      // if all else fails, set the temp string to "Invalid PACKET_SIZE"
      
      // note: this obviously happen mostly (if not only) at camera boot time. So maybe the F7
      // mode probing function could carry the function of setting a valid F7 by default.
      // definitely better... (and in libdc1394 so everyone enjoys the modification)
      grandtotal=0;
      sprintf(temp,"Invalid PACKET_SIZE");
    }
    else {
      // if there is packet padding, take it into account
      if (mode->total_bytes%mode->packet_size!=0) {
	grandtotal=(mode->total_bytes/mode->packet_size+1)*mode->packet_size;
      }
      else {
	grandtotal=mode->total_bytes;
      }
    }

    if (mode->packet_size>0)
      sprintf(temp," %d", bytesize);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"format7_imagebytes"), 
			 ctxt.format7_imagebytes_ctxt, ctxt.format7_imagebytes_id);
    ctxt.format7_imagebytes_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"format7_imagebytes"), 
						  ctxt.format7_imagebytes_ctxt,temp);
    
    if (mode->packet_size>0)
      sprintf(temp," %d", mode->size_x*mode->size_y);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"format7_imagepixels"), 
			 ctxt.format7_imagepixels_ctxt, ctxt.format7_imagepixels_id);
    ctxt.format7_imagepixels_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"format7_imagepixels"), 
						   ctxt.format7_imagepixels_ctxt,temp);
    
    if (mode->packet_size>0)
      sprintf(temp," %d", grandtotal);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"format7_totalbytes"), 
			 ctxt.format7_totalbytes_ctxt, ctxt.format7_totalbytes_id);
    ctxt.format7_totalbytes_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"format7_totalbytes"), 
						  ctxt.format7_totalbytes_ctxt,temp);
    
    if (mode->packet_size>0)
      sprintf(temp," %d", grandtotal - bytesize);
    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"format7_padding"), 
			 ctxt.format7_padding_ctxt, ctxt.format7_padding_id);
    ctxt.format7_padding_id=gtk_statusbar_push((GtkStatusbar*)lookup_widget(main_window,"format7_padding"), 
					       ctxt.format7_padding_ctxt,temp);
    
  }
  free(temp);
}

void
UpdateBandwidthFrame(void)
{

#ifdef HAVE_GET_PORT

  camera_t* cam;
  unsigned int bandwidth;
  float *ports;
  float ratio;
  char* temp;
  dc1394switch_t iso;
  int nports, i, truebps, theobps;
  chain_t* iso_service;
  GtkProgressBar *bar;
  dc1394video_mode_t video_mode;
  unsigned int port;
  
  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  // get the number of ports
  nports=port_num;
  
  ports=(float*)malloc(nports*sizeof(float));

  for (i=0;i<nports;i++)
    ports[i]=0;

  cam=cameras;
  while(cam!=NULL) {
    if (dc1394_video_get_transmission(cam->camera_info, &iso)!=DC1394_SUCCESS) {
      Error("Could not get ISO status");
    }

    if (iso==DC1394_ON) {
      if (dc1394_video_get_bandwidth_usage(cam->camera_info, &bandwidth)!=DC1394_SUCCESS) {
	Error("Could not get a camera bandwidth usage. Bus usage might be inaccurate.");
      }
    }
    else {
      bandwidth=0;
    }
    //fprintf(stderr,"%d\n",bandwidth);
    iso_service=GetService(cam,SERVICE_ISO);
    dc1394_video_get_mode(cam->camera_info,&video_mode);

    // if we are using format7 and there is a running ISO service, we can get a better estimate:
    if (((video_mode >= DC1394_VIDEO_MODE_FORMAT7_MIN) && (video_mode <= DC1394_VIDEO_MODE_FORMAT7_MAX))
	&&(iso_service!=NULL)){
      //fprintf(stderr,"better estimate can be found\n");
      // use the fractions of packets needed:
      theobps=8000*cam->format7_info.modeset.mode[cam->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].packet_size;
      truebps=iso_service->fps*cam->format7_info.modeset.mode[cam->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].total_bytes;
      ratio=(float)truebps/(float)theobps;
      //fprintf(stderr,"truebps: %d, theobps: %d, ratio: %.2f\n",truebps, theobps, ratio);
      // apply only if the ratio is less than 0.95 and greater than 0
      if ((ratio<.95)&&(ratio>0))
	bandwidth=(int)((float)bandwidth*ratio);
    }

    // sum the values of the bandwidths
    if (dc1394_camera_get_linux_port(cam->camera_info, &port)==DC1394_SUCCESS && port<port_num)
	ports[port]+=bandwidth;

    cam=cam->next;
  }

  for (i=0;i<nports;i++) {
    sprintf(temp,"bandwidth_bar%d",i);
    ports[i]=ports[i]/4915.0;
    if (ports[i]>1.0)
      ports[i]=1;
    //fprintf(stderr,"Cam bandwidth usage: %.1f%%\n",100*ports[i]);
    //fprintf(stderr,"test 2\n");
    bar=GTK_PROGRESS_BAR(lookup_widget(main_window,temp));
    gtk_progress_bar_update(GTK_PROGRESS_BAR(bar),ports[i] );
    //gtk_progress_bar_update(GTK_PROGRESS_BAR(bar),.33 );
  }
  free(ports);
  free(temp);
  //fprintf(stderr,"test 3\n");

#endif

  UpdateServiceTree();
  //fprintf(stderr,"ee\n");

}

void
UpdateServiceTree(void)
{
  char **temp;
  int i, j, n, s, service_id;
  camera_t* cam;
  chain_t* service;
  char tmp_string[20];
  GtkTreeModel *model;
  GtkTreeStore *store;
  GtkTreeView *view;
  GtkTreeIter cam_leaf, service_leaf;

  view  = (GtkTreeView*)lookup_widget(main_window,"service_tree");
  model = gtk_tree_view_get_model(view);
  store = GTK_TREE_STORE(model);

  temp=(char**)malloc(4*sizeof(char*));
  for (i=0;i<4;i++) {
    temp[i]=(char*)malloc(STRING_SIZE*sizeof(char));
  }

  gtk_tree_model_get_iter_first(model, &cam_leaf);

  //eprint("----------\n");
  cam=cameras;
  while(cam!=NULL) {
    n = gtk_tree_model_iter_n_children  (model, &cam_leaf);
    //fprintf(stderr,"Initial number of services in list: %d\n",n);

    if (n>0) {
      gtk_tree_model_iter_children(model, &service_leaf, &cam_leaf);
      // check that each existing service is alive
      for (i=0;i<n;i++) {
	gtk_tree_model_get (model, &service_leaf, 4, &service_id, -1);
	if (GetService(cam,service_id)==NULL) {
	  // remove this service row
	  //fprintf(stderr,"Service dead, removing\n");
	  gtk_tree_store_remove(store, &service_leaf);
	  n-=1;
	}
	// go to next service
	if (service_leaf.user_data!=NULL && n>=1)
	  gtk_tree_model_iter_next(model, &service_leaf);
      }
    }
    //n = gtk_tree_model_iter_n_children  (model, &cam_leaf);
    //fprintf(stderr,"Number of services in list after cleanup: %d\n",n);

    for (s=SERVICE_ISO;s<=SERVICE_FTP;s++) {
      service=GetService(cam,s);
      if (service!=NULL) {

	// update volatile data in all cases
	if (service->fps>0) {
	  sprintf(tmp_string," %.3f",service->fps);
	  sprintf(temp[1],"%.5f", service->fps);
	}
	else {
	  sprintf(tmp_string," %.3f",fabs(0.0));
	  sprintf(temp[1],"%.5f", fabs(0.0));
	}

	sprintf(temp[2],"%llu", service->processed_frames);
	sprintf(temp[3],"%d", service->drop_warning);

	// update main window FPS counters
	switch(s) {
	case SERVICE_ISO:
	  sprintf(temp[0],"Receive");
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_receive"),
				 ctxt.fps_receive_ctxt, ctxt.fps_receive_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_receive"),
						   ctxt.fps_receive_ctxt, tmp_string);
	  }
	  break;
	case SERVICE_DISPLAY:
	  sprintf(temp[0],"Display");
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_display"),
				 ctxt.fps_display_ctxt, ctxt.fps_display_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_display"),
						   ctxt.fps_display_ctxt, tmp_string);
	    }
	  break;
	case SERVICE_SAVE:
	  sprintf(temp[0],"Save");
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_save"),
				 ctxt.fps_save_ctxt, ctxt.fps_save_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_save"),
						   ctxt.fps_save_ctxt, tmp_string);
	  }
	  break;
	case SERVICE_V4L:
	  sprintf(temp[0],"V4L");
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_v4l"),
				 ctxt.fps_v4l_ctxt, ctxt.fps_v4l_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_v4l"),
						   ctxt.fps_v4l_ctxt, tmp_string);
	  }
	  break;
	case SERVICE_FTP:
	  sprintf(temp[0],"FTP");
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_ftp"),
				 ctxt.fps_ftp_ctxt, ctxt.fps_ftp_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_ftp"),
						   ctxt.fps_ftp_ctxt, tmp_string);
	  }
	  break;
	default:
	  sprintf(temp[0],"!! Unknown service ID !!");
	  break;
	}

	// find service in the list
	n = gtk_tree_model_iter_n_children  (model, &cam_leaf);
	if (n>0) {
	  gtk_tree_model_iter_children(model, &service_leaf, &cam_leaf);
	  for (j=0;j<n;j++) {
	    gtk_tree_model_get(model, &service_leaf, 4, &service_id, -1);
	    //fprintf(stderr,"service_id of node %d: %d\n",j,service_id);
	    if (service_id==s) {
	      //fprintf(stderr,"  service found\n");
	      break;
	    }
	    // go to next service
	    if (service_leaf.user_data!=NULL)
	      gtk_tree_model_iter_next(model, &service_leaf);
	  }
	}
	else {
	  service_id=-1; //force add
	}
	// if not there, create a new row and label it
	if (service_id!=s) {
	  //fprintf(stderr,"Add service id %d\n",s);
	  gtk_tree_store_append(store, &service_leaf, &cam_leaf);
	  gtk_tree_store_set(store, &service_leaf, 0, temp[0], 4, s, -1);
	}
	gtk_tree_store_set(store, &service_leaf, 1, temp[1], 2, temp[2], 3, temp[3], -1);

      }
      else {
	// we need to clear the FPS counters in the main window
      
	switch(s) {
	case SERVICE_ISO:
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_receive"),
				 ctxt.fps_receive_ctxt, ctxt.fps_receive_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_receive"),
						   ctxt.fps_receive_ctxt, "");
	  }
	  break;
	case SERVICE_DISPLAY:
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_display"),
				 ctxt.fps_display_ctxt, ctxt.fps_display_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_display"),
						   ctxt.fps_display_ctxt, "");
	    }
	  break;
	case SERVICE_SAVE:
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_save"),
				 ctxt.fps_save_ctxt, ctxt.fps_save_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_save"),
						   ctxt.fps_save_ctxt, "");
	  }
	  break;
	case SERVICE_V4L:
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_v4l"),
				 ctxt.fps_v4l_ctxt, ctxt.fps_v4l_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_v4l"),
						   ctxt.fps_v4l_ctxt, "");
	  }
	  break;
	case SERVICE_FTP:
	  if (cam==camera) {
	    gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"fps_ftp"),
				 ctxt.fps_ftp_ctxt, ctxt.fps_ftp_id);
	    ctxt.fps_receive_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"fps_ftp"),
						   ctxt.fps_ftp_ctxt,"");
	  }
	  break;
	default:
	  sprintf(temp[0],"!! Unknown service ID !!");
	  break;
	}
      }
    }
    //n = gtk_tree_model_iter_n_children  (model, &cam_leaf);
    //fprintf(stderr,"Number of services in list after adding: %d\n",n);

    cam=cam->next;
    gtk_tree_model_iter_next(model, &cam_leaf);
  }

  for (i=0;i<4;i++) {
    free(temp[i]);
  }
  free(temp);

}

void
UpdatePrefsDisplayOverlayFrame(void)
{
  //fprintf(stderr,"updated: %d %d\n",camera->prefs.overlay_pattern,camera->prefs.overlay_type);
  gtk_widget_set_sensitive(lookup_widget(main_window,"overlay_type_menu"), camera->prefs.overlay_pattern!=OVERLAY_PATTERN_OFF);
  gtk_widget_set_sensitive(lookup_widget(main_window,"overlay_color_picker"), (camera->prefs.overlay_pattern!=OVERLAY_PATTERN_OFF) && (camera->prefs.overlay_type==OVERLAY_TYPE_REPLACE));
}

void
UpdateSaveFilenameFrame(void)
{
  char *filename_out;
  
  filename_out=(char*)malloc(STRING_SIZE*sizeof(char));

  // set current menu item
  pthread_mutex_lock(&camera->uimutex);
  if (camera->prefs.save_format<SAVE_FORMAT_FIRST_VIDEO)
    gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "save_format_menu")),camera->prefs.save_format+1);
  else
    gtk_option_menu_set_history(GTK_OPTION_MENU(lookup_widget(main_window, "save_format_menu")),camera->prefs.save_format+2);
  pthread_mutex_unlock(&camera->uimutex);

  // update the filename string
  switch (camera->prefs.save_format) {
#ifdef HAVE_FFMPEG
  case SAVE_FORMAT_JPEG:
#endif
  case SAVE_FORMAT_PPM:
  case SAVE_FORMAT_RAW:
    // first handle the case of save-to-dir
    if (camera->prefs.save_to_dir==0) {
      switch (camera->prefs.save_append) {
      case SAVE_APPEND_NONE:
	sprintf(filename_out, "%s.%s", camera->prefs.save_filename, camera->prefs.save_filename_ext);
	break;
      case SAVE_APPEND_DATE_TIME:
	sprintf(filename_out, "%s-date-time-ms.%s", camera->prefs.save_filename, camera->prefs.save_filename_ext);
	break;
      case SAVE_APPEND_NUMBER:
	sprintf(filename_out,"%s-frame_number.%s", camera->prefs.save_filename, camera->prefs.save_filename_ext);
	break;
      }
    }
    else { // we save to a directory...
      switch (camera->prefs.save_append) {
      case SAVE_APPEND_NONE:
        sprintf(filename_out, "time or number should have been selected");
	break;
      case SAVE_APPEND_DATE_TIME:
	sprintf(filename_out, "%s-date-time-ms/date-time-ms.%s", camera->prefs.save_filename, camera->prefs.save_filename_ext);
	break;
      case SAVE_APPEND_NUMBER:
	sprintf(filename_out,"%s-date-time-ms/frame_number.%s", camera->prefs.save_filename, camera->prefs.save_filename_ext);
	break;
      }
      // 3. done!
    }
    break;
  case SAVE_FORMAT_RAW_VIDEO:
  case SAVE_FORMAT_PVN:
#ifdef HAVE_FFMPEG
  case SAVE_FORMAT_MPEG:
#endif
    switch (camera->prefs.save_append) {
    case SAVE_APPEND_NONE:
      sprintf(filename_out, "%s.%s", camera->prefs.save_filename, camera->prefs.save_filename_ext);
      break;
    case SAVE_APPEND_DATE_TIME:
      sprintf(filename_out, "%s-date-time-ms.%s", camera->prefs.save_filename, camera->prefs.save_filename_ext);
      break;
    case SAVE_APPEND_NUMBER:
      sprintf(filename_out,"%s-frame_number.%s", camera->prefs.save_filename, camera->prefs.save_filename_ext);
      break;
    }
    break;
  }

  gtk_statusbar_remove((GtkStatusbar*)lookup_widget(main_window,"save_filename_status"), ctxt.save_filename_ctxt, ctxt.save_filename_id);
  ctxt.save_filename_id=gtk_statusbar_push((GtkStatusbar*) lookup_widget(main_window,"save_filename_status"), ctxt.save_filename_ctxt, filename_out);

  free(filename_out);
  
}
