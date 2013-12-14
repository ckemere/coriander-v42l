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

#define MAINTHREAD_PLEASE_DO(func, arg...) \
{ \
    /* ask the main thread to do the job */ \
    /*fprintf (stderr,"%s: Asking mainthread to do the job\n", __FUNCTION__);*/ \
    /* only one request at a time */ \
    /*fprintf(stderr,"macro trying to lock\n");*/ \
    pthread_mutex_lock (&mainthread_info.do_mutex); \
    /* first the arg */ \
    mainthread_info.do_arg=arg; \
    /* then the function, this signals the mainthread in the timeout loop */ \
    mainthread_info.do_function=(void *(*)(void *))func; \
    /* the main thread signals when done */ \
    while (mainthread_info.do_function!=NULL) { \
      /*fprintf(stderr,".");*/ \
      usleep(DELAY); \
    } \
    /*fprintf (stderr,"%s: Thank you mainthread!\n", __FUNCTION__);*/ \
    pthread_mutex_unlock (&mainthread_info.do_mutex); \
    /*return (int)mainthread_info.do_result; \ */ \
}

void
ErrorPopup(char * string, int is_error)
{
  GtkWidget *error_box;
  GtkWidget *dialog_vbox;
  GtkWidget *dialog_action_area;
  GtkWidget *error_popup_button;

  //fprintf(stderr,"error popup %d\n",is_error);

  if (is_error>0) {
    error_box = gnome_message_box_new (_(string), GNOME_MESSAGE_BOX_ERROR, NULL);
    gtk_window_set_title (GTK_WINDOW (error_box), _("Coriander error"));
  }
  else {
    error_box = gnome_message_box_new (_(string), GNOME_MESSAGE_BOX_WARNING, NULL);
    gtk_window_set_title (GTK_WINDOW (error_box), _("Coriander warning"));
  }

  gtk_widget_set_name (error_box, "error_box");
  gtk_window_set_resizable (GTK_WINDOW (error_box), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (error_box), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox = GNOME_DIALOG (error_box)->vbox;
  gtk_widget_set_name (dialog_vbox, "dialog_vbox");
  gtk_widget_show (dialog_vbox);

  dialog_action_area = GNOME_DIALOG (error_box)->action_area;
  gtk_widget_set_name (dialog_action_area, "dialog_action_area");
  gtk_widget_show (dialog_action_area);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);

  gnome_dialog_append_button (GNOME_DIALOG (error_box), "gtk-ok");
  error_popup_button = GTK_WIDGET (g_list_last (GNOME_DIALOG (error_box)->buttons)->data);
  gtk_widget_set_name (error_popup_button, "error_popup_button");
  gtk_widget_show (error_popup_button);
  GTK_WIDGET_SET_FLAGS (error_popup_button, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) error_popup_button, "clicked",
                    G_CALLBACK (on_error_popup_button_activate), NULL);

  //fprintf(stderr,"Dialog built\n");

  gtk_widget_set_sensitive(main_window,0);

  //fprintf(stderr,"Sensitivity cancelled\n");

  gtk_widget_show(error_box);

  //fprintf(stderr,"Dialog shown\n");

  //gtk_widget_destroy(error_box);

}

inline void
Error(char *string)
{
 
  if (preferences.error_in_popup>0) {
    //fprintf(stderr,"err trying to lock\n");
    pthread_mutex_lock(&mainthread_info.dialog_mutex);
    mainthread_info.dialog_clicked=0;
    if (pthread_self()!=mainthread_info.thread) {
      // this macro returns with the result of the call
      strcpy(mainthread_info.message,string);
      MAINTHREAD_PLEASE_DO(ErrorPopup,mainthread_info.message,1);
      //MAINTHREAD_PLEASE_DO(fprintf,stderr,"test_error");
    }
    else {
      //fprintf(stderr,"Direct call!\n");
      ErrorPopup(string,1);
    }
    if (pthread_self()!=mainthread_info.thread) {
      while (mainthread_info.dialog_clicked==0) {
	//fprintf(stderr,":");
	usleep(DELAY);
      }
    }
    mainthread_info.dialog_clicked=0;
    pthread_mutex_unlock(&mainthread_info.dialog_mutex);
  
  }
  else
      fprintf(stderr,"ERROR: %s\n",string);
 
}

void
Warning(char *string)
{
  
  if (preferences.error_in_popup>0) {
    pthread_mutex_lock(&mainthread_info.dialog_mutex);
    mainthread_info.dialog_clicked=0;
    //fprintf(stderr,"Self: %x  \t  Main: %x\n",pthread_self(),mainthread_info.thread);
    if (pthread_self()!=mainthread_info.thread) {
      // this macro returns with the result of the call
      strcpy(mainthread_info.message,string);
      MAINTHREAD_PLEASE_DO(ErrorPopup,mainthread_info.message,0);
    }
    else {
      //fprintf(stderr,"Do popup myself\n");
      ErrorPopup(string,0);
    }
    
    if (pthread_self()!=mainthread_info.thread) {
      while (mainthread_info.dialog_clicked==0) {
	usleep(DELAY);
      }
    }
    mainthread_info.dialog_clicked=0;
    pthread_mutex_unlock(&mainthread_info.dialog_mutex);
  
  }
  else
      fprintf(stderr,"WARNING: %s\n",string);
  
}

