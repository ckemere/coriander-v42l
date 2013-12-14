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

gint
FtpStartThread(camera_t* cam)
{
  chain_t* ftp_service=NULL;
  ftpthread_info_t *info=NULL;

  ftp_service=GetService(cam,SERVICE_FTP);

  if (ftp_service==NULL) { // if no FTP service running...
    ftp_service=(chain_t*)malloc(sizeof(chain_t));
    ftp_service->current_buffer=NULL;
    ftp_service->next_buffer=NULL;
    ftp_service->data=(void*)malloc(sizeof(ftpthread_info_t));
    info=(ftpthread_info_t*)ftp_service->data;
    pthread_mutex_init(&ftp_service->mutex_data, NULL);
    pthread_mutex_init(&ftp_service->mutex_struct, NULL);
    pthread_mutex_init(&info->mutex_cancel, NULL);
    
    /* if you want a clean-interrupt thread:*/
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=0;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    /* setup ftp_thread: handles, ...*/
    pthread_mutex_lock(&ftp_service->mutex_data);
    //info->period=cam->prefs.ftp_period;
    //info->datenum=cam->prefs.ftp_datenum;
    //strcpy(info->address, cam->prefs.ftp_address);
    //strcpy(info->user, cam->prefs.ftp_user);
    //strcpy(info->password, cam->prefs.ftp_password);
    //strcpy(info->path, cam->prefs.ftp_path);
    //strcpy(info->filename, cam->prefs.ftp_filename);
    //tmp = strrchr(cam->prefs.ftp_filename, '.');
    /*
    if (tmp==NULL) {
      Error("You should supply an extension");
      pthread_mutex_unlock(&ftp_service->mutex_data);
      FreeChain(ftp_service);
      return(-1);
    }
    */
    //tmp[0] = '\0';// cut filename before point
    //strcpy(info->filename_ext, strrchr(cam->prefs.ftp_filename, '.'));
    
    CommonChainSetup(cam,ftp_service,SERVICE_FTP);
    
    info->frame.allocated_image_bytes=0;
    info->frame.image=NULL;
    
    //info->mode=cam->prefs.ftp_mode;
    
#ifdef HAVE_FTPLIB
    if (!OpenFtpConnection(ftp_service)) {
      Error("Failed to open FTP connection");
      pthread_mutex_unlock(&ftp_service->mutex_data);
      FreeChain(ftp_service);
      return(-1);
    }
#else
    Error("You don't have FTPLIB");
    pthread_mutex_unlock(&ftp_service->mutex_data);
    FreeChain(ftp_service);
    return(-1);
#endif
    
    /* Insert chain and start service*/
    pthread_mutex_lock(&ftp_service->mutex_struct);
    InsertChain(cam, ftp_service);

    if (pthread_create(&ftp_service->thread, NULL, FtpThread,(void*) ftp_service)) {
      /* error starting thread. You should cleanup here
	 (free, unset global vars,...):*/
      
      /* Mendatory cleanups:*/
      RemoveChain(cam, ftp_service);
      pthread_mutex_unlock(&ftp_service->mutex_struct);
      pthread_mutex_unlock(&ftp_service->mutex_data);
      //free(info->buffer);
      //info->buffer=NULL;
      FreeChain(ftp_service);
      ftp_service=NULL;
      return(-1);
    }

    pthread_mutex_unlock(&ftp_service->mutex_struct);
    pthread_mutex_unlock(&ftp_service->mutex_data);
    
  }
  
  return (1);
}


void*
FtpThread(void* arg)
{
  static gchar filename_out[STRING_SIZE];
  chain_t* ftp_service=NULL;
  ftpthread_info_t *info=NULL;
  //GdkImlibImage *im=NULL; // V20***
  long int skip_counter;
  float tmp;

  ftp_service=(chain_t*)arg;
  pthread_mutex_lock(&ftp_service->mutex_data);
  info=(ftpthread_info_t*)ftp_service->data;
  camera_t *cam=ftp_service->camera;
  skip_counter=(cam->prefs.ftp_period-1); /* send immediately, then start skipping */
  /* These settings depend on the thread. For 100% safe deferred-cancel
   threads, I advise you use a custom thread cancel flag. See display thread.*/
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
  pthread_mutex_unlock(&ftp_service->mutex_data);
 

  // time inits:
  ftp_service->prev_time = 0;
  ftp_service->prev_period = 0;
  ftp_service->drop_warning = 0;
  ftp_service->processed_frames=0;

  while (1) { 
    /* Clean cancel handlers */
    pthread_mutex_lock(&info->mutex_cancel);
    if (info->cancel_req>0) {
      break;
    }
    else {
      pthread_mutex_unlock(&info->mutex_cancel);
      pthread_mutex_lock(&ftp_service->mutex_data);
      if(GetBufferFromPrevious(ftp_service)) { // have buffers been rolled?
	FtpThreadCheckParams(ftp_service);
	if (ftp_service->current_buffer->frame.size[0]!=-1) {
	  if (skip_counter>=(cam->prefs.ftp_period-1)) {
	    skip_counter=0;
	    convert_to_rgb(&ftp_service->current_buffer->frame, &info->frame);
	    switch (cam->prefs.ftp_mode) {
	    case FTP_MODE_OVERWRITE:
	      sprintf(filename_out, "%s%s", cam->prefs.ftp_filename,cam->prefs.ftp_filename_ext);
	      break;
	    case FTP_MODE_SEQUENTIAL:
	      switch (cam->prefs.ftp_datenum) {
	      case FTP_TAG_DATE:
		sprintf(filename_out, "%s-%s%s", cam->prefs.ftp_filename, ftp_service->current_buffer->captime_string, cam->prefs.ftp_filename_ext);
		break;
	      case FTP_TAG_NUMBER:
		sprintf(filename_out,"%s-%10.10lli%s", cam->prefs.ftp_filename, ftp_service->processed_frames, cam->prefs.ftp_filename_ext);
		break;
	      }
	      break;
	    default:
	      break;
	    }
	    // V20***   
	    //im=gdk_imlib_create_image_from_data(info->buffer, NULL, ftp_service->current_buffer->width, ftp_service->current_buffer->height);
#ifdef HAVE_FTPLIB
	    if (!CheckFtpConnection(ftp_service)) {
	      Error("Ftp connection lost for good");
	      // AUTO CANCEL THREAD
	      pthread_mutex_lock(&info->mutex_cancel);
	      info->cancel_req=1;
	      pthread_mutex_unlock(&info->mutex_cancel);
	    }
	    else { 
	      // V20***
	      //FtpPutFrame(filename_out, im, info);
	    }
#endif
	    ftp_service->processed_frames++;

	    // V20***
	    //if (im != NULL)
	    //  gdk_imlib_kill_image(im);
	  }
	  else
	    skip_counter++;
	  
	  // FPS computation:
	  tmp=((float)(ftp_service->current_buffer->frame.timestamp-ftp_service->prev_time))/1000000.0;
	  if (ftp_service->prev_time==0) {
	    ftp_service->fps=fabs(0.0);
	  }
	  else {
	    if (tmp==0)
	      ftp_service->fps=fabs(0.0);
	    else
	      ftp_service->fps=fabs(1/tmp);
	  }
	  if (ftp_service->prev_time!=0) {
	    ftp_service->prev_period=tmp;
	  }
	  // produce a drop warning if the period difference is over 50%
	  if (ftp_service->prev_period!=0) {
	    if (fabs(ftp_service->prev_period-tmp)/(ftp_service->prev_period/2+tmp/2)>=.5)
	      ftp_service->drop_warning++;
	  }
	  ftp_service->prev_time=ftp_service->current_buffer->frame.timestamp;

	}
	
	PublishBufferForNext(ftp_service);
	pthread_mutex_unlock(&ftp_service->mutex_data);
      }
      else {
	pthread_mutex_unlock(&ftp_service->mutex_data);
      }
    }
    usleep(100);
  }

  pthread_mutex_unlock(&info->mutex_cancel);
  return ((void*)1);
}


gint
FtpStopThread(camera_t* cam)
{
  ftpthread_info_t *info;
  chain_t *ftp_service;
  ftp_service=GetService(cam,SERVICE_FTP);

  if (ftp_service!=NULL) { // if FTP service running...
    info=(ftpthread_info_t*)ftp_service->data;
    /* Clean cancel handler: */
    pthread_mutex_lock(&info->mutex_cancel);
    info->cancel_req=1;
    pthread_mutex_unlock(&info->mutex_cancel);
    
    /* common handlers...*/
    pthread_join(ftp_service->thread, NULL);
    
    pthread_mutex_lock(&ftp_service->mutex_data);
    pthread_mutex_lock(&ftp_service->mutex_struct);

    RemoveChain(cam,ftp_service);
    
    /* Do custom cleanups here...*/
    if ((info->frame.image!=NULL)&&(info->frame.allocated_image_bytes!=0)) {
      free(info->frame.image);
      info->frame.image=NULL;
      info->frame.allocated_image_bytes=0;
    }
#ifdef HAVE_FTPLIB
    CloseFtpConnection(info->ftp_handle);
#endif
    /* Mendatory cleanups: */
    pthread_mutex_unlock(&ftp_service->mutex_struct);
    pthread_mutex_unlock(&ftp_service->mutex_data);
    FreeChain(ftp_service);
    ftp_service=NULL;
    
  }

  return (1);
}

void
FtpThreadCheckParams(chain_t *ftp_service)
{

  ftpthread_info_t *info;
  info=(ftpthread_info_t*)ftp_service->data;

  // THIS IS ALL AUTOMATIC NOW

  // if some parameters changed, we need to re-allocate the local buffers and restart the ftp
  /*
  if ((ftp_service->current_buffer->frame.size[0]!=ftp_service->local_param_copy.frame.size[0]  )||
      (ftp_service->current_buffer->frame.size[1]!=ftp_service->local_param_copy.frame.size[1])) {
    
    if (info->buffer!=NULL) {
      free(info->buffer);
      info->buffer=NULL;
    }
    info->imlib_buffer_size=ftp_service->current_buffer->frame.size[0]*ftp_service->current_buffer->frame.size[1]*3;
    info->buffer=(unsigned char*)malloc(info->imlib_buffer_size*sizeof(unsigned char));
  }
  */
  // copy all new parameters:
  memcpy(&ftp_service->local_param_copy, ftp_service->current_buffer,sizeof(buffer_t));
  ftp_service->local_param_copy.frame.allocated_image_bytes=0;

}

#ifdef HAVE_FTPLIB
gboolean OpenFtpConnection(chain_t *ftp_service)
{
  char  tmp[STRING_SIZE];
  ftpthread_info_t* info=(ftpthread_info_t*)ftp_service->data;
  camera_t *cam=ftp_service->camera;
  FtpInit();

  //Warning("Ftp: starting...\n");
  if (!FtpConnect(cam->prefs.ftp_address, &info->ftp_handle)) {
    Error("Ftp: connection to server failed");
    return FALSE;
  }
  
  if (FtpLogin(cam->prefs.ftp_user, cam->prefs.ftp_password, info->ftp_handle) != 1) {
    Error("Ftp: login failed.");
    return FALSE;
  }
  
  sprintf(tmp, "Ftp: logged in as %s", cam->prefs.ftp_user);
  Warning(tmp);
  
  if (cam->prefs.ftp_path != NULL && strcmp(cam->prefs.ftp_path,"")) {
    if (!FtpChdir(cam->prefs.ftp_path, info->ftp_handle)) {
      Error("Ftp: chdir failed");
      return FALSE;
    }
    sprintf(tmp, "Ftp: chdir %s", cam->prefs.ftp_path);
    Warning(tmp);
  }
  
  Warning("Ftp: ready to send");

  return TRUE;
}

void
CloseFtpConnection(netbuf *ftp_handle)
{
  FtpQuit(ftp_handle);
}

gboolean
CheckFtpConnection(chain_t *ftp_service)
{
 
  ftpthread_info_t* info=(ftpthread_info_t*)ftp_service->data;

  if (!FtpChdir(".", info->ftp_handle))
    // we can't access the current directory! Connection is probably lost. Reconnect: 
    if (!OpenFtpConnection(ftp_service)) {
      Error("Ftp: Can't restore lost connection");
      return FALSE;
    }
  return TRUE;
}
// V20***
/*
gboolean
FtpPutFrame(char *filename, GdkImlibImage *im, ftpthread_info_t* info)
{
  //netbuf **file_handle=NULL;
  char *tmp;

  tmp=(char*)malloc(STRING_SIZE*sizeof(char));
  // we have to write to a local tmp file to convert...
  
  sprintf(tmp,"/tmp/coriander_ftp_image%s",info->filename_ext);

  gdk_imlib_save_image(im, tmp, NULL);

  if (!FtpPut(tmp, filename, FTPLIB_IMAGE, info->ftp_handle)) {
    free(tmp);
    tmp=NULL;
    Error("Ftp failed to put file.");
    return FALSE;
  }
  free(tmp);
  tmp=NULL;
  return TRUE;
}
*/
#endif // HAVE_FTPLIB
