//#include "sdl/SDL.h"
//#include <stdio.h>
//#include <time.h>
//
//const int SCREEN_WIDTH = 512;
//const int SCREEN_HEIGHT = 489;
//
//SDL_Window *window = NULL;
//SDL_Renderer *renderer = NULL;
//
//
//
///* print out error information */
//void ShowError()
//{
//	printf("%s\n", SDL_GetError());
//}
//
///* load image by filename */
//SDL_Texture* LoadImage(char file[]) 
//{
//	SDL_Surface *loadedImage = NULL;
//	SDL_Texture *texture = NULL;
//
//	loadedImage = SDL_LoadBMP(file);
//	if (loadedImage != NULL) {
//		texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
//		SDL_FreeSurface(loadedImage);
//	}
//	else
//		ShowError();
//	return texture;
//}
//
///* show the backgroud */
//void ApplyBackground(SDL_Texture *tex, SDL_Renderer *rend) 
//{
//	SDL_Rect pos;
//	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
//	pos.x = pos.y = 0;
//	SDL_RenderCopy(renderer, tex, NULL, NULL);
//}
//
///* show the pointers by angle */
//void ApplySurface(SDL_Texture *tex, SDL_Renderer *rend, double angle) 
//{
//	int x = SCREEN_WIDTH / 2 + 7;
//	int y = SCREEN_HEIGHT / 2;
//
//	SDL_RendererFlip flip = SDL_FLIP_VERTICAL;
//	SDL_Rect pos;
//	SDL_QueryTexture(tex, NULL, NULL, &pos.w, &pos.h);
//
//	pos.x = x;
//	pos.y = y;
//	SDL_Point p = { 0, 0 };
//	SDL_RenderCopyEx(renderer, tex, NULL, &pos, angle, &p, flip);
//}
//
//int main(void) 
//{
//	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) 
//	{
//		ShowError();
//		return 1;
//	}
//
//	/* Initial Window */
//	window = SDL_CreateWindow("Clock", SDL_WINDOWPOS_CENTERED,
//		SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
//	if (window == NULL) {
//		ShowError();
//		return 2;
//	}
//
//	/* Initial Renderer */
//	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
//	if (renderer == NULL) {
//		ShowError();
//		return 3;
//	}
//
//	/* load pictures */
//	SDL_Texture *background = NULL, *hour = NULL,
//		*min = NULL, *second = NULL;
//	background = LoadImage("background.bmp");
//	hour = LoadImage("hour.bmp");
//	min = LoadImage("min.bmp");
//	second = LoadImage("second.bmp");
//
//	if (background == NULL | hour == NULL | min == NULL | second == NULL) {
//		return 4;
//	}
//
//	time_t rawtime;
//	struct tm *timeinfo;
//	int quit = 0;
//	SDL_Event e;
//
//	/* begin to run */
//	while (!quit) {
//		time(&rawtime);
//		timeinfo = localtime(&rawtime);
//
//		/* get system time */
//		int h = timeinfo->tm_hour % 12, m = timeinfo->tm_min,
//			s = timeinfo->tm_sec;
//
//		/* calculate their angles */
//		double angle_s = s*1.0 * 6 - 90;
//		double angle_m = m*1.0 * 6 + angle_s / 360 * 6 - 90;
//		double angle_h = h*1.0 * 30 + angle_m / 360 / 2 - 90;
//
//		/* repaint */
//		SDL_RenderClear(renderer);
//		ApplyBackground(background, renderer);
//		ApplySurface(hour, renderer, angle_h);
//		ApplySurface(min, renderer, angle_m);
//		ApplySurface(second, renderer, angle_s);
//
//		SDL_RenderPresent(renderer);
//		SDL_Delay(1000);
//
//		/* way to quit */
//		while (SDL_PollEvent(&e)) {
//			if (e.type == SDL_QUIT) // close the window 
//				quit = 1;
//			if (e.type == SDL_MOUSEBUTTONDOWN) // click the mouse
//				quit = 1;
//		}
//	}
//	/* Free Memory */
//	SDL_DestroyTexture(background);
//	SDL_DestroyTexture(hour);
//	SDL_DestroyTexture(min);
//	SDL_DestroyTexture(second);
//	SDL_DestroyRenderer(renderer);
//	SDL_DestroyWindow(window);
//
//	SDL_Quit();
//
//	return 0;
//}