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

#ifndef __TOOLS_H__
#define __TOOLS_H__

typedef struct _xvinfo
{
#ifdef HAVE_XV
  Display *dpy;
  XvAdaptorInfo *ainfo;
  XvEncodingInfo *encodings;
  XvImageFormatValues *formats;
  unsigned int nencode, nadaptors;
  int numImages;
  int ImageEncodings;
#endif
  int max_height;
  int max_width;
} xvinfo_t;

void
GetFormat7Capabilities(camera_t* cam);

unsigned int
SwitchToNearestFPS(dc1394framerates_t *framerates, dc1394framerate_t current);

void
ChangeModeAndFormat         (GtkMenuItem     *menuitem,
			     gpointer         user_data);
void
IsoFlowCheck(int *state); 

void
IsoFlowResume(int *state);

void
GetContextStatus(void);

void
GrabSelfIds(camera_t *cams);
//GrabSelfIds(raw1394handle_t* handles, int portmax);
/*
void
SetIsoChannels(void);
*/
void
SetScaleSensitivity(GtkWidget* widget, int feature, dc1394bool_t sense);

void
SetAbsoluteControl(int feature, int power);

void
SetAbsValue(int feature);

void
GetAbsValue(int feature);

int
bus_reset_handler(raw1394handle_t handle, unsigned int generation);

int
main_timeout_handler(gpointer* port_num);

void
SetFormat7Crop(int sx, int sy, int px, int py, int mode);

int
NearestValue(int value, int step, int min, int max);

void
IsOptionAvailableWithFormat(camera_t* cam,int* bayer, int* stereo, int *bpp16);

/*
void
window_set_icon(GtkWidget* window);
*/

void
GetXvInfo(xvinfo_t *xvinfo);

void
AutoIsoReceive(dc1394switch_t pwr);

#endif // __TOOLS_H__

