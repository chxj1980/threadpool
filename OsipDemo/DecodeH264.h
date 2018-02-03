#ifndef __DECODE_H264_H__
#define __DECODE_H264_H__

//使用ffmpeg将H264视频流解析出RGB格式并用sdl库显示
extern "C"
{
	//ffmpeg库用于解码
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

	//sdl库用于显示
#include <SDL.h>
#include <SDL_thread.h>
}


namespace ffh264{

	class H264Decoder
	{
	private:
		//初始ffmpeg相关结构
		bool InitH264();

	public:
		//调用解码函数 ,解码格式
		/*
			解码格式 0 - RGB32
					 1 - RGB24
					 2 - YUV420P
					 3 - YUV422P
		*/
		void DecodeH264(unsigned char* outbuf, int nType);

	private:
		//以下函数见名知意不再解释
		void H264ToYUV420P();

		void H264ToYUV422P();

		void H264ToRGB32();

		void H264ToRGB24();

	private:
		//析构相关函数
		void ReleaseH264();

	private:
		//相关程序变量
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