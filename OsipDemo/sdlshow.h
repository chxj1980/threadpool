#ifndef _SDL_SHOW_H_
#define _SDL_SHOW_H_

extern "C"
{
	#include <sdl/SDL.h>
	#include <sdl/SDL_thread.h>
}

#pragma comment(lib, "sdl2.lib")
#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2test.lib")

namespace nsdl
{
	class sdlshow
	{
		sdlshow();

		//³õÊ¼»¯sdl
		int InitSdl();

		//Ïú»Ùsdl
		int ReleaseSdl();
	};
}





#endif