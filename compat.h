#ifndef COMPAT_H
#define COMPAT_H
#include <SDL2/SDL_surface.h>
#include <stdint.h>

typedef unsigned char UCHAR;
typedef int32_t DWORD;
typedef uint16_t USHORT;

int32_t GetTickCount();
void Sleep(int32_t);

class POINT {
public:
  int32_t x;
  int32_t y;
};

void NDDraw_Draw_Surface(SDL_Surface* src, int x, int y, int w, int h,
                         SDL_Surface* dest, int transparent = 1);

void DDraw_Draw_Surface(SDL_Surface* src, int x, int y, int w, int h,
                        SDL_Surface* dest, int transparent = 1);

void DDraw_DrawSized_Surface(SDL_Surface* src, int x, int y, int w, int h,
                             int sw, int sh, SDL_Surface* dest,
                             int transparent = 1);

#endif
