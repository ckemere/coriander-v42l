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

#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

gboolean
on_main_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_file_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit_activate                       (GtkMenuItem     *menuitem, 
                                        gpointer         user_data);

void
on_help_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_about_window_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_commander_window_delete_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_fps_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_power_on_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_power_off_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_power_reset_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_trigger_polarity_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_trigger_mode_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_trigger_source_activate              (GtkMenuItem     *menuitem,
					 gpointer         user_data);

void
on_memory_channel_activate             (GtkMenuItem     *menuitem,
					gpointer         user_data);

void
on_load_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_mem_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_trigger_external_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_iso_start_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_iso_stop_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_iso_restart_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_camera_select_activate              (GtkMenuItem     *menuitem,
					gpointer         user_data);

void
on_format7_window_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_scale_value_changed                 (GtkAdjustment    *adj,
				        gpointer         user_data);

void
on_format7_value_changed               (GtkAdjustment    *adj,
				        gpointer         user_data);

void
on_edit_format7_mode_activate          (GtkMenuItem     *menuitem,
					gpointer         user_data);

void
on_edit_format7_color_activate         (GtkMenuItem     *menuitem,
					gpointer         user_data);

void
on_preferences_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_prefs_update_power_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_preferences_window_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_prefs_save_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_close_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_update_value_changed          (GtkAdjustment    *adj,
					gpointer         user_data);

void
on_prefs_timeout_value_changed         (GtkAdjustment    *adj,
					gpointer         user_data);

void
on_service_iso_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_service_display_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_service_save_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_service_ftp_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_ftp_seq_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_ftp_mode_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_range_menu_activate                 (GtkMenuItem     *menuitem,
				        gpointer         user_data);

void
on_prefs_display_keep_ratio_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_format7_packet_size_changed               (GtkAdjustment    *adj,
					      gpointer         user_data);

void
on_key_bindings_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_name_camera_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_camera_name_text_changed            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_op_timeout_scale_changed      (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_update_scale_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_display_period_changed        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_save_period_changed           (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_address_changed           (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_path_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_user_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_password_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_ftp_filename_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_op_timeout_scale_changed      (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_update_scale_changed          (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_bayer_menu_activate                 (GtkMenuItem     *menuitem,
				        gpointer         user_data);

void
on_bayer_pattern_menu_activate           (GtkMenuItem     *menuitem,
					  gpointer         user_data);

void
on_stereo_menu_activate               (GtkToggleButton *menuitem,
                                       gpointer         user_data);

void
on_trigger_count_changed               (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_mono16_bpp_changed                  (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_abs_entry_activate           (GtkEditable     *editable,
				 gpointer         user_data);


void
on_global_iso_stop_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_global_iso_restart_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_global_iso_start_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_service_v4l_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_v4l_period_changed            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_prefs_v4l_dev_name_changed      (GtkEditable     *editable,
                                    gpointer         user_data);

void
on_prefs_save_date_tag_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_save_num_tag_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_ftp_date_tag_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_ftp_num_tag_toggled           (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_ram_buffer_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_ram_buffer_size_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_malloc_test_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_dma_buffer_size_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_display_redraw_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_display_redraw_rate_changed         (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_broadcast_button_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_overlay_byte_order_YUYV_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_overlay_byte_order_UYVY_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_overlay_type_menu_activate           (GtkMenuItem     *menuitem,
					 gpointer         user_data);

void
on_overlay_pattern_menu_activate        (GtkMenuItem     *menuitem,
					 gpointer         user_data);

void
on_overlay_color_picker_color_set      (GnomeColorPicker *gnomecolorpicker,
                                        guint            arg1,
                                        guint            arg2,
                                        guint            arg3,
                                        guint            arg4,
                                        gpointer         user_data);

void
on_overlay_file_subentry_changed       (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_save_filename_subentry_changed      (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_grab_now_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_mode_menu_activate             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_save_to_dir_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_save_format_menu_activate             (GtkEditable     *editable,
					  gpointer         user_data);

void
on_save_append_menu_activate             (GtkEditable     *editable,
					  gpointer         user_data);

void
on_iso_nodrop_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_save_to_stdout_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_error_popup_button_activate         (GtkButton       *button,
                                        gpointer         user_data);

void
on_offset_menu_activate             (GtkMenuItem     *menuitem,
				     gpointer         user_data);

void
on_register_write_button_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_register_read_button_clicked        (GtkButton       *button,
                                        gpointer         user_data);

void
on_prefs_no_overwrite_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_warning_in_popup_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_error_in_popup_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_auto_receive_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_prefs_auto_iso_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_isospeed_menu_activate             (GtkMenuItem     *menuitem,
			   	       gpointer         user_data);
#endif // __CALLBACKS_H__

void
on_bmode_button_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);
