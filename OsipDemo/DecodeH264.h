#ifndef __DECODE_H264_H__
#define __DECODE_H264_H__

//ʹ��ffmpeg��H264��Ƶ��������RGB��ʽ����sdl����ʾ
extern "C"
{
	//ffmpeg�����ڽ���
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

	//sdl��������ʾ
#include <SDL.h>
#include <SDL_thread.h>
}


namespace ffh264{

	class H264Decoder
	{
	private:
		//��ʼffmpeg��ؽṹ
		bool InitH264();

	public:
		//���ý��뺯�� ,�����ʽ
		/*
			�����ʽ 0 - RGB32
					 1 - RGB24
					 2 - YUV420P
					 3 - YUV422P
		*/
		void DecodeH264(unsigned char* outbuf, int nType);

	private:
		//���º�������֪�ⲻ�ٽ���
		void H264ToYUV420P();

		void H264ToYUV422P();

		void H264ToRGB32();

		void H264ToRGB24();

	private:
		//������غ���
		void ReleaseH264();

	private:
		//��س������
		AVCodec         *pCodec;
		AVCodecContext  *pCodecCtx;
		SwsContext      *img_convert_ctx;
		AVFrame         *pFrame;
		AVFrame         *pFrameRGB;
		AVPacket		pkt;
		int				sws_flags;
	};
}


#endif 