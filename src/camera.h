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

#ifndef __CAMERA_H__
#define __CAMERA_H__

int
GetCameraNodes(void);

camera_t*
NewCamera(void);

void
GetCameraData(camera_t* cam);

void
AppendCamera(camera_t* camera);

void
SetCurrentCamera(uint64_t guid);

void
RemoveCamera(uint64_t guid);

void
FreeCamera(camera_t* camera);

#endif // __CAMERA_H__
