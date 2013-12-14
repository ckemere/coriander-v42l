#ifndef _VIDEO_ENCODE_
#define _VIDEO_ENCODE_

#ifdef HAVE_FFMPEG

/* Video encode */
AVStream *add_video_stream(AVFormatContext *oc, int codec_id, int width, int height);
AVFrame *alloc_picture(int pix_fmt, int width, int height);
int open_video(AVFormatContext *oc, AVStream *st);
int write_video_frame(AVFormatContext *oc, AVStream *st, AVFrame *tmp_picture);
int close_video(AVFormatContext *oc, AVStream *st);

/* Single image encode */
int ImageToFile_JPEG(unsigned char *image_buffer, 
		     unsigned int image_width, 
		     unsigned int image_height, 
		     char *filename, 
		     char *comment);

int jpeg_write(AVFrame *jpeg_picture, unsigned int width, unsigned int height, int fmt, char *filename, float quality, char *comment);

void uyvy411_yuv411p(const unsigned char *frame, AVFrame *picture, const unsigned int width, const unsigned int height);
void uyvy422_yuv422p(const unsigned char *frame, AVFrame *picture, const unsigned int width, const unsigned int height);

#endif

#endif /* _VIDEO_ENCODE_ */


