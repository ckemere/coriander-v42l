/*
 *                     C  O  R  I  A  N  D  E  R
 *
 *            -- The IEEE-1394 Digital Camera controller --
 *
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *               Ftp and conversions by Dan Dennedy <ddennedy@coolsite.net>
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

int
main (int argc, char *argv[])
{
  int err;
  int main_timeout;
  GtkWidget* err_window;
  raw1394handle_t tmp_handle;

  main_timeout_ticker=0;
  WM_cancel_display=0;
  cameras=NULL;
  silent_ui_update=0;

  
  // only the main thread is allowed to use gtk_ functions
  memset (&mainthread_info, 0, sizeof(mainthread_info));
  mainthread_info.thread=pthread_self();
  pthread_mutex_init(&mainthread_info.do_mutex, NULL);
  pthread_mutex_init(&mainthread_info.dialog_mutex, NULL);
  

  /*
  pthread_mutex_init(&message_mutex,NULL);
  message_request=0;
  message_clicked=0;
  */
#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif
  gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
                      argc, argv,
                      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
                      NULL);

  GetXvInfo(&xvinfo);
  //eprint("%d %d\n", xvinfo.max_height, xvinfo.max_width);
  LoadConfigFile();
  //eprint("config loaded\n");

  //  port_num should be set here or later below
  tmp_handle=raw1394_new_handle();
  if (tmp_handle==NULL) {
    err_window=create_no_handle_window();
    gtk_widget_show(err_window);
    gtk_main();
    return(1);
  }
  port_num=raw1394_get_port_info(tmp_handle, NULL, 0);
  raw1394_destroy_handle(tmp_handle);
  //eprint("port OK\n");

  dc1394=dc1394_new();

  err=GetCameraNodes();
  //eprint("nodes OK\n");

  if (cameras==NULL) {
    err_window=create_no_camera_window();
    gtk_widget_show(err_window);
    gtk_main();
    return(1);
  }
  else if (err!=DC1394_SUCCESS) {
    fprintf(stderr, "Unknown error getting cameras on the bus.\nExiting\n");
    exit(1);
  }

  /*
  camera_t *cam_tmp=cameras;
  while (cam_tmp!=NULL) {
    dc1394_cleanup_iso_channels_and_bandwidth(cam_tmp->camera_info);
    cam_tmp=cam_tmp->next;
  }
  */
  //eprint("Starting to set ISO channels\n");
  //SetIsoChannels();
  //eprint("ISO channels are set\n");

  // current camera is the first camera:
  SetCurrentCamera(cameras->camera_info->guid);
  //eprint("current camera set\n");
  
  preferences_window= create_preferences_window();

  //eprint("preferences window created\n");
  
  main_window = create_main_window();
  //eprint("main window created\n");
  
  
  format7_tab_presence=1;
  gtk_notebook_set_homogeneous_tabs(GTK_NOTEBOOK(lookup_widget(main_window,"notebook2")),TRUE);
  gtk_notebook_set_homogeneous_tabs(GTK_NOTEBOOK(lookup_widget(main_window,"notebook5")),TRUE);

  // Setup the GUI in accordance with the camera capabilities
  GetContextStatus();
  //eprint("got context\n");
  BuildAllWindows();
  //eprint("windows built\n");
  UpdateAllWindows();
  //eprint("windows updated\n");

  int port;
  handles=(raw1394handle_t*)malloc(port_num*sizeof(raw1394handle_t));
  // Set bus reset handlers:
  for (port=0;port<port_num;port++) {
    // get a handle to the current interface card
    handles[port]=raw1394_new_handle();
    raw1394_set_port(handles[port],port);
    // set bus reset handler
    raw1394_set_bus_reset_handler(handles[port], bus_reset_handler);
    //raw1394_destroy_handle(handle);
  }

  //Warning("Welcome to Coriander...");
  gtk_widget_show (main_window); // this is the only window shown at boot-time
  
  main_timeout=gtk_timeout_add(10, (GtkFunction)main_timeout_handler, (gpointer)(unsigned long)port_num);

#ifdef HAVE_SDLLIB
  WatchStartThread(&watchthread_info);
#endif

  gtk_main();

  gtk_timeout_remove(main_timeout);
  
#ifdef HAVE_SDLLIB
  WatchStopThread(&watchthread_info);
#endif

  while (cameras!=NULL) {
    RemoveCamera(camera->camera_info->guid);
  }

  free(handles);
  
  dc1394_free(dc1394);

  return 0;
}
