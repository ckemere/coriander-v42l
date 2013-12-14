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

#define RANGE_TABLE_WIDTH 6

#define LABEL_SIZE  1
#define MENU_SIZE   1
#define ELABEL_SIZE 1
#define SPIN_SIZE   1

#define LM LABEL_SIZE
#define MR (LABEL_SIZE+MENU_SIZE) 
#define ER (LABEL_SIZE+MENU_SIZE+ELABEL_SIZE)
#define RS (RANGE_TABLE_WIDTH-SPIN_SIZE)

void
BuildEmptyRange(int feature, int pos)
{
    GtkWidget *table, *label1, *label2, *label3;
  char *stemp;

  stemp=(char*)malloc(STRING_SIZE*sizeof(char));
  table=lookup_widget(main_window, "feature_table");

  switch (feature) {
  case DC1394_FEATURE_TEMPERATURE:

    label1 = gtk_label_new (_("Current"));
    gtk_widget_ref (label1);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_temp_scale_current", label1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1);
    gtk_table_attach (GTK_TABLE (table), label1, MR, ER, pos, pos+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label1), 2, 2);
    
    label2 = gtk_label_new (_("Target"));
    gtk_widget_ref (label2);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_temp_scale_target", label2, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label2);
    gtk_table_attach (GTK_TABLE (table), label2, MR, ER, pos+1, pos+2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label2), 2, 2);

    break;
  case DC1394_FEATURE_WHITE_BALANCE:

    label1 = gtk_label_new (_("Blue/U"));
    gtk_widget_ref (label1);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_wb_scale_bu", label1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1);
    gtk_table_attach (GTK_TABLE (table), label1, MR, ER, pos, pos+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label1), 2, 2);

    label2 = gtk_label_new (_("Red/V"));
    gtk_widget_ref (label2);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_wb_scale_rv", label2, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label2);
    gtk_table_attach (GTK_TABLE (table), label2, MR, ER, pos+1, pos+2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label2), 2, 2);

    break;
  case DC1394_FEATURE_WHITE_SHADING:

    label1 = gtk_label_new (_("Red"));
    gtk_widget_ref (label1);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_ws_scale_r", label1, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label1);
    gtk_table_attach (GTK_TABLE (table), label1, MR, ER, pos, pos+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label1), 2, 2);

    label2 = gtk_label_new (_("Green"));
    gtk_widget_ref (label2);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_ws_scale_g", label2, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label2);
    gtk_table_attach (GTK_TABLE (table), label2, MR, ER, pos+1, pos+2, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label2), 2, 2);

    label3 = gtk_label_new (_("Blue"));
    gtk_widget_ref (label3);
    gtk_object_set_data_full (GTK_OBJECT (main_window), "label_ws_scale_b", label3, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label3);
    gtk_table_attach (GTK_TABLE (table), label3, MR, ER, pos+2, pos+3, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_RIGHT);
    gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label3), 2, 2);
    break;

  default:

    break;
  }

  // label
  sprintf(stemp,"<b>%s</b>",feature_name_list[feature-DC1394_FEATURE_MIN]);
  label1 = gtk_label_new (_(stemp));
  gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);
  gtk_widget_ref (label1);
  sprintf(stemp,"feature_%d_label",feature);
  gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, label1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE(table), label1, 0, LM, pos, pos+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

  free(stemp);
}

void
BuildRange(int feature, int *pos)
{
  GtkAdjustment *adjustment, *adjustment2, *adjustment3;
  GtkWidget* new_option_menu;
  GtkWidget* new_menu;
  GtkWidget* glade_menuitem;
  GtkWidget* scale, *scale2, *scale3;
  GtkWidget* spin, *spin2, *spin3;
  GtkWidget* abs_entry;
  GtkWidget* label;
  GtkTable* table;
  int nlines=0;

  char *stemp;
  stemp=(char*)malloc(STRING_SIZE*sizeof(char));

  BuildEmptyRange(feature,*pos);

  table=GTK_TABLE (lookup_widget(main_window, "feature_table"));

  // BUILD A NEW OPTION_MENU:

  sprintf(stemp,"feature_%d_menu",feature);
  
  new_option_menu = gtk_option_menu_new ();
  gtk_widget_ref (new_option_menu);
  gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, new_option_menu, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (new_option_menu);
  gtk_table_attach (table, new_option_menu, LM, MR, *pos, *pos+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (new_option_menu), 1);
  
  new_menu = gtk_menu_new ();

  int auto_capable=0;
  int manual_capable=0;
  int one_push_capable=0;
  int i;
  for (i=0;i<camera->feature_set.feature[feature-DC1394_FEATURE_MIN].modes.num;i++) {
    switch (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].modes.modes[i]) {
    case DC1394_FEATURE_MODE_MANUAL:
      manual_capable=1;
      break;
    case DC1394_FEATURE_MODE_AUTO:
      auto_capable=1;
      break;
    case DC1394_FEATURE_MODE_ONE_PUSH_AUTO:
      one_push_capable=1;
      break;
    }
  }
      
  int no_control= ( ( camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable   == DC1394_FALSE ) && // disable feature if there is no way to control it
		    ( camera->feature_set.feature[feature-DC1394_FEATURE_MIN].modes.num        == 0            ) );

  if (no_control==1) { // set menu to N/A
      glade_menuitem = gtk_menu_item_new_with_label (_("N/A"));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
  }
  else {
  // BUILD MENU ITEMS ====================================================================================
  // 'off' menuitem optional addition:
  if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].on_off_capable>0) {
      glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_OFF]));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
      g_signal_connect ((gpointer) glade_menuitem, "activate",
			G_CALLBACK (on_range_menu_activate),
			(gpointer)(unsigned long)(feature*1000+RANGE_MENU_OFF)); // i is an int passed in a pointer variable. This is 'normal'.
  }
  // 'man' menuitem optional addition:
  if (manual_capable>0) {
      glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_MAN]));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
      g_signal_connect ((gpointer) glade_menuitem, "activate",
			G_CALLBACK (on_range_menu_activate),
			(gpointer)(unsigned long)(feature*1000+RANGE_MENU_MAN));
  }
  // 'auto' menuitem optional addition:
  if (auto_capable>0) {
      glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_AUTO]));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
      g_signal_connect ((gpointer) glade_menuitem, "activate",
			G_CALLBACK (on_range_menu_activate),
			(gpointer)(unsigned long)(feature*1000+RANGE_MENU_AUTO));
  }
  // 'single' menuitem optional addition:
  if (one_push_capable>0) {
      glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_SINGLE]));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
      g_signal_connect ((gpointer) glade_menuitem, "activate",
			G_CALLBACK (on_range_menu_activate),
			(gpointer)(unsigned long)(feature*1000+RANGE_MENU_SINGLE));
  }
  // 'absolute' menuitem optional addition:
  if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable>0) {
      glade_menuitem = gtk_menu_item_new_with_label (_(feature_menu_items_list[RANGE_MENU_ABSOLUTE]));
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (new_menu), glade_menuitem);
      g_signal_connect ((gpointer) glade_menuitem, "activate",
			G_CALLBACK (on_range_menu_activate),
			(gpointer)(unsigned long)(feature*1000+RANGE_MENU_ABSOLUTE));
  }
  
  }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_option_menu), new_menu);

  // BUILD SCALE: ====================================================================================

  switch(feature) {
  case DC1394_FEATURE_WHITE_BALANCE:
    adjustment=(GtkAdjustment*)gtk_adjustment_new(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].max,1,10,0);
    adjustment2=(GtkAdjustment*)gtk_adjustment_new(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].max,1,10,0);
    scale = gtk_hscale_new (adjustment);
    gtk_widget_ref (scale);
    sprintf(stemp,"feature_%d_bu_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, scale, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (table, scale, ER, RS, *pos, *pos+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale, TRUE);
    gtk_scale_set_draw_value (GTK_SCALE (scale), 0);
    gtk_range_set_adjustment((GtkRange*)scale,adjustment);

    scale2 = gtk_hscale_new (adjustment2);
    gtk_widget_ref (scale2);
    sprintf(stemp,"feature_%d_rv_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, scale2, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale2);
    gtk_table_attach (table, scale2, ER, RS, *pos+1, *pos+2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale2, TRUE);
    gtk_scale_set_draw_value (GTK_SCALE (scale2), 0);
    gtk_range_set_adjustment((GtkRange*)scale2,adjustment2);

    // additional spinbuttons
    spin = gtk_spin_button_new (adjustment, 1, 0); // two lasts: step, decimal places. To be changed for absolute settings
    gtk_widget_ref (spin);
    sprintf(stemp,"feature_%d_bu_spin",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, spin, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin);
    gtk_table_attach (table, spin, RS, RANGE_TABLE_WIDTH, *pos, *pos+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin, TRUE);

    spin2 = gtk_spin_button_new (adjustment2, 1, 0); // two lasts: step, decimal places. To be changed for absolute settings
    gtk_widget_ref (spin2);
    sprintf(stemp,"feature_%d_rv_spin",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, spin2, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin2);
    gtk_table_attach (table, spin2, RS, RANGE_TABLE_WIDTH, *pos+1, *pos+2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin2, TRUE);

    // connect:
    g_signal_connect ((gpointer) adjustment, "value_changed", G_CALLBACK (on_scale_value_changed), (gpointer)(unsigned long) DC1394_FEATURE_WHITE_BALANCE+BU);
    g_signal_connect ((gpointer) (adjustment2), "value_changed", G_CALLBACK (on_scale_value_changed), (gpointer)(unsigned long) DC1394_FEATURE_WHITE_BALANCE+RV);
    nlines=2;
    break;
  case DC1394_FEATURE_TEMPERATURE:
    adjustment=(GtkAdjustment*)gtk_adjustment_new(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].max,1,10,0);
    scale = gtk_hscale_new (adjustment);
    gtk_widget_ref (scale);
    sprintf(stemp,"feature_%d_current_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, scale, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (table, scale, ER, RS, *pos, *pos+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale, FALSE);
    gtk_scale_set_draw_value (GTK_SCALE (scale), 0);
    gtk_range_set_adjustment((GtkRange*)scale,adjustment);

    adjustment2=(GtkAdjustment*)gtk_adjustment_new(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].max,1,10,0);
    scale2 = gtk_hscale_new (adjustment2);
    gtk_widget_ref (scale2);
    sprintf(stemp,"feature_%d_target_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, scale2,  (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale2);
    gtk_table_attach (table, scale2, ER, RS, *pos+1, *pos+2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),(GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale2, TRUE);
    gtk_scale_set_draw_value (GTK_SCALE (scale2), 0);
    gtk_range_set_adjustment((GtkRange*)scale2,adjustment2);

    // additional spinbuttons
    spin = gtk_spin_button_new (adjustment, 1, 0); // two lasts: step, decimal places. To be changed for absolute settings
    gtk_widget_ref (spin);
    sprintf(stemp,"feature_%d_current_spin",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, spin, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin);
    gtk_table_attach (table, spin, RS, RANGE_TABLE_WIDTH, *pos, *pos+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin, FALSE);

    spin2 = gtk_spin_button_new (adjustment2, 1, 0); // two lasts: step, decimal places. To be changed for absolute settings
    gtk_widget_ref (spin2);
    sprintf(stemp,"feature_%d_target_spin",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, spin2, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin2);
    gtk_table_attach (table, spin2, RS, RANGE_TABLE_WIDTH, *pos+1, *pos+2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin2, TRUE);

    // connect:
    g_signal_connect ((gpointer) adjustment, "value_changed", G_CALLBACK (on_scale_value_changed), (gpointer)(unsigned long) DC1394_FEATURE_TEMPERATURE);
    nlines=2;
    break;
  case DC1394_FEATURE_WHITE_SHADING:
    adjustment=(GtkAdjustment*)gtk_adjustment_new(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].max,1,10,0);
    adjustment2=(GtkAdjustment*)gtk_adjustment_new(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].max,1,10,0);
    adjustment3=(GtkAdjustment*)gtk_adjustment_new(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						   camera->feature_set.feature[feature-DC1394_FEATURE_MIN].max,1,10,0);
    scale = gtk_hscale_new (adjustment);
    gtk_widget_ref (scale);
    sprintf(stemp,"feature_%d_r_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, scale, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (table, scale, ER, RS, *pos, *pos+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale, TRUE);
    gtk_scale_set_draw_value (GTK_SCALE (scale), 0);
    gtk_range_set_adjustment((GtkRange*)scale,adjustment);

    scale2 = gtk_hscale_new (adjustment2);
    gtk_widget_ref (scale2);
    sprintf(stemp,"feature_%d_g_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, scale2, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale2);
    gtk_table_attach (table, scale2, ER, RS, *pos+1, *pos+2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale2, TRUE);
    gtk_scale_set_draw_value (GTK_SCALE (scale2), 0);
    gtk_range_set_adjustment((GtkRange*)scale2,adjustment2);

    scale3 = gtk_hscale_new (adjustment3);
    gtk_widget_ref (scale3);
    sprintf(stemp,"feature_%d_b_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, scale3, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale3);
    gtk_table_attach (table, scale3, ER, RS, *pos+2, *pos+3, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale3, TRUE);
    gtk_scale_set_draw_value (GTK_SCALE (scale3), 0);
    gtk_range_set_adjustment((GtkRange*)scale3,adjustment3);

    // additional spinbuttons
    spin = gtk_spin_button_new (adjustment, 1, 0); // two lasts: step, decimal places. To be changed for absolute settings
    gtk_widget_ref (spin);
    sprintf(stemp,"feature_%d_r_spin",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, spin, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin);
    gtk_table_attach (table, spin, RS, RANGE_TABLE_WIDTH, *pos, *pos+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin, TRUE);

    spin2 = gtk_spin_button_new (adjustment2, 1, 0); // two lasts: step, decimal places. To be changed for absolute settings
    gtk_widget_ref (spin2);
    sprintf(stemp,"feature_%d_g_spin",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, spin2, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin2);
    gtk_table_attach (table, spin2, RS, RANGE_TABLE_WIDTH, *pos+1, *pos+2, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin2, TRUE);

    spin3 = gtk_spin_button_new (adjustment3, 1, 0); // two lasts: step, decimal places. To be changed for absolute settings
    gtk_widget_ref (spin3);
    sprintf(stemp,"feature_%d_b_spin",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, spin3, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin3);
    gtk_table_attach (table, spin3, RS, RANGE_TABLE_WIDTH, *pos+2, *pos+3, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (spin3, TRUE);

    // connect:
    g_signal_connect ((gpointer) adjustment, "value_changed", G_CALLBACK (on_scale_value_changed), (gpointer)(unsigned long) DC1394_FEATURE_WHITE_SHADING+SHADINGR);
    g_signal_connect ((gpointer) (adjustment2), "value_changed", G_CALLBACK (on_scale_value_changed), (gpointer)(unsigned long) DC1394_FEATURE_WHITE_BALANCE+SHADINGG);
    g_signal_connect ((gpointer) (adjustment3), "value_changed", G_CALLBACK (on_scale_value_changed), (gpointer)(unsigned long) DC1394_FEATURE_WHITE_BALANCE+SHADINGB);
    nlines=3;
    break;
    
  default:
    // scale
    adjustment=(GtkAdjustment*)gtk_adjustment_new(camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].min,
						  camera->feature_set.feature[feature-DC1394_FEATURE_MIN].max,1,10,0);
    scale = gtk_hscale_new (adjustment);
    gtk_widget_ref (scale);
    sprintf(stemp,"feature_%d_scale",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, scale, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (scale);
    gtk_table_attach (table, scale, MR, RS, *pos, *pos+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_widget_set_sensitive (scale, TRUE);
    gtk_scale_set_draw_value (GTK_SCALE (scale), 0);
    gtk_range_set_adjustment((GtkRange*)scale,adjustment);

    // additional spinbutton
    spin = gtk_spin_button_new (adjustment, 1, 0); // two lasts: step, decimal places. To be changed for absolute settings
    gtk_widget_ref (spin);
    sprintf(stemp,"feature_%d_spin",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, spin, (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (spin);
    sprintf(stemp,"feature_%d_table",feature);

    gtk_table_attach (table, spin, RS, RANGE_TABLE_WIDTH, *pos, *pos+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (GTK_FILL), 0, 0);

    gtk_widget_set_sensitive (spin, TRUE);

    // connect:
    g_signal_connect ((gpointer) adjustment, "value_changed", G_CALLBACK (on_scale_value_changed), (gpointer)(unsigned long) feature);
    nlines=1;
    break;
    
  }

  // common action for absolute settings:
  if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable!=0) {
    // entry
    abs_entry = gtk_entry_new ();
    gtk_widget_ref (abs_entry);
    sprintf(stemp,"feature_%d_abs_entry",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, abs_entry,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (abs_entry);
    gtk_table_attach (table, abs_entry, RS-2, RS-1, *pos+nlines, *pos+nlines+1,
		      (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    g_signal_connect ((gpointer) abs_entry, "activate",
                      G_CALLBACK (on_abs_entry_activate),
                      (gpointer)(unsigned long)feature);
    // label
    label = gtk_label_new (_(feature_abs_label_list[feature-DC1394_FEATURE_MIN]));
    gtk_widget_ref (label);
    sprintf(stemp,"feature_%d_abs_label",feature);
    gtk_object_set_data_full (GTK_OBJECT (main_window), stemp, label,
			      (GtkDestroyNotify) gtk_widget_unref);
    gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
    gtk_widget_show (label);
    gtk_table_attach (table, label, RS-1, RS, *pos+nlines, *pos+nlines+1,
		      (GtkAttachOptions) (GTK_FILL|GTK_FILL),
		      (GtkAttachOptions) (0), 10, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  }

  free(stemp);

  switch(feature) {
  case DC1394_FEATURE_WHITE_BALANCE:
  case DC1394_FEATURE_TEMPERATURE:
      *pos+=2;
      break;
  case DC1394_FEATURE_WHITE_SHADING:
      *pos+=3;
      break;
  default:
      *pos+=1;
      break;
  }

  if (camera->feature_set.feature[feature-DC1394_FEATURE_MIN].absolute_capable==DC1394_TRUE)
      *pos+=1;
}

void
BuildFormat7Ranges(void)
{
  
  GtkAdjustment  *adjustment_px, *adjustment_py, *adjustment_sx, *adjustment_sy, *adjustment_packet;
  dc1394format7mode_t *info;
  
  info=&camera->format7_info.modeset.mode[camera->format7_info.edit_mode-DC1394_VIDEO_MODE_FORMAT7_MIN];

  if (dc1394_format7_get_mode_info(camera->camera_info, camera->format7_info.edit_mode, info)!=DC1394_SUCCESS)
      Error("Could not get format7 mode information");

  // define the adjustments for the 5 format7 controls. Note that (pos_x+size_x)<=max_size_x which yields some inter-dependencies
  // define adjustement for X-position
  adjustment_px=(GtkAdjustment*)gtk_adjustment_new(info->pos_x,0,info->max_size_x-info->size_x,info->unit_pos_x,info->unit_pos_x*4,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(main_window, "format7_hpos_scale"),adjustment_px);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(lookup_widget(main_window, "format7_hpos_spin")),adjustment_px, info->unit_pos_x, 0);
  camera->format7_info.scale_posx_handle=g_signal_connect((gpointer) adjustment_px, "value_changed", G_CALLBACK (on_format7_value_changed), (int*) FORMAT7_POS_X);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(main_window, "format7_hpos_scale"), GTK_UPDATE_DELAYED);
  gtk_adjustment_changed (adjustment_px);
  
  // define adjustement for Y-position 
  adjustment_py=(GtkAdjustment*)gtk_adjustment_new(info->pos_y,0,info->max_size_y-info->size_y,info->unit_pos_y,info->unit_pos_y*4,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(main_window, "format7_vpos_scale"),adjustment_py);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(lookup_widget(main_window, "format7_vpos_spin")),adjustment_py, info->unit_pos_y, 0);
  camera->format7_info.scale_posy_handle=g_signal_connect((gpointer) adjustment_py, "value_changed", G_CALLBACK (on_format7_value_changed), (int*) FORMAT7_POS_Y);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(main_window, "format7_vpos_scale"), GTK_UPDATE_DELAYED);
  gtk_adjustment_changed (adjustment_py);

  // define adjustement for X-size
  adjustment_sx=(GtkAdjustment*)gtk_adjustment_new(info->size_x,info->unit_size_x,info->max_size_x-info->pos_x,info->unit_size_x,info->unit_size_x*4,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(main_window, "format7_hsize_scale"),adjustment_sx);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(lookup_widget(main_window, "format7_hsize_spin")),adjustment_sx, info->unit_size_x, 0);
  camera->format7_info.scale_sizex_handle=g_signal_connect((gpointer) adjustment_sx, "value_changed", G_CALLBACK (on_format7_value_changed), (int*) FORMAT7_SIZE_X);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(main_window, "format7_hsize_scale"), GTK_UPDATE_DELAYED);
  gtk_adjustment_changed (adjustment_sx);

  // define adjustement for Y-size
  adjustment_sy=(GtkAdjustment*)gtk_adjustment_new(info->size_y,info->unit_size_y,info->max_size_y-info->pos_y,info->unit_size_y,info->unit_size_y*4,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(main_window, "format7_vsize_scale"),adjustment_sy);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(lookup_widget(main_window, "format7_vsize_spin")),adjustment_sy, info->unit_size_y, 0);
  camera->format7_info.scale_sizey_handle=g_signal_connect((gpointer) adjustment_sy, "value_changed", G_CALLBACK (on_format7_value_changed), (int*) FORMAT7_SIZE_Y);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(main_window, "format7_vsize_scale"), GTK_UPDATE_DELAYED);
  gtk_adjustment_changed (adjustment_sy);

  // define adjustment for packet size:
  adjustment_packet=(GtkAdjustment*)gtk_adjustment_new(info->packet_size,info->unit_packet_size,info->max_packet_size,info->unit_packet_size,(info->max_packet_size-info->unit_packet_size)/16,0);
  gtk_range_set_adjustment((GtkRange*)lookup_widget(main_window, "format7_psize"),adjustment_packet);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(lookup_widget(main_window, "format7_psize_spin")),adjustment_packet, info->unit_packet_size, 0);
  g_signal_connect((gpointer) adjustment_packet, "value_changed", G_CALLBACK (on_format7_value_changed),(int*) FORMAT7_PACKET);
  gtk_range_set_update_policy ((GtkRange*)lookup_widget(main_window, "format7_psize"), GTK_UPDATE_DELAYED);
  gtk_adjustment_changed (adjustment_packet);

}
