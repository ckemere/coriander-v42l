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
BuildCameraFrame(void)
{
  BuildCameraMenu();
}

void
BuildServiceFrame(void)
{
#ifdef HAVE_FTPLIB
  gtk_widget_set_sensitive(lookup_widget(main_window,"service_ftp"),TRUE);
#else
  gtk_widget_set_sensitive(lookup_widget(main_window,"service_ftp"),FALSE);
#endif
#ifdef HAVE_SDLLIB
  gtk_widget_set_sensitive(lookup_widget(main_window,"service_display"),TRUE);
#else
  gtk_widget_set_sensitive(lookup_widget(main_window,"service_display"),FALSE);
#endif
  // add V4L test here ??

}

void
BuildTriggerFrame(void)
{

  // the following line is necessary in order not to have unsensitive menu items:
  gtk_widget_set_sensitive(lookup_widget(main_window,"trigger_frame"),TRUE);

  //eprint("test1\n");
  BuildTriggerModeMenu();
  //eprint("test2\n");
  BuildFpsMenu();
  //eprint("test3\n");
  BuildTriggerSourceMenu();
  //eprint("test4\n");
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"trigger_external")),
			       camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].is_on);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"trigger_polarity")),
			       camera->feature_set.feature[DC1394_FEATURE_TRIGGER-DC1394_FEATURE_MIN].trigger_polarity);
}


void
BuildPowerFrame(void)
{
  // these two functions are always present:
  gtk_widget_set_sensitive(lookup_widget(main_window,"power_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"power_reset"),TRUE);

  // activate if camera capable of power on/off:
  gtk_widget_set_sensitive(lookup_widget(main_window,"power_on"),(camera->camera_info->can_switch_on_off));
  gtk_widget_set_sensitive(lookup_widget(main_window,"power_off"),(camera->camera_info->can_switch_on_off));
}


void
BuildMemoryFrame(void)
{
  // the following line is necessary in order not to have unsensitive menu items:
  gtk_widget_set_sensitive(lookup_widget(main_window,"memory_frame"),TRUE);

  // activate the mem channel menu:
  BuildMemoryChannelMenu();

}

void
BuildIsoFrame(void)
{
  // TODO: only if ISO capable
  dc1394switch_t is_iso_on;
  if (dc1394_video_get_transmission(camera->camera_info, &is_iso_on)!=DC1394_SUCCESS)
    Error("Can't get ISO status");
  dc1394operation_mode_t mode;
  if (dc1394_video_get_operation_mode(camera->camera_info, &mode)!=DC1394_SUCCESS)
    Error("Can't get operation mode");
  //fprintf(stderr,"bmode: %d, 1394b: %d, capable: %d\n",mode,DC1394_OPERATION_MODE_1394B,camera->camera_info->bmode_capable);

  //fprintf(stderr,"sync: %d\n",preferences.sync_control);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_start"),!is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_restart"),is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(main_window,"iso_stop"),is_iso_on);
  gtk_widget_set_sensitive(lookup_widget(main_window,"bmode_button"),camera->camera_info->bmode_capable);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"broadcast_button")),camera->prefs.broadcast);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"iso_nodrop")),camera->prefs.iso_nodrop);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"bmode_button")),camera->camera_info->bmode_capable&&(mode==DC1394_OPERATION_MODE_1394B));
    
  BuildIsoSpeedMenu();
}

void
BuildGlobalIsoFrame(void)
{
  // TODO: only if ISO capable
  gtk_widget_set_sensitive(lookup_widget(main_window,"global_iso_frame"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"global_iso_start"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"global_iso_restart"),TRUE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"global_iso_stop"),TRUE);

}

void
BuildFormat7ModeFrame(void)
{
  BuildFormat7ModeMenu();
  //eprint("test\n");
  BuildFormat7ColorMenu();
}

void
BuildCameraStatusFrame(void)
{ 
}

void
BuildRegisterAccessFrame(void)
{
  //fprintf(stderr,"build register menu\n");
  BuildRegisterAccessOffsetMenu();
}

void
BuildTransferStatusFrame(void)
{
}

void
BuildPrefsSaveFrame(void)
{
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window, "prefs_save_period"), camera->prefs.save_period);

  //filename
  gtk_entry_set_text(GTK_ENTRY(gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(lookup_widget(main_window, "save_filename_entry")))), camera->prefs.save_filename);

  // menus
  BuildSaveFormatMenu();
  BuildSaveAppendMenu();

  // save to dir mode:
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "save_to_dir"),camera->prefs.save_to_dir>0);

  // save to stdout:
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "save_to_stdout"),camera->prefs.save_to_stdout>0);

  // ram buffer
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "use_ram_buffer"),camera->prefs.use_ram_buffer);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,"ram_buffer_size"), camera->prefs.ram_buffer_size);

}

void
BuildPrefsV4lFrame(void)
{
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,
							  "prefs_v4l_period"), camera->prefs.v4l_period);
  //filename
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_v4l_dev_name")), camera->prefs.v4l_dev_name);
}

void
BuildPrefsGeneralFrame(void)
{
  
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,"prefs_op_timeout_scale"), preferences.op_timeout);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(preferences_window,"prefs_update_scale"), preferences.auto_update_frequency);
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window, "prefs_no_overwrite"), preferences.no_overwrite);
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window, "prefs_warning_in_popup"), preferences.warning_in_popup);
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window, "prefs_error_in_popup"), preferences.error_in_popup);
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window, "prefs_auto_receive"), preferences.automate_receive);
  gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(preferences_window, "prefs_auto_iso"), preferences.automate_iso);
}

void
BuildPrefsFtpFrame(void)
{
#ifdef HAVE_FTPLIB
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window, "prefs_ftp_period"), camera->prefs.ftp_period);
  // mode
  switch(camera->prefs.ftp_mode) {
  case FTP_MODE_OVERWRITE:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_ftp_mode"),TRUE);
    break;
  case FTP_MODE_SEQUENTIAL:
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_ftp_seq"),TRUE);
    break;
  }
  // file,... names
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_ftp_filename")), camera->prefs.ftp_filename);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_ftp_address")), camera->prefs.ftp_address);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_ftp_path")), camera->prefs.ftp_path);
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(main_window, "prefs_ftp_user")), camera->prefs.ftp_user);

  //fprintf(stderr,"%s\n",camera->prefs.ftp_filename);
  //fprintf(stderr,"%s\n",camera->prefs.ftp_address);
  //fprintf(stderr,"%s\n",camera->prefs.ftp_path);
  //fprintf(stderr,"%s\n",camera->prefs.ftp_user);

#else

  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_framedrop_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_mode_frame"),FALSE);
  gtk_widget_set_sensitive(lookup_widget(main_window,"prefs_ftp_server_frame"),FALSE);

#endif

  // file sequence tags
  if (camera->prefs.ftp_datenum==FTP_TAG_DATE)
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_ftp_date_tag"),TRUE);
  else
    gtk_toggle_button_set_active((GtkToggleButton*)lookup_widget(main_window, "prefs_ftp_num_tag"),TRUE);
}

void
BuildPrefsDisplayFrame(void)
{
  //GtkAdjustment* adj;
  //chain_t* service;
  // frame drop
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,"prefs_display_period"), camera->prefs.display_period);

  // keep aspect ratio
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"prefs_display_keep_ratio")), camera->prefs.display_keep_ratio);

  // display redraw
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(main_window,"display_redraw")), camera->prefs.display_redraw==DISPLAY_REDRAW_ON);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,"display_redraw_rate"), camera->prefs.display_redraw_rate);

  BuildOverlayPatternMenu();
  BuildOverlayTypeMenu();

  gnome_color_picker_set_i8(GNOME_COLOR_PICKER(lookup_widget(main_window,"overlay_color_picker")),
			    camera->prefs.overlay_color_r,
			    camera->prefs.overlay_color_g,
			    camera->prefs.overlay_color_b,0);
  
  /*

  // display scaling
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window,"prefs_display_scale"), camera->prefs.display_scale);
  
  adj=gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON (lookup_widget(main_window, "prefs_display_scale")));

  service=GetService(camera, SERVICE_DISPLAY);
  if (service==NULL) {
    adj->upper=10;
    adj->lower=0;
  }
  else {
    // do something smart here...
    adj->upper=10;
    adj->lower=0;
  }
  adj->step_increment=1;
  adj->page_increment=1;
  if (adj->value<adj->lower)
    adj->value=adj->lower;
  if (adj->value>adj->upper)
    adj->value=adj->upper;
  g_signal_emit_by_name((gpointer) adj, "changed");
  
  */
}

void
BuildPrefsReceiveFrame(void)
{
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window, "dma_buffer_size"), camera->prefs.dma_buffer_size);
}

void
BuildOptionFrame(void)
{
  
  dc1394_video_get_data_depth(camera->camera_info, &camera->bpp);

  pthread_mutex_lock(&camera->uimutex);
  gtk_spin_button_set_value((GtkSpinButton*)lookup_widget(main_window, "mono16_bpp"),camera->bpp);
  pthread_mutex_unlock(&camera->uimutex);
  BuildBayerMenu();
  BuildBayerPatternMenu();
  BuildStereoMenu();
}

void
BuildBandwidthFrame(void)
{

  GtkWidget *bandwidth_table;
  GtkWidget *label;
  char* temp;
  int nports;

  temp=(char*)malloc(STRING_SIZE*sizeof(char));

  // get the number of ports
  nports=port_num;

  //destroy table first.
  gtk_widget_destroy(GTK_WIDGET (lookup_widget(main_window, "bandwidth_table")));

  // build new table
  bandwidth_table = gtk_table_new (nports, 5, TRUE);
  gtk_widget_ref (bandwidth_table);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "bandwidth_table", bandwidth_table,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bandwidth_table);
    
  gtk_container_add (GTK_CONTAINER (gtk_notebook_get_nth_page(GTK_NOTEBOOK(lookup_widget(main_window,"notebook7")),1)), bandwidth_table);
  gtk_container_set_border_width (GTK_CONTAINER (bandwidth_table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (bandwidth_table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (bandwidth_table), 2);

#ifdef HAVE_GET_PORT
  int i;
  GtkWidget *bandwidth_bar;

  // build each bandwidth bar:
  for (i=0;i<nports;i++) {
    sprintf(temp,"Bus %d: ",i);
    label = gtk_label_new (_(temp));
    gtk_widget_ref (label);
    sprintf(temp,"label_bandwidth%d",i);
    gtk_object_set_data_full (GTK_OBJECT (main_window), temp, label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (bandwidth_table), label, 0, 1, i, i+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    bandwidth_bar = gtk_progress_bar_new();
    gtk_widget_ref (bandwidth_bar);
    sprintf(temp,"bandwidth_bar%d",i);
    gtk_object_set_data_full (GTK_OBJECT (main_window), temp, bandwidth_bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (bandwidth_bar);
    gtk_table_attach (GTK_TABLE (bandwidth_table), bandwidth_bar, 1, 5, i, i+1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_progress_set_show_text(GTK_PROGRESS (bandwidth_bar), 1);
    gtk_progress_set_text_alignment(GTK_PROGRESS (bandwidth_bar), .5, .5);
    gtk_progress_set_format_string(GTK_PROGRESS (bandwidth_bar),"%p %%");
  }

#else

    sprintf(temp,"Bandwidth information not available with your version of libdc1394");
    label = gtk_label_new (_(temp));
    gtk_widget_ref (label);
    sprintf(temp,"label_bandwidth");
    gtk_object_set_data_full (GTK_OBJECT (main_window), temp, label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (bandwidth_table), label, 0, 5, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

#endif


  free(temp);

}


void
BuildSeviceTreeFrame(void)
{   
  int i;
  camera_t *camera_ptr;
  GtkTreeViewColumn *col;
  GtkCellRenderer   *renderer;
  GtkTreeView       *tree_view;
  GtkTreeStore      *treestore;
  GtkTreeIter       toplevel;
  GtkTreeModel      *model;

  //fprintf(stderr,"Start building tree...");
  tree_view=(GtkTreeView*)lookup_widget(main_window,"service_tree");

  // clear current view
  col=gtk_tree_view_get_column (tree_view,0);
  while (col!=NULL) {
    gtk_tree_view_remove_column (tree_view,col);
    col=gtk_tree_view_get_column (tree_view,0);
  }

  // --- First column ---
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Camera");

  // pack tree view column into tree view
  gtk_tree_view_append_column(tree_view, col);
  renderer = gtk_cell_renderer_text_new();
  g_object_set(renderer, "weight", PANGO_WEIGHT_BOLD, "weight-set", TRUE, NULL); // set bold text

  // pack cell renderer into tree view column
  gtk_tree_view_column_pack_start(col, renderer, TRUE);

  // connect 'text' property of the cell renderer to model column that contains the camera
  gtk_tree_view_column_add_attribute(col, renderer, "text", 0);

  // --- Second column ---
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Framerate");

  // pack tree view column into tree view
  gtk_tree_view_append_column(tree_view, col);
  renderer = gtk_cell_renderer_text_new();

  // pack cell renderer into tree view column
  gtk_tree_view_column_pack_start(col, renderer, TRUE);

  // connect 'text' property of the cell renderer to model column that contains the camera
  gtk_tree_view_column_add_attribute(col, renderer, "text", 1);

  // --- Third column ---
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Processed");

  // pack tree view column into tree view
  gtk_tree_view_append_column(tree_view, col);
  renderer = gtk_cell_renderer_text_new();

  // pack cell renderer into tree view column
  gtk_tree_view_column_pack_start(col, renderer, TRUE);

  // connect 'text' property of the cell renderer to model column that contains the camera
  gtk_tree_view_column_add_attribute(col, renderer, "text", 2);

  // --- Fourth column ---
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Drop warnings");

  // pack tree view column into tree view
  gtk_tree_view_append_column(tree_view, col);
  renderer = gtk_cell_renderer_text_new();

  // pack cell renderer into tree view column
  gtk_tree_view_column_pack_start(col, renderer, TRUE);

  // connect 'text' property of the cell renderer to model column that contains the camera
  gtk_tree_view_column_add_attribute(col, renderer, "text", 3);

  // --- create model and fill with camera names ---
  treestore = gtk_tree_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT); 
  // last value is hidden but used for deletion, etc.
  // It stores the service type (e.g. SERVICE_ISO) or the camera number

  // Append two cameras
  camera_ptr=cameras;
  i=0;
  while (camera_ptr!=NULL) {
    gtk_tree_store_append(treestore, &toplevel, NULL);
    gtk_tree_store_set(treestore, &toplevel, 0, camera_ptr->prefs.name, 1, "", 2, "", 3, "", 4, i++, -1);
    camera_ptr=camera_ptr->next;
  }

  model = GTK_TREE_MODEL(treestore);

  gtk_tree_view_set_model(tree_view, model);

  g_object_unref(model); // destroy model automatically with view
  
  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(tree_view), GTK_SELECTION_NONE);

  //fprintf(stderr,"done\n");

}

