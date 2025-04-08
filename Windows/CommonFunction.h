
#ifndef COMMON_FUNCTION_H_
#define COMMON_FUNCTION_H_

#include <windows.h>
#include <string>
#include <SDL.h>
#include <SDL_image.h>



extern SDL_Window* g_window;
extern SDL_Renderer* g_screen;
static SDL_Event g_event;

//Screen
const int Screen_WIDTH = 1280;
const int Screen_HEIGHT = 640;
const int Screen_BPP = 32;

const int COLOR_KEY_R = 167;
const int COLOR_KEY_G = 175;
const int COLOR_KEY_B = 180;

const int RENDER_DRAW_COLOR = 0xff;
#endif
