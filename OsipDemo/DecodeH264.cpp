#include "DecodeH264.h"



//调用sdl库直接将RGB视频流显示
//#pragma comment(lib, "SDL2.lib")

//解码库
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")

using namespace ffh264;

//初始ffmpeg相关结构
bool H264Decoder::InitH264()
{
	// /*注册所有可用的格式和编解码器*/
	av_register_all();
	return false;
}

//调用解码函数 ,解码格式
/*
解码格式 
0 - RGB32
1 - RGB24
2 - YUV420P
3 - YUV422P
*/
void H264Decoder::DecodeH264(unsigned char* outbuf, int nType)
{
	
}


//以下函数见名知意不再解释
void H264Decoder::H264ToYUV420P()
{

}

void H264Decoder::H264ToYUV422P()
{

}

void H264Decoder::H264ToRGB32()
{

}

void H264Decoder::H264ToRGB24()
{

}

//析构相关函数
void H264Decoder::ReleaseH264()
{

}