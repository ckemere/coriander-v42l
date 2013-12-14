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

int
GetCameraNodes(void) {

  int err;
  camera_t* camera_ptr;
  dc1394camera_list_t *camera_list;
  int i;
  /*
  struct timeval t1;
  struct timeval t2;

  gettimeofday(&t1,NULL);
  */
  err=dc1394_camera_enumerate(dc1394,&camera_list);
  /*
  gettimeofday(&t2,NULL);

  fprintf(stderr,"dc1394_camera_enumerate :   %ld ms\n", 
	  ( t2.tv_sec - t1.tv_sec )*1000 + ( t2.tv_usec - t1.tv_usec )/1000);
  t1.tv_sec=t2.tv_sec;
  t1.tv_usec=t2.tv_usec;
  */
  // create a list of cameras with coriander's camera type camera_t
  for (i=0;i<camera_list->num;i++) {
    camera_ptr=NewCamera();
    
    // copy the info in the dc structure into the coriander struct.
    camera_ptr->camera_info=dc1394_camera_new(dc1394,camera_list->ids[i].guid);
    /*
    gettimeofday(&t2,NULL);
    fprintf(stderr,"%s %s :   %ld ms\n", 
	    camera_ptr->camera_info->vendor, camera_ptr->camera_info->model,
	    ( t2.tv_sec - t1.tv_sec )*1000 + ( t2.tv_usec - t1.tv_usec )/1000);
    t1.tv_sec=t2.tv_sec;
    t1.tv_usec=t2.tv_usec;
    */

    GetCameraData(camera_ptr);

    AppendCamera(camera_ptr);
  }

  // free camera list:
  dc1394_camera_free_list(camera_list);

  return err;
}

camera_t*
NewCamera(void) {
  camera_t* cam;

  cam=(camera_t*)malloc(sizeof(camera_t));
  cam->prefs.ftp_user = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_address = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_password = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_path = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.v4l_dev_name = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.name = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.save_filename = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.save_filename_ext = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_filename = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_filename_ext = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.ftp_filename_base = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->prefs.overlay_filename = (char*)malloc(STRING_SIZE*sizeof(char));
  cam->bayer_pattern=DC1394_COLOR_FILTER_RGGB;

  pthread_mutex_init(&cam->uimutex, NULL);
  //fprintf(stderr,"new camera allocated\n");
  return cam;

}

void
GetCameraData(camera_t* cam) {

  // this segment is handled manually now...
  /*
  if (cam->camera_info->bmode_capable>0) {
    // set b-mode and reprobe modes,... (higher fps formats might not be reported as available in legacy mode)
    dc1394_video_set_operation_mode(cam->camera_info, DC1394_OPERATION_MODE_1394B);
  }
  */
  if (dc1394_feature_get_all(cam->camera_info, &cam->feature_set)!=DC1394_SUCCESS)
    Error("Could not get camera feature information!");

  //fprintf(stderr,"Grabbing F7 stuff\n");
  GetFormat7Capabilities(cam);
  cam->image_pipe=NULL;
  pthread_mutex_lock(&cam->uimutex);
  cam->want_to_display=0;
  cam->bayer=-1;
  cam->stereo=-1;
  cam->bpp=8;
  CopyCameraPrefs(cam);

  switch (cam->prefs.save_format) {
  case SAVE_FORMAT_PPM:
      sprintf(cam->prefs.save_filename_ext,"ppm");
      break;
  case SAVE_FORMAT_RAW:
      sprintf(cam->prefs.save_filename_ext,"raw");
      break;
#ifdef HAVE_FFMPEG
  case SAVE_FORMAT_JPEG:
      sprintf(cam->prefs.save_filename_ext,"jpeg");
      break;
#endif
  case SAVE_FORMAT_RAW_VIDEO:
      sprintf(cam->prefs.save_filename_ext,"raw");
      break;
  case SAVE_FORMAT_PVN:
      sprintf(cam->prefs.save_filename_ext,"pvn");
      break;
#ifdef HAVE_FFMPEG
  case SAVE_FORMAT_MPEG:
      sprintf(cam->prefs.save_filename_ext,"mpeg");
      break;
#endif
  default:
      sprintf(cam->prefs.save_filename_ext,"unknown");
      break;
  }

  pthread_mutex_unlock(&cam->uimutex);

}

void
AppendCamera(camera_t* cam) {

  camera_t* ptr;

  // if first camera:
  if (cameras==NULL) {
    cameras=cam;
    cam->prev=NULL;
    cam->next=NULL;
  }
  else {
    ptr=cameras; 
    while(ptr->next!=NULL) {
      ptr=ptr->next;
    }
    ptr->next=cam;
    cam->prev=ptr;
    cam->next=NULL;
  }
}

void
SetCurrentCamera(uint64_t guid) {

  camera_t* ptr;
  ptr=cameras;

  while ((ptr->camera_info->guid!=guid)&&(ptr->next!=NULL)) {
    ptr=ptr->next;
  }
  if (ptr->camera_info->guid!=guid)
    fprintf(stderr,"Kaai! Can't find camera GUID in the camera stack!\n");
  else
    camera=ptr;
}

void
RemoveCamera(uint64_t guid) {

  camera_t* ptr;
  ptr=cameras;

  while ((ptr->camera_info->guid!=guid)&&(ptr->next!=NULL)) {
    ptr=ptr->next;
  }

  V4lStopThread(ptr);
  FtpStopThread(ptr);
  SaveStopThread(ptr);
  DisplayStopThread(ptr);
  IsoStopThread(ptr);

  if (ptr->prev!=NULL) // not first camera?
    ptr->prev->next=ptr->next;
  else {
    cameras=ptr->next;
    if (ptr->next!=NULL)
      ptr->next->prev=NULL;
  }

  if (ptr->next!=NULL) // not last camera?
    ptr->next->prev=ptr->prev;
  else {
    if (ptr->prev!=NULL)
      ptr->prev->next=NULL;
  }

  FreeCamera(ptr);
  ptr=NULL;
}

void
FreeCamera(camera_t* cam)
{  
  free(cam->prefs.ftp_user);
  free(cam->prefs.ftp_address);
  free(cam->prefs.ftp_filename); 
  free(cam->prefs.ftp_password); 
  free(cam->prefs.ftp_path); 
  free(cam->prefs.v4l_dev_name); 
  free(cam->prefs.name);
  free(cam->prefs.save_filename);
  free(cam->prefs.save_filename_ext);
  free(cam->prefs.overlay_filename);
  free(cam->camera_info);
  free(cam);
}

