/*
 * Frame information functions for Coriander 
 * Copyright (c) 2004, Sergio Rui Silva
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "coriander.h"

char * new_subtitle(int n, float fps, char *time_string, char * subtitle) {

  sprintf(subtitle, "{%d}{%d} Frame : %d, FPS : %3.2f, Time stamp : %s\n",
	  n,
	  n+1,
	  n,
	  fps,
	  time_string);

  return subtitle;
}

char * subtitle_header(char * subtitle) {

  sprintf(subtitle, "### BEGIN HEADER\nFRAME TIMESTAMP FILE\nFor each"
        "frame: seconds since epoch [UINT64], fractional nanoseconds [UINT64]\n"
        "###END HEADER\n");

  return subtitle;
}
