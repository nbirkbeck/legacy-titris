#include <SDL2/SDL.h>
#include <iostream>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void DDraw_DrawSized_Surface(SDL_Surface* src, int x, int y, int w, int h,
                             int sw, int sh, SDL_Surface* dest,
                             int transparent) {
  SDL_Rect src_rect = {0, 0, w, h};
  SDL_Rect dest_rect = {x, y, sw, sh};
  SDL_BlitScaled(src, &src_rect, dest, &dest_rect);
}

void DDraw_Draw_Surface(SDL_Surface* src, int x, int y, int w, int h,
                        SDL_Surface* dest, int transparent) {
  SDL_Rect dest_rect = {x, y, w, h};
  SDL_BlitScaled(src, nullptr, dest, &dest_rect);
}

void NDDraw_Draw_Surface(SDL_Surface* src, int x, int y, int w, int h,
                         SDL_Surface* dest, int transparent) {
  SDL_Rect src_rect = {x, y, w, h};
  SDL_BlitScaled(src, &src_rect, dest, nullptr);
}

int32_t GetTickCount() {
  static bool init = false;
  static struct timeval base_time;
  if (!init) {
    init = true;
    gettimeofday(&base_time, nullptr);
  }
  struct timeval tv;
  struct timeval elapsed_time;
  gettimeofday(&tv, nullptr);
  timersub(&tv, &base_time, &elapsed_time);
  return elapsed_time.tv_sec * 1000 + elapsed_time.tv_usec / 1000;
}

void Sleep(int32_t ms) {
  useconds_t usec = ((useconds_t)ms) * 1000;
#ifdef __EMSCRIPTEN__
  emscripten_sleep(usec / 1000);
#else
  usleep(usec);
#endif
}
