#include "sdlshow.h"
#include <stdio.h>

using namespace nsdl;

sdlshow::sdlshow()
{

}

//��ʼ��sdl
int InitSdl()
{
#if 1
	//��ʼ��sdl��
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "can not initialize SDL:%s\n", SDL_GetError());
		return -1;
	}

	SDL_Surface *screen;
	screen = SDL_SetVideoMode(720, 576, 24, SDL_SWSURFACE | SDL_ANYFORMAT);
	if (screen == NULL)
	{
		exit(2);
	}
	SDL_Surface *image;

#endif 


	Uint32 rmask, gmask, bmask, amask;

	return 0;
}

//����sdl
int ReleaseSdl()
{
	return 0;
}