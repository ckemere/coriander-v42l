/*
 * Video encoding functions for Coriander 
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

/*
 * This code is based on the example code supplied with FFMPEG
 */

#include "coriander.h"

#ifdef HAVE_FFMPEG

#define STREAM_FRAME_RATE 30 /* 30 images/s */ // This should be extracted from camera setting

//AVFrame *picture;
uint8_t *video_outbuf;
int video_outbuf_size;

void uyvy411_yuv411p(const unsigned char *frame, AVFrame *picture, const unsigned int width, const unsigned int height) {
  const register unsigned char *src = frame;
  register unsigned char *y = picture->data[0], 
    *u = picture->data[1], 
    *v = picture->data[2];
  register unsigned int size = width*height/4;
  while (size--) {
    *u++ = *src++; // U2
    *y++ = *src++; // Y0
    *y++ = *src++; // Y1
    *v++ = *src++; // V2
    *y++ = *src++; // Y2
    *y++ = *src++; // Y3
  };
  return;
}

void uyvy422_yuv422p(const unsigned char *frame, AVFrame *picture, const unsigned int width, const unsigned int height)
{
  const register unsigned char *src = frame;
  register unsigned char *y = picture->data[0], 
    *u = picture->data[1], 
    *v = picture->data[2];
  register unsigned int size = width*height/2;
  while (size--) {
    *u++ = *src++; // U0
    *y++ = *src++; // Y0
    *v++ = *src++; // V0
    *y++ = *src++; // Y1
  };
  return;
}

void rgb8_rgb24(const unsigned char *frame, AVFrame *picture, const unsigned int width, const unsigned int height)
{
  const register unsigned char *src = frame;
  register unsigned char *r = picture->data[0], 
    *g = picture->data[1], 
    *b = picture->data[2];
  register unsigned int size = width*height;
  while (size--) {
    *r++ = *src++; // R
    *g++ = *src++; // G
    *b++ = *src++; // B
  };
  return;
}


/* add a video output stream */
AVStream *add_video_stream(AVFormatContext *oc, enum CodecID codec_id, int width, int height)
{
  AVCodec *codec;
  AVCodecContext *c;
  AVStream *st;

  codec = avcodec_find_encoder(codec_id);

  if (!codec) {
    fprintf(stderr, "Could not find encoder for '%d'.\n", codec_id);
    exit(1);
  }
  
  st = avformat_new_stream(oc, codec);
  if (!st) {
    fprintf(stderr, "Could not alloc stream\n");
    exit(1);
  }

  c = st->codec;
  c->codec_id = codec_id;
  c->codec_type = AVMEDIA_TYPE_VIDEO;
  
  /* put sample parameters */
  // Stuff to try to force high quality encoding
  c->bit_rate = 6000000;
  c->qmax = 2; // low is better, so frame-by-frame force high quality
  /* just for testing, we also get rid of B frames */
  if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
    c->max_b_frames = 0;
  }

  if (c->codec_id == CODEC_ID_THEORA) {
    c->global_quality = 10000; // for libtheora
    c->flags |= CODEC_FLAG_QSCALE;  // for libtheora
  }

  /* resolution must be a multiple of two */
  c->width = width;  
  c->height = height;
  c->pix_fmt = PIX_FMT_YUV420P; 
  //c->pix_fmt = PIX_FMT_YUVJ420P;  // only for LJPEG or MJPEG
  /* frames per second */
  c->time_base = (AVRational){1,STREAM_FRAME_RATE};
  //st->avg_frame_rate = c->time_base;
  c->gop_size = 1; /* emit one intra frame every n frames at most */
  // some formats want stream headers to be seperate
  if (oc->oformat->flags & AVFMT_GLOBALHEADER)
    c->flags |= CODEC_FLAG_GLOBAL_HEADER;

  return st;
}

AVFrame *alloc_pframe(int pix_fmt, int width, int height) {
    AVFrame *picture;
    uint8_t *picture_buf;
    int size;
    
    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = av_malloc(size);
    if (!picture_buf) {
        av_free(picture);
        picture=NULL;
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf, 
                   pix_fmt, width, height);
    return picture;
}
    
int open_video(AVFormatContext *oc, AVStream *st) {
    AVCodec *codec;
    AVCodecContext *c;
    int err;

    c = st->codec;

    /* find the video encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        return -1;
    }

    /* open the codec */
    err = avcodec_open2(c, codec, NULL);
    if (err < 0) {
        fprintf(stderr, "could not open codec (err: %d)\n", err);
        return -1;
    }

    video_outbuf = NULL;
    if (!(oc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        video_outbuf_size = 1000000;
        video_outbuf = malloc(video_outbuf_size);
    }


    return 1;
}

int write_video_frame(AVFormatContext *oc, AVStream *st, AVFrame *picture) {
    int out_size, ret;
    AVCodecContext *c;
    
    c = st->codec;

    /* encode the image */
    /* if zero size, it means the image was buffered */
    /* write the compressed frame in the media file */
    /* XXX: in case of B frames, the pts is not yet valid */
    out_size = avcodec_encode_video(c, video_outbuf, video_outbuf_size, picture);
    if (out_size > 0) {
        AVPacket pkt;
        av_init_packet(&pkt);

        if (c->coded_frame->pts != AV_NOPTS_VALUE)
            pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
        if(c->coded_frame->key_frame)
            pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index= st->index;
        pkt.data= video_outbuf;
        pkt.size= out_size;

        /* write the compressed frame in the media file */
        //ret = av_interleaved_write_frame(oc, &pkt);
        ret = av_write_frame(oc, &pkt);
    } else {
        ret = 0;
    }

    if (ret != 0) {
        fprintf(stderr, "Error while writing video frame\n");
        return -1;
    }
    
    return ret;
}

int close_video(AVFormatContext *oc, AVStream *st, AVFrame *picture) {
    avcodec_close(st->codec);
    av_free(picture->data[0]);
    av_free(picture);

    av_free(video_outbuf);
    video_outbuf=NULL;

    return 1;
}

/* 
 * Encode an image to JPEG using FFMPEG
 */

int
jpeg_write(AVFrame *jpeg_picture, unsigned int width, unsigned int height, int fmt, char *filename, float quality, char *comment)
{
  AVCodec *encoder;
  AVCodecContext *jpeg_c = NULL;
  uint8_t *jpeg_outbuf   = NULL;
  unsigned int jpeg_outbuf_size, jpeg_size;
  FILE *jpeg_fd;
  
  avcodec_register_all();
  encoder=avcodec_find_encoder(CODEC_ID_MJPEG);
  if (!encoder) {
      fprintf(stderr,"************\n");
      return -1;
  }

  jpeg_c = avcodec_alloc_context();

  //jpeg_c->bit_rate = 10000000; // heuristic...
  jpeg_c->width = width;
  jpeg_c->height = height;
  jpeg_c->pix_fmt = fmt;//PIX_FMT_YUVJ420P;
  //jpeg_c->dct_algo = FF_DCT_AUTO;
  //jpeg_c->idct_algo = FF_IDCT_AUTO;
  /* set the quality */
  /* XXX: a parameter should be used */
  //jpeg_picture->quality = (int) 1 + FF_LAMBDA_MAX * ( (100-quality)/100.0 ); 
  jpeg_picture->quality = (int) 1+4096*((100-quality)/100.0);
  jpeg_c->flags |= CODEC_FLAG_QSCALE;
 
  //sprintf(jpeg_c->comment, comment);

  if (avcodec_open(jpeg_c, encoder) < 0) {
    fprintf(stderr,"************\n");
    av_free(jpeg_c);
    jpeg_c=NULL;
    return -1;
  }
  
  jpeg_outbuf_size = 3*width*height; // heuristic...
  jpeg_outbuf = av_malloc(jpeg_outbuf_size);

  jpeg_size = avcodec_encode_video(jpeg_c, jpeg_outbuf, jpeg_outbuf_size, jpeg_picture);
  if (jpeg_size > 0) {
    jpeg_fd = fopen(filename, "w");
    fwrite(jpeg_outbuf, jpeg_size, 1, jpeg_fd);
    fclose(jpeg_fd);
  }
  
  avcodec_close(jpeg_c);
  av_free(jpeg_outbuf);
  av_free(jpeg_c);
  jpeg_c=NULL;
  jpeg_outbuf=NULL;
  return 1;
}


#endif
