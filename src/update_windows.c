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

void
UpdateFormat7Window(void)
{
  UpdateFormat7Ranges();
  BuildFormat7ColorMenu();
  UpdateFormat7InfoFrame();
}

void
UpdatePreferencesWindow(void)
{
  UpdatePrefsReceiveFrame();    
  UpdatePrefsDisplayFrame();
  UpdatePrefsGeneralFrame();    
  UpdatePrefsSaveFrame();
  UpdatePrefsFtpFrame();
  UpdatePrefsV4lFrame();
}

void
UpdateCommanderWindow(void)
{
  UpdateCameraFrame();
  UpdateTriggerFrame();
  UpdatePowerFrame();
  UpdateMemoryFrame();
  //  UpdateFormatMenu();
  UpdateIsoFrame();
  UpdateOptionFrame();
  UpdateServiceFrame();
}

void
UpdateFeatureWindow(void)
{
  int i;

  //char stemp[256];

  for (i=DC1394_FEATURE_MIN;i<=DC1394_FEATURE_MAX;i++) {
    if ((camera->feature_set.feature[i-DC1394_FEATURE_MIN].available>0)&&
	(i!=DC1394_FEATURE_TRIGGER)) {
      UpdateRange(i);
    }
  }
}

void
UpdateStatusWindow(void)
{
  UpdateCameraStatusFrame();
  UpdateTransferStatusFrame();
  UpdateBandwidthFrame();
}


void
UpdateAllWindows(void)
{
  UpdatePreferencesWindow();
  //eprint("1\n")
  UpdateFeatureWindow();
  //eprint("2\n")
  UpdateStatusWindow();
  //eprint("3\n")
  UpdateCommanderWindow();
  //eprint("4\n")
  if ((camera->format7_info.edit_mode!=-1)&&(format7_tab_presence==1))
    UpdateFormat7Window();
  //eprint("5\n")
}
