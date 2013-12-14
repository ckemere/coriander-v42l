/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *                         Dan Dennedy  <dan@dennedy.org>
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

/**********************************************************************
 *
 *  CONVERSION FUNCTIONS TO UYVY 
 *
 **********************************************************************/

void
convert_to_rgb(dc1394video_frame_t *in, dc1394video_frame_t *out)
{
  out->color_coding=DC1394_COLOR_CODING_RGB8;
  dc1394_convert_frames(in, out);
}

// we should optimize this for RGB too: RGB modes could use RGB-SDL instead of YUV overlay
void
convert_to_yuv_for_SDL(dc1394video_frame_t *in, SDL_Overlay *sdloverlay, unsigned int overlay_byte_order)
{
  dc1394video_frame_t out;
  unsigned int padding=in->padding_bytes;
  in->padding_bytes=0;             // don't print padding in YUV video buffer
  in->total_bytes=in->image_bytes; // don't print padding in YUV video buffer
  //fprintf(stderr,"%d ",in->color_coding);
  out.color_coding=DC1394_COLOR_CODING_YUV422;
  out.yuv_byte_order=overlay_byte_order;
  out.image=sdloverlay->pixels[0];
  // very large value to avoid re-allocation. In reality, this buffer is allocated by SDL functions so
  // we don't want to touch it here.
  out.allocated_image_bytes=1e12;

  dc1394_convert_frames(in, &out);

  // revert to true values:
  in->padding_bytes=padding;
  in->total_bytes=in->image_bytes+padding;
}

void
convert_for_pvn(unsigned char *buffer, unsigned int width, unsigned int height,
		unsigned int page, int color_mode, unsigned char *dest)
{
  unsigned int bpp;
  unsigned char *buf_loc;

  dc1394_get_color_coding_bit_size(color_mode, &bpp);
  buf_loc=buffer+(page*bpp*width*height)/8;

  if(dest==NULL)
    return;

  switch(color_mode) {
    case DC1394_COLOR_CODING_MONO8:
    case DC1394_COLOR_CODING_RAW8:
      memcpy(dest,buf_loc,width*height);
      break;
    case DC1394_COLOR_CODING_YUV411:
    case DC1394_COLOR_CODING_YUV422:
    case DC1394_COLOR_CODING_YUV444:
      dc1394_convert_to_RGB8(buf_loc, dest, width, height, DC1394_BYTE_ORDER_YUYV, color_mode, 16);
      break;
    case DC1394_COLOR_CODING_RGB8:
      memcpy(dest,buf_loc,3*width*height);
      break;
    case DC1394_COLOR_CODING_MONO16:
    case DC1394_COLOR_CODING_MONO16S:
    case DC1394_COLOR_CODING_RAW16:
      memcpy(dest,buf_loc,2*width*height);
      break;
    case DC1394_COLOR_CODING_RGB16:
      memcpy(dest,buf_loc,6*width*height);
      break;
    default:
      fprintf(stderr, "Unknown buffer format!\n");
      break;
  }
}
