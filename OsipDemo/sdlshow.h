#ifndef _SDL_SHOW_H_
#define _SDL_SHOW_H_

extern "C"
{
	#include <sdl/SDL.h>
	#include <sdl/SDL_thread.h>
}

#pragma comment(lib, "sdl.lib")
#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "SDLtest.lib")

namespace nsdl
{
	class sdlshow
	{
		sdlshow();

		//��ʼ��sdl
		int InitSdl();

		//����sdl
		int ReleaseSdl();
	};
}





#endif