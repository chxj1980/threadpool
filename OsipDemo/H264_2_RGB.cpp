#include "H264_2_RGB.h"

extern "C"
{
	#include "sdl/SDL.h"
}

extern FILE* g_fp;

AVCodec         *pCodec = NULL;
AVCodecContext  *pCodecCtx = NULL;
SwsContext      *img_convert_ctx = NULL;
AVFrame         *pFrame = NULL;
AVFrame         *pFrameRGB = NULL;
AVPacket		pkt;

FILE * fw;

//初始化ffmpeg相关组件
int H264_Init(void)
{
	avcodec_register_all();
	av_register_all();
	AVCodecID avCode;
	avCode = AV_CODEC_ID_H264;

	pCodec = avcodec_find_decoder(avCode);
	pCodecCtx = avcodec_alloc_context3(pCodec);
	
	pFrame = av_frame_alloc();
	if (pFrame == NULL)
		return -1;
	pFrameRGB = av_frame_alloc();
	if (pFrameRGB == NULL)
		return -1;
	av_init_packet(&pkt);

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		return -1;

	if (fw == NULL)
		fw = fopen("a.mp4", "wb");

	return 0;
}

int g_init = 0;

SDL_Surface *screen;
SDL_Surface *image;

//转码函数
int H264_2_RGB(unsigned char *inputbuf, int frame_size, unsigned char *outputbuf, unsigned int*outsize, int *nWidth, int *nHeight)
{
	int             decode_size;
	int             numBytes;
	int             av_result;
//	uint8_t         *buffer = NULL;

	printf("Video decoding\n");

	int nGetPic;
	uint8_t *buffer_stream;
	buffer_stream = (uint8_t*)malloc(1024 * 1024 * 4);
	int buffer_pos = 0;
	int total_decode_len = 0;

	uint8_t* buffer_rgb = NULL;

	//memcpy(buffer_stream + buffer_pos, inputbuf, frame_size);
	//buffer_pos += frame_size;

	//pkt.data = (unsigned char*)buffer_stream + total_decode_len;
	//pkt.size = buffer_pos;

	pkt.data = inputbuf;
	pkt.size = frame_size;

	av_result = avcodec_decode_video2(pCodecCtx, pFrame, &nGetPic, &pkt);
	if (av_result < 0)
	{
		fprintf(stderr, "decode failed: inputbuf = 0x%x , input_framesize = %d\n", inputbuf, frame_size);
		return -1;
	}
	else
	{
	/*	buffer_pos -= av_result;
		total_decode_len += av_result;
		pkt.data = (unsigned char*)buffer_stream + total_decode_len + av_result;
		pkt.size -= av_result;*/
	}

	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
		return -1;

	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0)
		return -1; 
	
	if (nGetPic)
	{
		//if (pFrameRGB == NULL)
		//{
			//pFrameRGB = av_frame_alloc();
		int bytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
		buffer_rgb = (uint8_t *)av_malloc(bytes);
		avpicture_fill((AVPicture *)pFrameRGB, buffer_rgb, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

		if (g_init == 0)
		{
			if (SDL_Init(SDL_INIT_VIDEO) < 0)
			{
				fprintf(stderr, "can not initialize SDL:%s\n", SDL_GetError());
				return -1;
			}

			atexit(SDL_Quit);
			screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, SDL_SWSURFACE | SDL_ANYFORMAT);
			if (screen == NULL)
			{
				exit(2);
			}

			Uint32 rmask, gmask, bmask, amask;
#if 0//SDL_BYTEORDER == SDL_BIG_ENDIAN
			rmask = 0xff000000;
			gmask = 0x00ff0000;
			bmask = 0x0000ff00;
			amask = 0x000000ff;
#else
			rmask = 0x000000ff;
			gmask = 0x0000ff00;
			bmask = 0x00ff0000;
			amask = 0xff000000;
#endif
			image = SDL_CreateRGBSurface(SDL_SWSURFACE, pCodecCtx->width, pCodecCtx->height, 0,
				rmask, gmask, bmask, NULL);
			if (image == NULL)
			{
				//fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
				exit(1);
			}
			g_init = 1;
		}

		//}
		// Determine required buffer size and allocate buffer  
		//numBytes = avpicture_get_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
		//buffer = (uint8_t*)malloc(numBytes * sizeof(uint8_t));

		// Assign appropriate parts of buffer to image planes in pFrameRGB  
		//avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);

		//img_convert_ctx = sws_getCachedContext(img_convert_ctx, pCodecCtx->width, pCodecCtx->height,
		//	//PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,  
		//	pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24,
		//	SWS_X, NULL, NULL, NULL);

		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
			pCodecCtx->width,
			pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		if (img_convert_ctx == NULL)
		{

			printf("can't init convert context!\n");
			return -1;
		}
		//pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
		//pFrame->linesize[0] *= 1;
		//pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);;
		//pFrame->linesize[1] *= 1;
		//pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);;
		//pFrame->linesize[2] *= 1;
		//sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, 0 - pCodecCtx->width, pFrameRGB->data, pFrameRGB->linesize);
		sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
		


		*outsize = pCodecCtx->width * 3 * pCodecCtx->height;
		//printf("%s\n", pFrameRGB->data[0]);
		//*outsize = (unsigned int)pFrameRGB->linesize;
		//*outsize = pkt.size;
		//*outsize = pFrameRGB->width * pFrameRGB->height * 3;
		//fprintf(g_fp, "%s\n", pFrameRGB->data[0]);
		//if (fw != NULL)
		//	fwrite(pFrameRGB->data, 1, *outsize, fw);
		memcpy(screen->pixels, buffer_rgb, *outsize);
		
		*nWidth = pCodecCtx->width;
		*nHeight = pCodecCtx->height;

		SDL_UpdateRect(screen, 0, 0, image->w, image->h);

		/* Free the allocated BMP surface */
		//SDL_FreeSurface(image);

		sws_freeContext(img_convert_ctx);
		//free(buffer_rgb);
	}
	
	return 0;
}

void H264_Release(void)
{
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	av_free(pFrame);
	av_free(pFrameRGB);
	if (fw != NULL)
	{
		fclose(fw);
		fw = NULL;
	}
	//av_free(pkt);
}