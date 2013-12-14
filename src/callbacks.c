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

#define EEPROM_CNFG       0xF00U
#define TEST_CNFG         0xF04U
#define CCR_BASE          0xFFFFF0F00000ULL

gboolean
on_main_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_exit(0);
  return FALSE;
}


void
on_file_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  // function intentionnaly left almost blank: intermediate menu
}


void
on_exit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_exit(0);
}


void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  about_window = create_about_window ();
  gtk_widget_show (about_window);
}


void
on_fps_activate                    (GtkMenuItem     *menuitem,
				    gpointer         user_data)
{
  int state;
  
  IsoFlowCheck(&state);
    
  if(dc1394_video_set_framerate(camera->camera_info, (int)(unsigned long)user_data)!=DC1394_SUCCESS)
    Error("Could not set framerate");

  IsoFlowResume(&state);
  UpdateFeatureWindow(); // because several controls may change, especially exposure, gamma, shutter,... since the framerate changes.
}


void
on_power_on_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if(dc1394_camera_set_power(camera->camera_info, DC1394_ON)!=DC1394_SUCCESS)
    Error("Could not set camera 'on'");
}


void
on_power_off_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  if(dc1394_camera_set_power(camera->camera_info, DC1394_OFF)!=DC1394_SUCCESS)
    Error("Could not set camera 'off'");
}


void
on_power_reset_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  if (dc1394_camera_reset(camera->camera_info)!=DC1394_SUCCESS)
    Error("Could not initilize camera");
}

void
on_trigger_polarity_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if  ( (camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].available == DC1394_TRUE) &&
	(camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].polarity_capable == DC1394_TRUE)) {
    if (togglebutton->active) {
      if (dc1394_external_trigger_set_polarity(camera->camera_info,DC1394_TRIGGER_ACTIVE_HIGH)!=DC1394_SUCCESS)
	Error("Cannot set trigger polarity");
      else
	camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_polarity=DC1394_TRIGGER_ACTIVE_HIGH;
    }
    else {
      if (dc1394_external_trigger_set_polarity(camera->camera_info,DC1394_TRIGGER_ACTIVE_LOW)!=DC1394_SUCCESS)
	Error("Cannot set trigger polarity");
      else
	camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_polarity=DC1394_TRIGGER_ACTIVE_LOW;
    }
  }
}


void
on_trigger_mode_activate              (GtkMenuItem     *menuitem,
				       gpointer         user_data)
{
  if (dc1394_external_trigger_set_mode(camera->camera_info, (int)user_data)!=DC1394_SUCCESS)
    Error("Could not set trigger mode");
  else
    camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_mode=(int)user_data;
  UpdateTriggerFrame();
}

void
on_trigger_source_activate              (GtkMenuItem     *menuitem,
				       gpointer         user_data)
{
  if (dc1394_external_trigger_set_source(camera->camera_info, (int)user_data)!=DC1394_SUCCESS)
    Error("Could not set trigger source");
  else
    camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_source=(int)user_data;
  UpdateTriggerFrame();
}

void
on_trigger_external_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if  (camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].available == DC1394_TRUE) {
    if (dc1394_feature_set_power(camera->camera_info, DC1394_FEATURE_TRIGGER, togglebutton->active)!=DC1394_SUCCESS)
      Error("Could not set external trigger source");
    else
      camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].is_on=togglebutton->active;
    UpdateTriggerFrame();
  }
}


void
on_memory_channel_activate              (GtkMenuItem     *menuitem,
					 gpointer         user_data)
{
  camera->memory_channel=(int)(unsigned long)user_data; // user data is an int.
  UpdateMemoryFrame();
}



void
on_load_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  if (dc1394_memory_load(camera->camera_info, camera->memory_channel)!=DC1394_SUCCESS)
    Error("Cannot load memory channel");
  UpdateAllWindows();

}

void
on_save_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{ 
  unsigned long int timeout_bin=0;
  unsigned long int step;
  dc1394bool_t value=TRUE;
  step=(unsigned long int)(1000000.0/preferences.auto_update_frequency);

  if (dc1394_memory_save(camera->camera_info, camera->memory_channel)!=DC1394_SUCCESS)
    Error("Could not save setup to memory channel");
  else {
    while ((value==DC1394_TRUE) &&(timeout_bin<(unsigned long int)(preferences.op_timeout*1000000.0)) ) {
      usleep(step);
      if (dc1394_memory_busy(camera->camera_info, &value)!=DC1394_SUCCESS)
	Error("Could not query if memory save is in operation");
      timeout_bin+=step;
    }
    if (timeout_bin>=(unsigned long int)(preferences.op_timeout*1000000.0))
      Warning("Save operation function timed-out!"); 
  }
}

void
on_iso_start_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
  dc1394switch_t status;

  if (dc1394_video_set_transmission(camera->camera_info,DC1394_ON)!=DC1394_SUCCESS)
    Error("Could not start ISO transmission");
  else {
    usleep(DELAY);
    if (dc1394_video_get_transmission(camera->camera_info, &status)!=DC1394_SUCCESS)
      Error("Could get ISO status");
    else {
      if (status==DC1394_FALSE) {
	Error("ISO transmission refuses to start");
      }
      UpdateIsoFrame();
    }
  }
  UpdateTransferStatusFrame();
}


void
on_iso_stop_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  dc1394switch_t status;

  if (dc1394_video_set_transmission(camera->camera_info,DC1394_OFF)!=DC1394_SUCCESS)
    Error("Could not stop ISO transmission");
  else {
    usleep(DELAY);
    if (dc1394_video_get_transmission(camera->camera_info, &status)!=DC1394_SUCCESS)
      Error("Could get ISO status");
    else {
      if (status==DC1394_TRUE) {
	Error("ISO transmission refuses to stop");
      }
      UpdateIsoFrame();
    }
  }
  UpdateTransferStatusFrame();
}


void
on_iso_restart_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  on_iso_stop_clicked(GTK_BUTTON(lookup_widget(main_window,"iso_stop")),NULL);
  on_iso_start_clicked(GTK_BUTTON(lookup_widget(main_window,"iso_start")),NULL);
  UpdateTransferStatusFrame();
}

void
on_camera_select_activate              (GtkMenuItem     *menuitem,
					gpointer         user_data)
{
  camera_t* camera_ptr;
  
  //fprintf(stderr,"old camera: %s\n",camera->prefs.name);
  // close current display (we don't want display to be used by 2 threads at the same time 'cause SDL forbids it)
  DisplayStopThread(camera);
  //fprintf(stderr,"camera: %s\n",camera->prefs.name);

  camera_ptr=(camera_t*)user_data;
  //fprintf(stderr,"new camera: %s\n",camera_ptr->prefs.name);

  // set current camera pointers:
  SetCurrentCamera(camera_ptr->camera_info->guid);
  //fprintf(stderr,"new camera: %s\n",camera_ptr->prefs.name);

#ifdef HAVE_SDLLIB
  watchthread_info.draw=0;
  watchthread_info.mouse_down=0;
  watchthread_info.crop=0;
#endif

  if (camera->want_to_display>0)
    DisplayStartThread(camera);

  //fprintf(stderr,"new camera: %s\n",camera->prefs.name);

  // redraw all:
  //eprint("Build All Windows\n");
  BuildAllWindows();
  //eprint("Update All Windows\n");

  // WARNING: blocking the 'changed' signal from the camera name entry is necessary to avoid an endless loop of callbacks:
  // menu item changed -> update text entry -> text entry changed -> update menu item
  // this did not appear before, it probably comes form the gnome 1.2 -> gnome 2.0 migration.

  g_signal_handlers_block_by_func(GTK_OBJECT(lookup_widget(main_window,"camera_name_text")), G_CALLBACK (on_camera_name_text_changed), NULL);
  UpdateAllWindows();
  g_signal_handlers_unblock_by_func(GTK_OBJECT(lookup_widget(main_window,"camera_name_text")), G_CALLBACK (on_camera_name_text_changed), NULL);

}

void
on_edit_format7_mode_activate             (GtkMenuItem     *menuitem,
					   gpointer         user_data)
{
  camera->format7_info.edit_mode=(int)(unsigned long)user_data;

  if (dc1394_format7_get_mode_info(camera->camera_info, camera->format7_info.edit_mode, 
				   &camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN])!=DC1394_SUCCESS)
    Error("Could not get format7 mode information");

  UpdateFormat7Window();
}

void
on_edit_format7_color_activate             (GtkMenuItem     *menuitem,
					    gpointer         user_data)
{
  int state;

  dc1394video_mode_t video_mode;
  dc1394_video_get_mode(camera->camera_info, &video_mode);

  // if the mode is the 'live' mode:
  if (camera->format7_info.edit_mode==video_mode)
    IsoFlowCheck(&state);

  if (dc1394_format7_set_color_coding(camera->camera_info, camera->format7_info.edit_mode, (int)(unsigned long)user_data)!=DC1394_SUCCESS)
    Error("Could not change Format7 color coding");
  else {
    if (dc1394_format7_get_mode_info(camera->camera_info, camera->format7_info.edit_mode, 
				     &camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN])!=DC1394_SUCCESS)
      Error("Could not get format7 mode information");
  }

  UpdateOptionFrame();
  UpdateFormat7Window();

  // if the mode is the 'live' mode:
  if (camera->format7_info.edit_mode==video_mode) {
    IsoFlowResume(&state);
  }

}


void
on_scale_value_changed             ( GtkAdjustment    *adj,
				     gpointer         user_data)
{
  switch((int)(unsigned long)user_data) {
  case DC1394_FEATURE_TEMPERATURE:
    if (dc1394_feature_temperature_set_value(camera->camera_info,adj->value)!=DC1394_SUCCESS)
      Error("Could not set temperature");
    else
      camera->feature_set.feature[DC1394_FEATURE_TEMPERATURE-DC1394_FEATURE_MIN].target_value=adj->value;
    break;
  case DC1394_FEATURE_WHITE_BALANCE+BU: // why oh why is there a *4?
    if (dc1394_feature_whitebalance_set_value(camera->camera_info,adj->value, camera->feature_set.feature[DC1394_FEATURE_WHITE_BALANCE-DC1394_FEATURE_MIN].RV_value)!=DC1394_SUCCESS)
      Error("Could not set B/U white balance");
    else {
      //fprintf(stderr,"WB changed to %.0f\n",adj->value);
      camera->feature_set.feature[DC1394_FEATURE_WHITE_BALANCE-DC1394_FEATURE_MIN].BU_value=adj->value;
      if (camera->feature_set.feature[DC1394_FEATURE_WHITE_BALANCE-DC1394_FEATURE_MIN].absolute_capable!=0) {
	GetAbsValue(DC1394_FEATURE_WHITE_BALANCE);
      }
    }
    break;
  case DC1394_FEATURE_WHITE_BALANCE+RV: // why oh why is there a *4?
    if (dc1394_feature_whitebalance_set_value(camera->camera_info, camera->feature_set.feature[DC1394_FEATURE_WHITE_BALANCE-DC1394_FEATURE_MIN].BU_value, adj->value)!=DC1394_SUCCESS)
      Error("Could not set R/V white balance");
    else {
      camera->feature_set.feature[DC1394_FEATURE_WHITE_BALANCE-DC1394_FEATURE_MIN].RV_value=adj->value;
      if (camera->feature_set.feature[DC1394_FEATURE_WHITE_BALANCE-DC1394_FEATURE_MIN].absolute_capable!=0) {
	GetAbsValue(DC1394_FEATURE_WHITE_BALANCE);
      }
    }
    break;
  case DC1394_FEATURE_WHITE_SHADING+SHADINGR:
    if (dc1394_feature_whiteshading_set_value(camera->camera_info,  adj->value, camera->feature_set.feature[DC1394_FEATURE_WHITE_SHADING-DC1394_FEATURE_MIN].G_value,
					      camera->feature_set.feature[DC1394_FEATURE_WHITE_SHADING-DC1394_FEATURE_MIN].B_value)!=DC1394_SUCCESS)
      Error("Could not set white shading / blue channel");
    else {
      camera->feature_set.feature[DC1394_FEATURE_WHITE_SHADING-DC1394_FEATURE_MIN].R_value=adj->value;
      // don't know about abs values for white shading:
      /*
      if (camera->feature_set.feature[FEATURE_WHITE_SHADING-FEATURE_MIN].absolute_capable!=0) {
	GetAbsValue(FEATURE_WHITE_SHADING);
      }
      */
    }
    break;
  case DC1394_FEATURE_WHITE_SHADING+SHADINGG:
    if (dc1394_feature_whiteshading_set_value(camera->camera_info, camera->feature_set.feature[DC1394_FEATURE_WHITE_SHADING-DC1394_FEATURE_MIN].R_value,
				 adj->value, camera->feature_set.feature[DC1394_FEATURE_WHITE_SHADING-DC1394_FEATURE_MIN].B_value)!=DC1394_SUCCESS)
      Error("Could not set white shading / blue channel");
    else {
      camera->feature_set.feature[DC1394_FEATURE_WHITE_SHADING-DC1394_FEATURE_MIN].G_value=adj->value;
      // don't know about abs values for white shading:
      /*
      if (camera->feature_set.feature[FEATURE_WHITE_SHADING-FEATURE_MIN].absolute_capable!=0) {
	GetAbsValue(FEATURE_WHITE_SHADING);
      }
      */
    }
    break;
  case DC1394_FEATURE_WHITE_SHADING+SHADINGB:
    if (dc1394_feature_whiteshading_set_value(camera->camera_info, camera->feature_set.feature[DC1394_FEATURE_WHITE_SHADING-DC1394_FEATURE_MIN].R_value,
				 camera->feature_set.feature[DC1394_FEATURE_WHITE_SHADING-DC1394_FEATURE_MIN].G_value, adj->value)!=DC1394_SUCCESS)
      Error("Could not set white shading / blue channel");
    else {
      camera->feature_set.feature[DC1394_FEATURE_WHITE_SHADING-DC1394_FEATURE_MIN].B_value=adj->value;
      // don't know about abs values for white shading:
      /*
      if (camera->feature_set.feature[FEATURE_WHITE_SHADING-FEATURE_MIN].absolute_capable!=0) {
	GetAbsValue(FEATURE_WHITE_SHADING);
      }
      */
    }
    break;
    
  default: // includes trigger_count
    if (dc1394_feature_set_value(camera->camera_info,(int)(unsigned long)user_data,adj->value)!=DC1394_SUCCESS)
      Error("Could not set feature");
    else {
      camera->feature_set.feature[(int)(unsigned long)user_data-DC1394_FEATURE_MIN].value=adj->value;
      if ((int)(unsigned long)user_data!=DC1394_FEATURE_TRIGGER) {
	if (camera->feature_set.feature[(int)(unsigned long)user_data-DC1394_FEATURE_MIN].absolute_capable!=0) {
	  GetAbsValue((int)(unsigned long)user_data);
	}
      }
    }
    // optical filter sometimes changes white balance (sony cameras) so we update the WB.
    if ((int)(unsigned long)user_data==DC1394_FEATURE_OPTICAL_FILTER) {
      UpdateRange(DC1394_FEATURE_WHITE_BALANCE);
    }
    break;
  }
}

void
on_format7_value_changed             ( GtkAdjustment    *adj,
				       gpointer         user_data)
{
  int sx, sy, px, py;
  dc1394format7mode_t* info;

  if (camera->format7_info.edit_mode>=0) { // check if F7 is supported
    info=&camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN];
    sx=info->size_x;
    sy=info->size_y;
    px=info->pos_x;
    py=info->pos_y;

    if ((int)user_data==FORMAT7_PACKET) {
	unsigned int packet_size;
	int state;
	adj->value=NearestValue(adj->value,camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].unit_packet_size,
				camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].unit_packet_size,
				camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].max_packet_size);
	IsoFlowCheck(&state);
	
	if (dc1394_format7_set_packet_size(camera->camera_info, 
				     camera->format7_info.edit_mode, adj->value)!=DC1394_SUCCESS)
	    Error("Could not change Format7 bytes per packet");
	
	if (dc1394_format7_get_packet_size(camera->camera_info,
					   camera->format7_info.edit_mode,&packet_size)!=DC1394_SUCCESS) 
	    Error("Could not query Format7 bytes per packet");
	else {
	    camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN].packet_size=packet_size;
	    if (packet_size==0)
		fprintf(stderr,"Packet size is zero in %s at line %d\n",__FUNCTION__,__LINE__);
	    
	    // tell the range to change its setting
	    adj->value=packet_size;
	}
	IsoFlowResume(&state);

	UpdateFormat7Ranges();
	
    }
    else {
	switch((int)user_data) {
	case FORMAT7_SIZE_X:
	    sx=NearestValue(adj->value,info->unit_size_x, info->unit_size_x, info->max_size_x - px);
	    adj->value=sx;
	    break;
	case FORMAT7_SIZE_Y:
	    sy=NearestValue(adj->value,info->unit_size_y, info->unit_size_y, info->max_size_y - py);
	    adj->value=sy;
	    break;
	case FORMAT7_POS_X:
	    px=NearestValue(adj->value,info->unit_pos_x, 0, info->max_size_x - info->unit_pos_x);
	    adj->value=px;
	    break;
	case FORMAT7_POS_Y:
	    py=NearestValue(adj->value,info->unit_pos_y, 0, info->max_size_y - info->unit_pos_y);
	    adj->value=py;
	    break;
	}
	SetFormat7Crop(sx,sy,px,py, camera->format7_info.edit_mode); // this includes re-building the ranges
    }

    UpdateFeatureWindow(); // because several controls may change, especially exposure, gamma, shutter,... since the framerate changes.
  }

}


void
on_preferences_window_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show(preferences_window);
}


void
on_service_iso_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      if (IsoStartThread(camera)==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    }
    else
      IsoStopThread(camera);
  }
  UpdatePrefsReceiveFrame();
  UpdateIsoFrame();
}


void
on_service_display_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

  if (!silent_ui_update) {
    if (togglebutton->active) {
      pthread_mutex_lock(&camera->uimutex);
      camera->want_to_display=1;
      pthread_mutex_unlock(&camera->uimutex);
      if (DisplayStartThread(camera)==-1) {
	gtk_toggle_button_set_active(togglebutton,0);
	camera->want_to_display=0;
      }
      else
          AutoIsoReceive(DC1394_ON);
    } 
    else {
      DisplayStopThread(camera);
      pthread_mutex_lock(&camera->uimutex);
      camera->want_to_display=0;
      pthread_mutex_unlock(&camera->uimutex);
      AutoIsoReceive(DC1394_OFF);
    } 
  }
  UpdatePrefsDisplayFrame();
}


void
on_service_save_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      if (SaveStartThread(camera)==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    }
    else {
      //fprintf(stderr,"stopping service...\n");
      SaveStopThread(camera);
      //fprintf(stderr,"Stopped\n");
    }
  }
  //fprintf(stderr,"updating...\n");
  UpdatePrefsSaveFrame();
  //fprintf(stderr,"done\n");
}


void
on_service_ftp_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      if (FtpStartThread(camera)==-1)
	gtk_toggle_button_set_active(togglebutton,0);
    }
    else
      FtpStopThread(camera);
  }
  UpdatePrefsFtpFrame();
}


void
on_service_v4l_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (!silent_ui_update) {
    if (togglebutton->active) {
      if (V4lStartThread(camera)==-1) {
	gtk_toggle_button_set_active(togglebutton,0);
	gtk_widget_show(create_v4l_failure_window());
      }
    }
    else
      V4lStopThread(camera);
  }
  UpdatePrefsV4lFrame();
}

void
on_offset_menu_activate             (GtkMenuItem     *menuitem,
				    gpointer         user_data)
{
  int offset;

  offset=(int)user_data;
  //fprintf(stderr,"offset: %d\n",offset);
  camera->register_offset=offset;
}

void
on_isospeed_menu_activate             (GtkMenuItem     *menuitem,
			   	       gpointer         user_data)
{
  dc1394speed_t speed;

  speed=(dc1394speed_t)user_data;
  //fprintf(stderr,"Asking for speed %d\n",speed);
  dc1394_video_set_iso_speed(camera->camera_info, speed);

  BuildAllWindows();
  UpdateAllWindows();
  
}


void
on_register_write_button_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
  quadlet_t value;
  octlet_t offset;
  char *string;
  dc1394error_t err=DC1394_SUCCESS;
  char string2[16];

  string=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window, "register_address_entry")));
  sscanf(string,"%llx",&offset);
  string=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window, "register_data_entry")));
  sscanf(string,"%x",&value);

  switch(camera->register_offset) {
  case REGISTER_OFFSET_BASE:
    err=dc1394_set_register(camera->camera_info, offset, value);
    break;
  case REGISTER_OFFSET_CRB:
    err=dc1394_set_control_register(camera->camera_info, offset, value);
    break;
    /*
  case REGISTER_OFFSET_UD:
    err=dc1394_set_register(camera->camera_info, offset+camera->camera_info->unit_directory, value);
    break;
    */
  case REGISTER_OFFSET_UDD:
    err=dc1394_set_register(camera->camera_info, offset+camera->camera_info->unit_dependent_directory, value);
    break;
  case DC1394_VIDEO_MODE_FORMAT7_0:
  case DC1394_VIDEO_MODE_FORMAT7_1:
  case DC1394_VIDEO_MODE_FORMAT7_2:
  case DC1394_VIDEO_MODE_FORMAT7_3:
  case DC1394_VIDEO_MODE_FORMAT7_4:
  case DC1394_VIDEO_MODE_FORMAT7_5:
  case DC1394_VIDEO_MODE_FORMAT7_6:
  case DC1394_VIDEO_MODE_FORMAT7_7:
    err=dc1394_set_format7_register(camera->camera_info, camera->register_offset, offset, value);
    break;
  case REGISTER_OFFSET_PIO:
    err=dc1394_set_PIO_register(camera->camera_info, offset, value);
    break;
  case REGISTER_OFFSET_SIO:
    err=dc1394_set_SIO_register(camera->camera_info, offset, value);
    break;
  case REGISTER_OFFSET_STROBE:
    err=dc1394_set_strobe_register(camera->camera_info, offset, value);
    break;
  default:
    fprintf(stderr,"Unrecognized offset!\n");
    break;
  }
  if (err!=DC1394_SUCCESS) {
    sprintf(string2,"Error");

    gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "register_data_entry")),string2);
  }

}


void
on_register_read_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{

  quadlet_t value;
  octlet_t offset;
  char *string;
  char string2[16];
  dc1394error_t err=DC1394_SUCCESS;

  string=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window, "register_address_entry")));
  sscanf(string,"%llx",&offset);

  //fprintf(stderr,"0x%llx\n",offset);

  switch(camera->register_offset) {
  case REGISTER_OFFSET_BASE:
    err=dc1394_get_register(camera->camera_info, offset, &value);
    break;
  case REGISTER_OFFSET_CRB:
    err=dc1394_get_control_register(camera->camera_info, offset, &value);
    break;
    /*
  case REGISTER_OFFSET_UD:
    err=dc1394_get_register(camera->camera_info, offset+camera->camera_info->unit_directory, &value);
    break;
    */
  case REGISTER_OFFSET_UDD:
    err=dc1394_get_register(camera->camera_info, offset+camera->camera_info->unit_dependent_directory, &value);
    break;
  case DC1394_VIDEO_MODE_FORMAT7_0:
  case DC1394_VIDEO_MODE_FORMAT7_1:
  case DC1394_VIDEO_MODE_FORMAT7_2:
  case DC1394_VIDEO_MODE_FORMAT7_3:
  case DC1394_VIDEO_MODE_FORMAT7_4:
  case DC1394_VIDEO_MODE_FORMAT7_5:
  case DC1394_VIDEO_MODE_FORMAT7_6:
  case DC1394_VIDEO_MODE_FORMAT7_7:
    err=dc1394_get_format7_register(camera->camera_info, camera->register_offset, offset, &value);
    break;
  case REGISTER_OFFSET_PIO:
    err=dc1394_get_PIO_register(camera->camera_info, offset, &value);
    break;
  case REGISTER_OFFSET_SIO:
    err=dc1394_get_SIO_register(camera->camera_info, offset, &value);
    break;
  case REGISTER_OFFSET_STROBE:
    err=dc1394_get_strobe_register(camera->camera_info, offset, &value);
  default:
    fprintf(stderr,"Unrecognized offset!\n");
    break;
  }
  if (err!=DC1394_SUCCESS)
    sprintf(string2,"Error");
  else
    sprintf(string2,"%08x",value);

  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "register_data_entry")),string2);
}


void
on_range_menu_activate             (GtkMenuItem     *menuitem,
				    gpointer         user_data)

{
  int feature;
  int action;

  // single auto variables:
  unsigned long int timeout_bin=0;
  unsigned long int step;
  dc1394feature_mode_t value=DC1394_FEATURE_MODE_ONE_PUSH_AUTO;

  action=((int)(unsigned long)user_data)%1000;
  feature=(((int)(unsigned long)user_data)-action)/1000;

  switch (action) {
  case RANGE_MENU_OFF : // ============================== OFF ==============================
    if (dc1394_feature_set_power(camera->camera_info, feature, DC1394_OFF)!=DC1394_SUCCESS)
      Error("Could not set feature on/off");
    else {
      camera->feature_set.feature[feature-DC1394_FEATURE_MIN].is_on=FALSE;
      UpdateRange(feature);
    }
    break;
  case RANGE_MENU_MAN : // ============================== MAN ==============================
      if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable) {
	if (dc1394_feature_set_power(camera->camera_info, feature, DC1394_ON)!=DC1394_SUCCESS) {
	  Error("Could not set feature on");
	  break;
	}
	else
	  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].is_on=TRUE;
      }
      if (dc1394_feature_set_mode(camera->camera_info, feature, DC1394_FEATURE_MODE_MANUAL)!=DC1394_SUCCESS)
	Error("Could not set manual mode");
      else {
	camera->feature_set.feature[feature-DC1394_FEATURE_MIN].current_mode=DC1394_FEATURE_MODE_MANUAL;
	if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable)
	  SetAbsoluteControl(feature, FALSE);
	UpdateRange(feature);
      }
      break;
  case RANGE_MENU_AUTO : // ============================== AUTO ==============================
    if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable) {
      if (dc1394_feature_set_power(camera->camera_info, feature, DC1394_ON)!=DC1394_SUCCESS) {
	Error("Could not set feature on");
	break;
      }
      else
	camera->feature_set.feature[feature-DC1394_FEATURE_MIN].is_on=TRUE;
    }
    if (dc1394_feature_set_mode(camera->camera_info, feature, DC1394_FEATURE_MODE_AUTO)!=DC1394_SUCCESS)
      Error("Could not set auto mode");
    else {
      camera->feature_set.feature[feature-DC1394_FEATURE_MIN].current_mode=DC1394_FEATURE_MODE_AUTO;
      if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable)
	SetAbsoluteControl(feature, FALSE);
      UpdateRange(feature);
    }
    break;
    case RANGE_MENU_SINGLE : // ============================== SINGLE ==============================
      if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable) {
	if (dc1394_feature_set_power(camera->camera_info, feature, DC1394_ON)!=DC1394_SUCCESS) {
	  Error("Could not set feature on");
	  break;
	}
	else
	  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].is_on=TRUE;
      }
      step=(unsigned long int)(1000000.0/preferences.auto_update_frequency);
      if (dc1394_feature_set_mode(camera->camera_info, feature, DC1394_FEATURE_MODE_ONE_PUSH_AUTO)!=DC1394_SUCCESS)
	Error("Could not start one-push operation");
      else {
	SetScaleSensitivity(GTK_WIDGET(menuitem),feature,FALSE);
	while ((value==DC1394_FEATURE_MODE_ONE_PUSH_AUTO) && (timeout_bin<(unsigned long int)(preferences.op_timeout*1000000.0)) ) {
	  usleep(step);
	  if (dc1394_feature_get_mode(camera->camera_info, feature, &value)!=DC1394_SUCCESS)
	    Error("Could not query one-push operation");
	  timeout_bin+=step;
	  UpdateRange(feature);
	}
	if (timeout_bin>=(unsigned long int)(preferences.op_timeout*1000000.0))
	  Warning("One-Push function timed-out!");

	if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable)
	  SetAbsoluteControl(feature, FALSE);
	UpdateRange(feature);
	// should switch back to manual mode here. Maybe a recursive call??
	// >> Not necessary because UpdateRange reloads the status which folds
	// back to 'man' in the camera
      }
      break;
  case RANGE_MENU_ABSOLUTE : // ============================== ABSOLUTE ==============================
    if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable) {
      if (dc1394_feature_set_power(camera->camera_info, feature, TRUE)!=DC1394_SUCCESS) {
	Error("Could not set feature on");
	break;
      }
      else
	camera->feature_set.feature[feature-DC1394_FEATURE_MIN].is_on=TRUE;
    }
    SetAbsoluteControl(feature, TRUE);
    UpdateRange(feature);
    break;
  }
}


void
on_key_bindings_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  help_window = create_help_window();
  BuildHelpWindow();
  gtk_widget_show(help_window);
}

/******************************
 *       PREFERENCES          *
 ******************************/

void
on_camera_name_text_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  // FIXME: update for multi-unit cameras
  char *tmp, *tmp_ptr;
  GtkWidget *item;
  const char *camera_name_str =  "coriander/camera_names/";
  tmp=(char*)malloc(STRING_SIZE*sizeof(char));
  tmp_ptr=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window, "camera_name_text")));
  sprintf(tmp,"%s%llx",camera_name_str, camera->camera_info->guid);
  gnome_config_set_string(tmp,tmp_ptr);
  gnome_config_sync();
  strcpy(camera->prefs.name,tmp_ptr);

  //eprint("name changed: updating menu to %s\n",tmp_ptr);
  item=GTK_WIDGET(GTK_BIN(GTK_OPTION_MENU(lookup_widget(main_window,"camera_select")))->child);
  gtk_label_set_text (GTK_LABEL (item),tmp_ptr);

  free(tmp);
}

void
on_prefs_op_timeout_scale_changed      (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.op_timeout=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(lookup_widget(preferences_window,"prefs_op_timeout_scale")));
  gnome_config_set_float("coriander/global/one_push_timeout",preferences.op_timeout);
  gnome_config_sync();
}


void
on_prefs_update_scale_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  preferences.auto_update_frequency=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(lookup_widget(preferences_window,"prefs_update_scale")));
  gnome_config_set_float("coriander/global/auto_update_frequency",preferences.auto_update_frequency);
  gnome_config_sync();
}

void
on_prefs_display_period_changed        (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.display_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_display_period")));
  gnome_config_set_int("coriander/display/period",camera->prefs.display_period);
  gnome_config_sync();
}
/*
void
on_prefs_display_scale_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  displaythread_info_t* info;
  chain_t* service;
  preferences.display_scale=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_display_scale")));
  gnome_config_set_int("coriander/display/scale",preferences.display_scale);
  gnome_config_sync();
  service=GetService(camera,SERVICE_DISPLAY);
  if (service!=NULL) {
    info=service->data;
    info->scale_previous=info->scale;
    info->scale=preferences.display_scale;
  }
}
*/
void
on_prefs_save_period_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.save_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_save_period")));
  gnome_config_set_int("coriander/save/period",camera->prefs.save_period);
  gnome_config_sync();
}


void
on_prefs_v4l_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.v4l_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_v4l_period")));
  gnome_config_set_int("coriander/v4l/period",camera->prefs.v4l_period);
  gnome_config_sync();
}

void
on_prefs_ftp_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.ftp_period=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"prefs_ftp_period")));
  gnome_config_set_int("coriander/ftp/period",camera->prefs.ftp_period);
  gnome_config_sync();
}


void
on_prefs_ftp_address_changed           (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_address")));
  strcpy(camera->prefs.ftp_address,tmp_ptr);
  gnome_config_set_string("coriander/ftp/address",camera->prefs.ftp_address);
  gnome_config_sync();
}


void
on_prefs_ftp_path_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_path")));
  strcpy(camera->prefs.ftp_path,tmp_ptr);
  gnome_config_set_string("coriander/ftp/path",camera->prefs.ftp_path);
  gnome_config_sync();

}


void
on_prefs_ftp_user_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_user")));
  strcpy(camera->prefs.ftp_user,tmp_ptr);
  gnome_config_set_string("coriander/ftp/user",camera->prefs.ftp_user);
  gnome_config_sync();

}


void
on_prefs_ftp_password_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_password")));
  strcpy(camera->prefs.ftp_password,tmp_ptr);
  // don't save passwords!
  //gnome_config_set_string("coriander/ftp/password",preferences.ftp_password);
  //gnome_config_sync();
}


void
on_prefs_ftp_filename_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  gchar *tmp;

  tmp_ptr=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_ftp_filename")));
  strcpy(camera->prefs.ftp_filename,tmp_ptr);
  strcpy(camera->prefs.ftp_filename_base,tmp_ptr);
  gnome_config_set_string("coriander/ftp/filename",camera->prefs.ftp_filename);
  gnome_config_sync();

  // extract extension and basename
  tmp = strrchr(camera->prefs.ftp_filename_base, '.');
  if(tmp != NULL) {
    tmp[0] = '\0';// cut filename before point
    strcpy(camera->prefs.ftp_filename_ext, strrchr(camera->prefs.ftp_filename,'.')+1);
  }
  else {
    // error: no extension provided
    // The following message should go in the FTP thread setup functions //////////////////////////
    // Error("You should provide an extension for the save filename. Default extension is RAW");
  }
  
}


void
on_prefs_update_power_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.auto_update=togglebutton->active;
  gnome_config_set_int("coriander/global/auto_update",preferences.auto_update);
  gnome_config_sync();
}
/*
void
on_prefs_save_seq_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active) {
    camera->prefs.save_mode=SAVE_MODE_SEQUENTIAL;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"use_ram_buffer")),0);
    gnome_config_set_int("coriander/save/mode",camera->prefs.save_mode);
    gnome_config_sync();
    UpdatePrefsSaveFrame();
    service=GetService(camera,SERVICE_SAVE);
    if (service!=NULL) {
      info=service->data;
      info->mode=camera->prefs.save_mode;
    }
  }
}


void
on_prefs_save_mode_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active) {
    camera->prefs.save_mode=SAVE_MODE_OVERWRITE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"use_ram_buffer")),0);
    gnome_config_set_int("coriander/save/mode",camera->prefs.save_mode);
    gnome_config_sync();
    UpdatePrefsSaveFrame();
    service=GetService(camera,SERVICE_SAVE);
    if (service!=NULL) {
      info=service->data;
      info->mode=camera->prefs.save_mode;
    }
  }
}


void
on_prefs_save_video_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active) {
    camera->prefs.save_mode=SAVE_MODE_VIDEO;
    gnome_config_set_int("coriander/save/mode",camera->prefs.save_mode);
    gnome_config_sync();
    UpdatePrefsSaveFrame();
    service=GetService(camera,SERVICE_SAVE);
    if (service!=NULL) {
      info=service->data;
      info->mode=camera->prefs.save_mode;
    }
  }
}

void
on_prefs_save_convert_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.save_convert=SAVE_CONVERT_ON;
  gnome_config_set_int("coriander/save/convert",camera->prefs.save_convert);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->rawdump=camera->prefs.save_convert;
  }
}


void
on_prefs_save_noconvert_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.save_convert=SAVE_CONVERT_OFF;
  gnome_config_set_int("coriander/save/convert",camera->prefs.save_convert);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->rawdump=camera->prefs.save_convert;
  }
}
*/

void
on_prefs_ftp_seq_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active) {
    camera->prefs.ftp_mode=FTP_MODE_SEQUENTIAL;
    gnome_config_set_int("coriander/ftp/mode",camera->prefs.ftp_mode);
    gnome_config_sync();
    UpdatePrefsFtpFrame();
  }
}


void
on_prefs_ftp_mode_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active) {
    camera->prefs.ftp_mode=FTP_MODE_OVERWRITE;
    gnome_config_set_int("coriander/ftp/mode",camera->prefs.ftp_mode);
    gnome_config_sync();
    UpdatePrefsFtpFrame();
  }
}


void
on_prefs_display_keep_ratio_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  camera->prefs.display_keep_ratio=togglebutton->active;
  gnome_config_set_int("coriander/display/keep_ratio",camera->prefs.display_keep_ratio);
  gnome_config_sync();
}


void
on_prefs_v4l_dev_name_changed      (GtkEditable     *editable,
                                  gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"prefs_v4l_dev_name")));
  strcpy(camera->prefs.v4l_dev_name,tmp_ptr);
  gnome_config_set_string("coriander/v4l/v4l_dev_name",camera->prefs.v4l_dev_name);
  gnome_config_sync();

}


void
on_bayer_menu_activate           (GtkMenuItem     *menuitem,
				  gpointer         user_data)
{
  int tmp;
  tmp=(int)(unsigned long)user_data;
  
  pthread_mutex_lock(&camera->uimutex);
  camera->bayer=tmp;
  pthread_mutex_unlock(&camera->uimutex);
  UpdateOptionFrame();
}

void
on_bayer_pattern_menu_activate           (GtkMenuItem     *menuitem,
					  gpointer         user_data)
{
  int tmp;
  tmp=(int)user_data;
  
  camera->bayer_pattern=tmp;
}

void
on_stereo_menu_activate               (GtkToggleButton *menuitem,
                                       gpointer         user_data)
{
  int tmp;
  tmp=(int)user_data;
  
  camera->stereo=tmp;

  UpdateOptionFrame();

}


void
on_trigger_count_changed               (GtkEditable     *editable,
                                        gpointer         user_data)
{
  int value;
  value=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"trigger_count")));

  if (dc1394_feature_set_value(camera->camera_info, DC1394_FEATURE_TRIGGER, value)!=DC1394_SUCCESS)
    Error("Could not set external trigger count");
  else
    camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].value=value;
}


void
on_mono16_bpp_changed                  (GtkEditable     *editable,
                                        gpointer         user_data)
{
  int value;
  value=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"mono16_bpp")));

  camera->bpp=value;
}


void
on_abs_entry_activate              (GtkEditable     *editable,
                                    gpointer         user_data)
{ 
  int feature;
  feature=(int)(unsigned long)user_data;
  SetAbsValue(feature);
}


void
on_global_iso_stop_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  dc1394switch_t status;
  camera_t* camera_ptr;
  camera_ptr=cameras;
  dc1394switch_t is_iso_on;

  while (camera_ptr!=NULL) {
    if (dc1394_video_get_transmission(camera_ptr->camera_info, &is_iso_on)!=DC1394_SUCCESS) {
      Error("Could not get ISO status");
    }
    if (is_iso_on==DC1394_TRUE) {
      if (dc1394_video_set_transmission(camera_ptr->camera_info, DC1394_OFF)!=DC1394_SUCCESS) {
	Error("Could not stop ISO transmission");
      }
      else {
	if (dc1394_video_get_transmission(camera_ptr->camera_info, &status)!=DC1394_SUCCESS)
	  Error("Could not get ISO status");
	else {
	  if (status==DC1394_TRUE)
	    Error("Broacast ISO stop failed for a camera");
	}
      } 
    }
    camera_ptr=camera_ptr->next; 
  }

  UpdateIsoFrame();
  UpdateTransferStatusFrame();
}


void
on_global_iso_restart_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
  on_global_iso_stop_clicked(GTK_BUTTON(lookup_widget(main_window,"global_iso_stop")),NULL);
  on_global_iso_start_clicked(GTK_BUTTON(lookup_widget(main_window,"global_iso_start")),NULL);
}


void
on_global_iso_start_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{  
  dc1394switch_t status;
  camera_t* camera_ptr;
  camera_ptr=cameras;
  dc1394switch_t is_iso_on;

  while (camera_ptr!=NULL) {
    if (dc1394_video_get_transmission(camera_ptr->camera_info, &is_iso_on)!=DC1394_SUCCESS) {
      Error("Could not get ISO status");
    }
    if (is_iso_on==DC1394_FALSE) {
      if (dc1394_video_set_transmission(camera_ptr->camera_info, DC1394_ON)!=DC1394_SUCCESS) {
	Error("Could not start ISO transmission");
      }
      else {
	if (dc1394_video_get_transmission(camera_ptr->camera_info, &status)!=DC1394_SUCCESS)
	  Error("Could not get ISO status");
	else {
	  if (status==DC1394_FALSE)
	    Error("Broacast ISO start failed for a camera");
	}
      } 
    }
    camera_ptr=camera_ptr->next; 
  }

  UpdateIsoFrame();
  UpdateTransferStatusFrame();
}

/*
void
on_prefs_save_date_tag_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.save_datenum=SAVE_TAG_DATE;
  gnome_config_set_int("coriander/save/datenum",camera->prefs.save_datenum);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->datenum=camera->prefs.save_datenum;
  }
}


void
on_prefs_save_num_tag_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  savethread_info_t* info;
  chain_t* service;
  if (togglebutton->active)
    camera->prefs.save_datenum=SAVE_TAG_NUMBER;
  gnome_config_set_int("coriander/save/datenum",camera->prefs.save_datenum);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
  service=GetService(camera,SERVICE_SAVE);
  if (service!=NULL) {
    info=service->data;
    info->datenum=camera->prefs.save_datenum;
  }
}
*/

void
on_prefs_ftp_date_tag_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    camera->prefs.ftp_datenum=FTP_TAG_DATE;
  gnome_config_set_int("coriander/ftp/datenum",camera->prefs.ftp_datenum);
  gnome_config_sync();
  UpdatePrefsFtpFrame();
}


void
on_prefs_ftp_num_tag_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    camera->prefs.ftp_datenum=FTP_TAG_NUMBER;
  gnome_config_set_int("coriander/ftp/datenum",camera->prefs.ftp_datenum);
  gnome_config_sync();
  UpdatePrefsFtpFrame();
}


void
on_ram_buffer_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  camera->prefs.use_ram_buffer=togglebutton->active;
  gnome_config_set_int("coriander/save/use_ram_buffer",camera->prefs.use_ram_buffer);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}


void
on_ram_buffer_size_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.ram_buffer_size=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"ram_buffer_size")));
  gnome_config_set_int("coriander/save/ram_buffer_size",camera->prefs.ram_buffer_size);
  gnome_config_sync();
  //fprintf(stderr,"Buffer size: %d\n",camera->prefs.ram_buffer_size);
  UpdatePrefsSaveFrame();
}


void
on_malloc_test_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  unsigned char *temp;

  // test if we can allocate enough memory
  temp=(unsigned char*)malloc(camera->prefs.ram_buffer_size*1024*1024*sizeof(unsigned char));
  
  if (temp==NULL)
    Warning("\tFailed to allocate memory");
  else {
    Warning("\tAllocation succeeded");
    free(temp);
  }
}


void
on_dma_buffer_size_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.dma_buffer_size=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(main_window,"dma_buffer_size")));
  gnome_config_set_int("coriander/receive/dma_buffer_size",camera->prefs.dma_buffer_size);
  gnome_config_sync();
}


void
on_display_redraw_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    camera->prefs.display_redraw=DISPLAY_REDRAW_ON;
  else
    camera->prefs.display_redraw=DISPLAY_REDRAW_OFF;
    
  gnome_config_set_int("coriander/display/redraw",camera->prefs.display_redraw);
  gnome_config_sync();
  UpdatePrefsDisplayFrame();
}


void
on_display_redraw_rate_changed         (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.display_redraw_rate=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(lookup_widget(main_window,"display_redraw_rate")));
  gnome_config_set_float("coriander/display/redraw_rate",camera->prefs.display_redraw_rate);
  gnome_config_sync();
  UpdatePrefsDisplayFrame();
}


void
on_broadcast_button_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if(togglebutton->active>0) {
    dc1394_camera_set_broadcast(camera->camera_info, DC1394_ON);
  }
  else {
    dc1394_camera_set_broadcast(camera->camera_info, DC1394_OFF);
  }
  camera->prefs.broadcast=togglebutton->active;
  gnome_config_set_int("coriander/global/broadcast",camera->prefs.broadcast);
  gnome_config_sync();
}


void
on_overlay_byte_order_YUYV_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.overlay_byte_order=DC1394_BYTE_ORDER_YUYV;
  else
    preferences.overlay_byte_order=DC1394_BYTE_ORDER_UYVY;
  gnome_config_set_int("coriander/global/overlay_byte_order",preferences.overlay_byte_order);
  gnome_config_sync();
}


void
on_overlay_byte_order_UYVY_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  if (togglebutton->active)
    preferences.overlay_byte_order=DC1394_BYTE_ORDER_UYVY;
  else
    preferences.overlay_byte_order=DC1394_BYTE_ORDER_YUYV;
  gnome_config_set_int("coriander/global/overlay_byte_order",preferences.overlay_byte_order);
  gnome_config_sync();
}


void
on_overlay_type_menu_activate           (GtkMenuItem     *menuitem,
					 gpointer         user_data)
{
  camera->prefs.overlay_type=(int)(unsigned long)user_data;
  gnome_config_set_int("coriander/display/overlay_type",camera->prefs.overlay_type);
  gnome_config_sync();
  UpdatePrefsDisplayOverlayFrame();
}

void
on_overlay_pattern_menu_activate        (GtkMenuItem     *menuitem,
					 gpointer         user_data)
{
  camera->prefs.overlay_pattern=(int)(unsigned long)user_data;
  gnome_config_set_int("coriander/display/overlay_pattern",camera->prefs.overlay_pattern);
  gnome_config_sync();
  UpdatePrefsDisplayOverlayFrame();
}

void
on_overlay_color_picker_color_set      (GnomeColorPicker *gnomecolorpicker,
                                        guint            arg1,
                                        guint            arg2,
                                        guint            arg3,
                                        guint            arg4,
                                        gpointer         user_data)
{
  //fprintf(stderr,"0x%x 0x%x 0x%x\n",arg1,arg2,arg3);
  camera->prefs.overlay_color_r=arg1>>8;
  camera->prefs.overlay_color_g=arg2>>8;
  camera->prefs.overlay_color_b=arg3>>8;
  gnome_config_set_int("coriander/display/overlay_color_r",camera->prefs.overlay_color_r);
  gnome_config_set_int("coriander/display/overlay_color_g",camera->prefs.overlay_color_g);
  gnome_config_set_int("coriander/display/overlay_color_b",camera->prefs.overlay_color_b);
  gnome_config_sync();
}


void
on_overlay_file_subentry_changed       (GtkEditable     *editable,
                                        gpointer         user_data)
{
  char *tmp_ptr;
  tmp_ptr=(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"overlay_file_subentry")));
  strcpy(camera->prefs.overlay_filename,tmp_ptr);
  gnome_config_set_string("coriander/display/overlay_filename",camera->prefs.overlay_filename);
  gnome_config_sync();
}


void
on_save_filename_subentry_changed      (GtkEditable     *editable,
                                        gpointer         user_data)
{
  strcpy(camera->prefs.save_filename,(char*)gtk_entry_get_text(GTK_ENTRY(lookup_widget(main_window,"save_filename_subentry"))));
  gnome_config_set_string("coriander/save/filename",camera->prefs.save_filename);
  gnome_config_sync();

  //BuildSaveModeMenu();
  UpdateSaveFilenameFrame();

}


void
on_grab_now_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_save_format_menu_activate             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  // rebuild the append menu if we switch between video and still formats
  if ((((int)(unsigned long)user_data>=SAVE_FORMAT_RAW_VIDEO)&&(camera->prefs.save_format<SAVE_FORMAT_RAW_VIDEO)) ||
      (((int)(unsigned long)user_data<SAVE_FORMAT_RAW_VIDEO)&&(camera->prefs.save_format>=SAVE_FORMAT_RAW_VIDEO))) {
    camera->prefs.save_format=(int)(unsigned long)user_data;
    BuildSaveAppendMenu();
  }
  else
    camera->prefs.save_format=(int)(unsigned long)user_data;

  switch (camera->prefs.save_format) {
  case SAVE_FORMAT_PPM:
      sprintf(camera->prefs.save_filename_ext,"ppm");
      break;
  case SAVE_FORMAT_RAW:
      sprintf(camera->prefs.save_filename_ext,"raw");
      break;
#ifdef HAVE_FFMPEG
  case SAVE_FORMAT_JPEG:
      sprintf(camera->prefs.save_filename_ext,"jpeg");
      break;
#endif
  case SAVE_FORMAT_RAW_VIDEO:
      sprintf(camera->prefs.save_filename_ext,"raw");
      break;
  case SAVE_FORMAT_PVN:
      sprintf(camera->prefs.save_filename_ext,"pvn");
      break;
#ifdef HAVE_FFMPEG
  case SAVE_FORMAT_MPEG:
      sprintf(camera->prefs.save_filename_ext,"mpeg");
      break;
#endif
  default:
      sprintf(camera->prefs.save_filename_ext,"unknown");
      break;
  }
  gnome_config_set_int("coriander/save/format",camera->prefs.save_format);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}

void
on_save_append_menu_activate             (GtkEditable     *editable,
                                        gpointer         user_data)
{
  camera->prefs.save_append=(int)(unsigned long)user_data;
  gnome_config_set_int("coriander/save/append",camera->prefs.save_append);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}

void
on_save_to_dir_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  camera->prefs.save_to_dir=togglebutton->active;
  gnome_config_set_int("coriander/save/save_to_dir",camera->prefs.save_to_dir);
  gnome_config_sync();
  BuildSaveAppendMenu();
  UpdatePrefsSaveFrame();
}


void
on_iso_nodrop_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  camera->prefs.iso_nodrop=togglebutton->active;
  gnome_config_set_int("coriander/receive/iso_nodrop",camera->prefs.iso_nodrop);
  gnome_config_sync();
}


void
on_save_to_stdout_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  camera->prefs.save_to_stdout=togglebutton->active;
  gnome_config_set_int("coriander/save/save_to_stdout",camera->prefs.save_to_stdout);
  gnome_config_sync();
  UpdatePrefsSaveFrame();
}


void
on_error_popup_button_activate         (GtkButton       *button,
                                        gpointer         user_data)
{
  // close popup window
  // (automatic)
  /*
  // release process
  pthread_mutex_lock(&message_mutex);
  message_request=0;
  pthread_mutex_unlock(&message_mutex);
  */
  // reset main window sensitiveness
  gtk_widget_set_sensitive(main_window,1);
  mainthread_info.dialog_clicked=1;

}

void
on_prefs_no_overwrite_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.no_overwrite=togglebutton->active;
  gnome_config_set_float("coriander/global/no_overwrite",preferences.no_overwrite);
  gnome_config_sync();

}


void
on_prefs_warning_in_popup_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.warning_in_popup=togglebutton->active;
  gnome_config_set_float("coriander/global/warning_in_popup",preferences.warning_in_popup);
  gnome_config_sync();
}


void
on_prefs_error_in_popup_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.error_in_popup=togglebutton->active;
  gnome_config_set_float("coriander/global/error_in_popup",preferences.error_in_popup);
  gnome_config_sync();
}


void
on_prefs_auto_receive_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.automate_receive=togglebutton->active;
  gnome_config_set_float("coriander/global/automate_receive",preferences.automate_receive);
  gnome_config_sync();
}


void
on_prefs_auto_iso_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
  preferences.automate_iso=togglebutton->active;
  gnome_config_set_float("coriander/global/automate_iso",preferences.automate_iso);
  gnome_config_sync();
}


void
on_bmode_button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

  if (togglebutton->active){
    //fprintf(stderr,"setting 1394b\n");
    dc1394_video_set_operation_mode(camera->camera_info, DC1394_OPERATION_MODE_1394B);
    BuildIsoSpeedMenu();
  }
  else {
    dc1394speed_t isospeed;
    dc1394_video_get_iso_speed(camera->camera_info,&isospeed);
    if (isospeed>DC1394_ISO_SPEED_400)
      dc1394_video_set_iso_speed(camera->camera_info,DC1394_ISO_SPEED_400);

    //fprintf(stderr,"setting legacy\n");
    dc1394_video_set_operation_mode(camera->camera_info, DC1394_OPERATION_MODE_LEGACY);
    BuildIsoSpeedMenu();
  }

}

