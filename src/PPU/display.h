#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <cstdint>
#include <SDL2/SDL.h>
#include <stdexcept>
#include "PPU_def.h"

class Display {
  SDL_Event event;
  SDL_Renderer *renderer;
  SDL_Window *window;

  uint8_t width;
  uint8_t height;
  uint8_t scale_factor;

public:
        Display(uint8_t, uint8_t, uint8_t);
  void  update(uint8_t, uint8_t, uint32_t);
  void  clear(uint32_t);
        ~Display();
};

#endif // !__DISPLAY_H

