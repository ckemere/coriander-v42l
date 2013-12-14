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

chain_t*
GetService(camera_t* cam, service_t service)
{
  chain_t *chain;

  chain=cam->image_pipe;

  if (chain==NULL) {
    return(NULL);
  }
  else {
    while(chain!=NULL) { 
      pthread_mutex_lock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
      if (service==chain->service) {
	pthread_mutex_unlock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
	return(chain);
      }
      else {
	pthread_mutex_unlock(&chain->mutex_struct); // WARNING: see callbacks.c, on_service_iso_toggled
	chain=chain->next_chain;
      }
    }
  }

  return(NULL);
}


void
PublishBufferForNext(chain_t *chain)
{
  buffer_t* tmp_buffer;
  chain_t *tmp_chain;

  // mutexes are not necessary (and should not be there)
  // because we are in a mutex clamp when this function is executed.

  //pthread_mutex_lock(&chain->mutex_data);
  
  if ((chain->service==SERVICE_ISO)){
    if (chain->next_chain!=NULL) {

      tmp_buffer=chain->current_buffer;
      chain->current_buffer=chain->next_buffer;
      chain->next_buffer=tmp_buffer;
      chain->updated=1;
      //fprintf(stderr,"service %d published buffer 0x%x, ready =%d\n",
      //      chain->service,chain->next_buffer, chain->ready);
      
      chain->ready=0;
    }
  }
  else {
    if (chain->next_chain!=NULL) {
      tmp_buffer=chain->current_buffer;
      chain->current_buffer=chain->next_buffer;
      chain->next_buffer=tmp_buffer;
      chain->updated=1;
    }
    else {
      //eprint("find iso\n");
      // find the ISO service
      tmp_chain=chain;
      while(tmp_chain->prev_chain!=NULL) {
	tmp_chain=tmp_chain->prev_chain;
	if (tmp_chain->service==SERVICE_ISO)
	  break;
      }
      if (tmp_chain->service==SERVICE_ISO)
	tmp_chain->ready=1;
      //chain->current_buffer->used=1;
    }
  }
  //pthread_mutex_unlock(&chain->mutex_data);
}

int
GetBufferFromPrevious(chain_t *chain)
{
  int new_current=0;
  buffer_t* tmp_buffer;

  if (chain->prev_chain!=NULL) { // look for previous chain existance
    pthread_mutex_lock(&chain->prev_chain->mutex_data);
    if (chain->prev_chain->updated>0) { // look for updated picture

      //fprintf(stderr,"service %d got buffer 0x%x, updated=%d\n",
      //      chain->service,chain->prev_chain->next_buffer, chain->prev_chain->updated);

      // get previous chain image
      tmp_buffer=chain->current_buffer;
      chain->current_buffer=chain->prev_chain->next_buffer;
      chain->prev_chain->next_buffer=tmp_buffer;
      chain->prev_chain->updated=0;
      new_current=1;
    }
    else
      new_current=0;
    
    pthread_mutex_unlock(&chain->prev_chain->mutex_data);
  }

  return(new_current);
  
}


void
CommonChainSetup(camera_t* cam, chain_t* chain, service_t req_service)
{
  chain_t* probe_chain;

  // no thread tries to access this data before it is connected.
  // It is thus safe not to use mutex here (for local data only of course).
  
  chain->service=req_service;
  
  probe_chain=cam->image_pipe; // set the begin point for search

  if (req_service==SERVICE_ISO) {
    chain->next_chain=probe_chain;// the chain is inserted BEFORE probe_chain
    chain->prev_chain=NULL;
  }
  else if (probe_chain==NULL) { // empty pipe
    chain->next_chain=NULL;
    chain->prev_chain=NULL;
  }
  else {
    pthread_mutex_lock(&probe_chain->mutex_struct);
    while (1) {
      if (probe_chain->service>req_service) {// the chain is inserted BEFORE probe_chain
	chain->next_chain=probe_chain;
	chain->prev_chain=probe_chain->prev_chain;
	pthread_mutex_unlock(&probe_chain->mutex_struct);
	break;
      }
      else if (probe_chain->next_chain==NULL) {
	chain->next_chain=probe_chain->next_chain;// the chain is inserted AFTER probe_chain
	chain->prev_chain=probe_chain;
	pthread_mutex_unlock(&probe_chain->mutex_struct);
	break;
      }
      else { //advance in pipeline
	pthread_mutex_unlock(&probe_chain->mutex_struct);
	probe_chain=probe_chain->next_chain;
	pthread_mutex_lock(&probe_chain->mutex_struct);
      }
    }
  }

  // allocate buffer structures, NOT IMAGE BUFFERS:
  chain->current_buffer=(buffer_t*)malloc(sizeof(buffer_t));
  chain->next_buffer=(buffer_t*)malloc(sizeof(buffer_t));

  chain->camera=cam;

  InitBuffer(chain->current_buffer);
  InitBuffer(chain->next_buffer);
  InitBuffer(&chain->local_param_copy);

  chain->updated=0;

  chain->ready=1;
}


void
InsertChain(camera_t* cam, chain_t* chain)
{

  // we should only use mutex_struct in this function
  /*
  chain_t *tmp;
  eprint("Services before: ");
  tmp=cam->image_pipe;
  if (tmp!=NULL) {
    eprint("%d ",tmp->service);

    while (tmp->next_chain!=NULL) {
      tmp=tmp->next_chain;
      eprint("%d ",tmp->service);
    }
  }
  else
    eprint("none\n");
  */

  // we should now effectively make the break in the pipe:
  if (chain->next_chain!=NULL)
    pthread_mutex_lock(&chain->next_chain->mutex_struct);
  if (chain->prev_chain!=NULL)
    pthread_mutex_lock(&chain->prev_chain->mutex_struct);
  
  if (chain->prev_chain!=NULL)
    chain->prev_chain->next_chain=chain;
  else // we have a new first chain
    cam->image_pipe=chain;

  if (chain->next_chain!=NULL)
    chain->next_chain->prev_chain=chain;
  
  if (chain->prev_chain!=NULL)
    pthread_mutex_unlock(&chain->prev_chain->mutex_struct);
  if (chain->next_chain!=NULL)
    pthread_mutex_unlock(&chain->next_chain->mutex_struct);   

  //chain_t *tmp;
  /*
  eprint("Services after: ");
  tmp=cam->image_pipe;
  if (tmp!=NULL) {
    eprint("%d ",tmp->service);

    while (tmp->next_chain!=NULL) {
      tmp=tmp->next_chain;
      eprint("%d ",tmp->service);
    }
  }
  else
    eprint("none\n");

  eprint("\n");
  */
}


void
RemoveChain(camera_t* cam, chain_t* chain)
{

  // we should only use mutex_struct in this function

  // LOCK
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    pthread_mutex_lock(&chain->prev_chain->mutex_struct);
  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    pthread_mutex_lock(&chain->next_chain->mutex_struct);

  // note that we want simultaneous lock of the prev AND next chains before we disconnect
  // the current chain.
  
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    chain->prev_chain->next_chain=chain->next_chain;
  else // it was the first chain
    cam->image_pipe=chain->next_chain;

  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    chain->next_chain->prev_chain=chain->prev_chain;
  
  // UNLOCK
  if (chain->prev_chain!=NULL)// lock prev_mutex if we are not the first in the line
    pthread_mutex_unlock(&chain->prev_chain->mutex_struct);
  if (chain->next_chain!=NULL)// lock next_mutex if we are not the last in the line
    pthread_mutex_unlock(&chain->next_chain->mutex_struct);


}

void
FreeChain(chain_t* chain)
{

  if (chain!=NULL) {
    if (chain->data!=NULL) {
      free(chain->data);
      chain->data=NULL;
    }
    if (chain->current_buffer!=NULL) {
      if (chain->current_buffer->frame.allocated_image_bytes!=0) {
	free(chain->current_buffer->frame.image);
	chain->current_buffer->frame.image=NULL;
	chain->current_buffer->frame.allocated_image_bytes=0;
      }
      free(chain->current_buffer);
      chain->current_buffer=NULL;
    }
    if (chain->next_buffer!=NULL) {
      if (chain->next_buffer->frame.allocated_image_bytes!=0) {
	//fprintf(stderr,"0x%x\n",chain->next_buffer->frame.image);
	free(chain->next_buffer->frame.image);
	chain->next_buffer->frame.image=NULL;
	chain->next_buffer->frame.allocated_image_bytes=0;
      }
      free(chain->next_buffer);
      chain->next_buffer=NULL;
    }
    free(chain);
  }
}


void
InitBuffer(buffer_t *buffer)
{
  buffer->frame.size[0]=-1;
  buffer->frame.size[1]=-1;
  buffer->frame.total_bytes=-1;
  buffer->bayer=-1;
  buffer->frame.data_depth=-1;
  buffer->frame.color_filter=-1; //COLOR_FILTER_FORMAT7_BGGR
  buffer->stereo=-1;
  buffer->frame.color_coding=-1;
  buffer->frame.image=NULL;
  buffer->frame.image_bytes=-1;
  buffer->frame.allocated_image_bytes=0;
}
