#include "DecodeH264.h"



//����sdl��ֱ�ӽ�RGB��Ƶ����ʾ
//#pragma comment(lib, "SDL2.lib")

//�����
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")

using namespace ffh264;

//��ʼffmpeg��ؽṹ
bool H264Decoder::InitH264()
{
	// /*ע�����п��õĸ�ʽ�ͱ������*/
	av_register_all();
	return false;
}

//���ý��뺯�� ,�����ʽ
/*
�����ʽ 
0 - RGB32
1 - RGB24
2 - YUV420P
3 - YUV422P
*/
void H264Decoder::DecodeH264(unsigned char* outbuf, int nType)
{
	
}


//���º�������֪�ⲻ�ٽ���
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

//������غ���
void H264Decoder::ReleaseH264()
{

}