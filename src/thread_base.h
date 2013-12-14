/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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

#ifndef __THREAD_BASE_H__
#define __THREAD_BASE_H__

#define GUID_YUV12_PLANAR 0x32315659 
#define GUID_YUY2_PACKED  0x32595559
#define GUID_UYVY_PACKED  0x59565955

chain_t*
GetService(camera_t* cam, service_t service);

void
CommonChainSetup(camera_t* cam, chain_t* chain, service_t req_service);

void
PublishBufferForNext(chain_t *chain);

int
GetBufferFromPrevious(chain_t *chain);

void
InsertChain(camera_t* cam, chain_t* chain);

void
RemoveChain(camera_t* cam, chain_t* chain);

void
InitChain(camera_t* cam, chain_t *chain, service_t service);

void
FreeChain(chain_t* chain);

void
InitBuffer(buffer_t *buffer);

#endif // __THREAD_BASE_H__
