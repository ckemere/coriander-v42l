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

extern const char *help_key_bindings_keys[KEY_BINDINGS_NUM];
extern const char *help_key_bindings_functions[KEY_BINDINGS_NUM];

#define FORMAT7_TABLE_WIDTH 7

void
BuildPreferencesWindow(void)
{
  BuildPrefsGeneralFrame();
  BuildPrefsDisplayFrame();
  BuildPrefsReceiveFrame();
  BuildPrefsSaveFrame();
  BuildPrefsFtpFrame();
  BuildPrefsV4lFrame();
}

void
BuildFormat7Window(void)
{
  int f;
  GtkWidget* notebook2;
  GtkWidget* vbox;
  GtkWidget* spin;
  GtkWidget* table;
  GtkWidget* frame;
  GtkWidget* label;
  GtkWidget* frame_title;
  GtkWidget* bar;
  GtkWidget* scale;

  //eprint("building F7 window\n");

  // this window is built only if the camera supports F7. If there is a support,
  // the default edit mode is either the currently selected mode (F7 active) or
  // the first available mode (F7 inactive)

  dc1394video_mode_t video_mode;
  dc1394_video_get_mode(camera->camera_info,&video_mode);

  // if we are using F7, choose current F7 mode as default
  if (dc1394_is_video_mode_scalable(video_mode)) {
    camera->format7_info.edit_mode=video_mode;
  }
  // if we are NOT using F7, check if an F7 mode is supported and use the first one as default
  else { 
    // get first supported F7 mode
    for (f=DC1394_VIDEO_MODE_FORMAT7_MIN;f<=DC1394_VIDEO_MODE_FORMAT7_MAX;f++) {
      if (camera->format7_info.modeset.mode[f-DC1394_VIDEO_MODE_FORMAT7_MIN].present>0) {
	f++;
	break;
      }
    }
    f--;

    if (camera->format7_info.modeset.mode[f-DC1394_VIDEO_MODE_FORMAT7_MIN].present==0) {
      // F7 not supported. don't build anything
      camera->format7_info.edit_mode=-1;
    }
    else {
      camera->format7_info.edit_mode=f;
    }
  }
  //eprint("finished edit mode seletion: %d\n",camera->format7_info.edit_mode);

  notebook2=lookup_widget(main_window,"notebook2");
  // if the page exists, remove it:
  if (format7_tab_presence==1) {
    //eprint("page exists, removing\n");
    gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2),3);
    format7_tab_presence=0;
  }

  //eprint("check 0\n");
  if (camera->format7_info.edit_mode>=0) {

    //eprint("check 0b\n");
    label = gtk_label_new (_("ROI"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_format7_page", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    //eprint("check 1\n");

    // big vbox for the whole tab
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_ref (vbox);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "vbox34", vbox,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vbox);

    // the format7 tab should be placed in position 3 here:
    gtk_notebook_insert_page(GTK_NOTEBOOK (notebook2), GTK_WIDGET(vbox),label, 3);

    // mode frame
    frame = gtk_frame_new (NULL);
    gtk_widget_ref (frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_mode_frame", frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    frame_title = gtk_label_new (_("<b>Current mode</b>"));
    gtk_widget_show (frame_title);
    gtk_frame_set_label_widget (GTK_FRAME (frame), frame_title);
    gtk_label_set_use_markup (GTK_LABEL (frame_title), TRUE);

    table = gtk_table_new (1, 3, FALSE);
    gtk_widget_ref (table);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table19", table,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);

    label = gtk_label_new (_("Coding:"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label13", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_padding (GTK_MISC (label), 6, 0);

    //frame
    frame = gtk_frame_new (NULL);
    gtk_widget_ref (frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_setup_frame", frame, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    frame_title = gtk_label_new (_("<b>Mode Setup</b>"));
    gtk_widget_show (frame_title);
    gtk_frame_set_label_widget (GTK_FRAME (frame), frame_title);
    gtk_label_set_use_markup (GTK_LABEL (frame_title), TRUE);

    // table
    table = gtk_table_new (2, 2, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE (table),5);
    gtk_container_set_border_width(GTK_CONTAINER (table),5);
    gtk_table_set_row_spacings(GTK_TABLE (table),5);
    gtk_table_set_homogeneous (GTK_TABLE(table),TRUE);
    gtk_widget_ref (table);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "f7setup_table", table, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);

    // packet size
    label = gtk_label_new (_("Packet Size"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_psize", label, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_psize", scale, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (GTK_TABLE (table), scale, 2, FORMAT7_TABLE_WIDTH-1, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (scale), 0);

    spin = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)), 1, 0);
    gtk_widget_ref (spin);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_psize_spin", spin, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin);
    gtk_table_attach (GTK_TABLE (table), spin, FORMAT7_TABLE_WIDTH-1, FORMAT7_TABLE_WIDTH, 0, 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin, TRUE);

    // horizontal size
    label = gtk_label_new (_("Horizontal Size"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_hsize", label, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 2, 1, 2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_hsize_scale", scale, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (GTK_TABLE (table), scale, 2, FORMAT7_TABLE_WIDTH-1, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (scale), 0);

    spin = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)), 1, 0);
    gtk_widget_ref (spin);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_hsize_spin", spin, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin);
    gtk_table_attach (GTK_TABLE (table), spin, FORMAT7_TABLE_WIDTH-1, FORMAT7_TABLE_WIDTH, 1, 2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin, TRUE);

    // horizontal position
    label = gtk_label_new (_("Horizontal Position"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_hpos", label, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 2, 2, 3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_hpos_scale", scale, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (GTK_TABLE (table), scale, 2, FORMAT7_TABLE_WIDTH-1, 2, 3, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (scale), 0);

    spin = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)), 1, 0);
    gtk_widget_ref (spin);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_hpos_spin", spin, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin);
    gtk_table_attach (GTK_TABLE (table), spin, FORMAT7_TABLE_WIDTH-1, FORMAT7_TABLE_WIDTH, 2, 3, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin, TRUE);

    // vertical size
    label = gtk_label_new (_("Vertical Size"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_vsize", label, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 2, 3, 4, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vsize_scale", scale, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (GTK_TABLE (table), scale, 2, FORMAT7_TABLE_WIDTH-1, 3, 4, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (scale), 0);

    spin = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)), 1, 0);
    gtk_widget_ref (spin);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vsize_spin", spin, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin);
    gtk_table_attach (GTK_TABLE (table), spin, FORMAT7_TABLE_WIDTH-1, FORMAT7_TABLE_WIDTH, 3, 4, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin, TRUE);

    // vertical position
    label = gtk_label_new (_("Vertical Position"));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_vpos", label, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 2, 4, 5, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);

    scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 1, 255, 10, 0, 0)));
    gtk_widget_ref (scale);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vpos_scale", scale, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (GTK_TABLE (table), scale, 2, FORMAT7_TABLE_WIDTH-1, 4, 5, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (scale), 0);

    spin = gtk_spin_button_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)), 1, 0);
    gtk_widget_ref (spin);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_vpos_spin", spin, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin);
    gtk_table_attach (GTK_TABLE (table), spin, FORMAT7_TABLE_WIDTH-1, FORMAT7_TABLE_WIDTH, 4, 5, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin, TRUE);

    // information frame
    frame = gtk_frame_new (NULL);
    gtk_widget_ref (frame);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_frame_info_frame", frame,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (frame);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    
    frame_title = gtk_label_new (_("<b>Frame info [bytes]</b>"));
    gtk_widget_show (frame_title);
    gtk_frame_set_label_widget (GTK_FRAME (frame), frame_title);
    gtk_label_set_use_markup (GTK_LABEL (frame_title), TRUE);

    table = gtk_table_new (2, 4, TRUE);
    gtk_widget_ref (table);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "table15", table,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (frame), table);
    
    label = gtk_label_new (_("Image pixels :  "));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label14", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    label = gtk_label_new (_("Image size :  "));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label15", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    bar = gtk_statusbar_new ();
    gtk_widget_ref (bar);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_imagebytes", bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (bar), FALSE);
    gtk_widget_show (bar);
    gtk_table_attach (GTK_TABLE (table), bar, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 2, 2);
    
    bar = gtk_statusbar_new ();
    gtk_widget_ref (bar);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_imagepixels", bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (bar), FALSE);
    gtk_widget_show (bar);
    gtk_table_attach (GTK_TABLE (table), bar, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 2, 2);

    label = gtk_label_new (_("Padding :  "));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label159a", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    label = gtk_label_new (_("Total size:  "));
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label160a", label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_padding (GTK_MISC (label), 2, 2);
    
    bar = gtk_statusbar_new ();
    gtk_widget_ref (bar);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_padding", bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (bar), FALSE);
    gtk_widget_show (bar);
    gtk_table_attach (GTK_TABLE (table), bar, 3, 4, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    
    bar = gtk_statusbar_new ();
    gtk_widget_ref (bar);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "format7_totalbytes", bar,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (bar), FALSE);
    gtk_widget_show (bar);
    gtk_table_attach (GTK_TABLE (table), bar, 3, 4, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    
    //eprint("check 7\n");

    // now build the ranges and menus for this frame:
    //eprint("check 8\n");
    BuildFormat7ModeFrame();
    //eprint("check 9\n");
    BuildFormat7Ranges();
    //eprint("check 10\n");
    format7_tab_presence=1;
  }
  //eprint("finished building F7 window\n");

}

void
BuildFeatureWindow(void)
{
  GtkWidget* vbox_features, *table;
  int i;
  // destroy previous feature vbox
  gtk_widget_destroy(lookup_widget(main_window,"vbox_features"));

  // build new feature vbox
  vbox_features = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox_features);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "vbox_features", vbox_features,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox_features);
  gtk_container_add (GTK_CONTAINER (lookup_widget(main_window,"viewport1")), vbox_features);

  table = gtk_table_new (10, 6, FALSE);
  //gtk_table_set_homogeneous (GTK_TABLE(table),TRUE);
  gtk_table_set_col_spacings(GTK_TABLE (table),5);
  gtk_container_set_border_width(GTK_CONTAINER (table),5);
  gtk_widget_ref (table);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "feature_table", table, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX(vbox_features), table, FALSE, TRUE, 0);

  int pos=0;
  for (i=DC1394_FEATURE_MIN;i<=DC1394_FEATURE_MAX;i++) {
    if ((camera->feature_set.feature[i-DC1394_FEATURE_MIN].available>0)&&
	(i!=DC1394_FEATURE_TRIGGER)) {
      BuildRange(i,&pos);
    }
  }
}

void
BuildMainWindow(void)
{ 
  //eprint("testin\n");

  gtk_menu_item_right_justify(GTK_MENU_ITEM (lookup_widget(main_window,"help")));
  BuildPowerFrame();
  //eprint("testoutdd\n");
  BuildServiceFrame();
  //eprint("testoutddd\n");
  BuildTriggerFrame();
  //eprint("testout\n");
  BuildIsoFrame();
  //eprint("testoutg\n");
  BuildGlobalIsoFrame();
  //eprint("testout\n");
  BuildCameraFrame();
  //eprint("testoutw\n");
  BuildMemoryFrame();
  //eprint("testout\n");
  BuildFormatMenu();
  BuildOptionFrame();

  //eprint("testoute\n");

}

void
BuildStatusWindow(void)
{
  BuildCameraStatusFrame();
  BuildTransferStatusFrame();
  BuildBandwidthFrame();
  BuildSeviceTreeFrame();
  BuildRegisterAccessFrame();
}

void
BuildAllWindows(void)
{
  //eprint("building windows:\n");
  BuildPreferencesWindow();
  //eprint("  preferences\n");
  BuildMainWindow();
  //eprint("  main\n");
  BuildFeatureWindow();
  //eprint("  features\n");
  BuildFormat7Window();
  //eprint("  F7\n");
  BuildStatusWindow();
  //eprint("  status\n");
}

void
BuildHelpWindow(void)
{
  int i;
  GtkTreeViewColumn *col;
  GtkCellRenderer   *renderer;
  GtkTreeView       *tree_view;
  GtkTreeStore      *treestore;
  GtkTreeIter       toplevel;
  GtkTreeModel      *model;

  //fprintf(stderr,"Start building tree...");
  tree_view=(GtkTreeView*)lookup_widget(help_window,"key_bindings");

  // --- First column ---
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Key");

  // pack tree view column into tree view
  gtk_tree_view_append_column(tree_view, col);
  renderer = gtk_cell_renderer_text_new();

  // pack cell renderer into tree view column
  gtk_tree_view_column_pack_start(col, renderer, TRUE);

  // connect 'text' property of the cell renderer to model column that contains the camera
  gtk_tree_view_column_add_attribute(col, renderer, "text", 0);

  // --- Second column ---
  col = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(col, "Function");

  // pack tree view column into tree view
  gtk_tree_view_append_column(tree_view, col);
  renderer = gtk_cell_renderer_text_new();

  // pack cell renderer into tree view column
  gtk_tree_view_column_pack_start(col, renderer, TRUE);

  // connect 'text' property of the cell renderer to model column that contains the camera
  gtk_tree_view_column_add_attribute(col, renderer, "text", 1);

  // --- create model and fill with camera names ---
  treestore = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING); 

  // Append help lines:
  for (i=0;i<KEY_BINDINGS_NUM;i++) {
    gtk_tree_store_append(treestore, &toplevel, NULL);
    gtk_tree_store_set(treestore, &toplevel, 0, help_key_bindings_keys[i], 1, help_key_bindings_functions[i], -1);
  }

  model = GTK_TREE_MODEL(treestore);

  gtk_tree_view_set_model(tree_view, model);

  g_object_unref(model); // destroy model automatically with view
  
  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(tree_view), GTK_SELECTION_NONE);

}
