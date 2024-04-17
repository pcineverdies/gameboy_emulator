#ifndef __JOYPAD_H
#define __JOYPAD_H

#include "../bus/bus_obj.h"
#include "../memory/memory_map.h"
#include <cstdint>
#include <string>
#include <SDL2/SDL.h>
#include "../utils/gb_global_t.h"
#include <stdexcept>

#define JOYPAD_VOLUME_DEBOUNCING_DELAY 400
#define JOYPAD_MAX_VOLUME 10
#define JOYPAD_MIN_VOLUME 0

#define JOYPAD_SB_MASK (1 << 5)
#define JOYPAD_DP_MASK (1 << 4)

#define JOYPAD_START_MASK (1 << 3)
#define JOYPAD_DOWN_MASK (1 << 3)

#define JOYPAD_SELECT_MASK (1 << 2)
#define JOYPAD_UP_MASK (1 << 2)

#define JOYPAD_B_MASK (1 << 1)
#define JOYPAD_LEFT_MASK (1 << 1)

#define JOYPAD_A_MASK (1 << 0)
#define JOYPAD_RIGHT_MASK (1 << 0)

#define JOYPAD_START_BUTTON   SDL_SCANCODE_B
#define JOYPAD_SELECT_BUTTON  SDL_SCANCODE_V
#define JOYPAD_A_BUTTON       SDL_SCANCODE_J
#define JOYPAD_B_BUTTON       SDL_SCANCODE_K
#define JOYPAD_UP_BUTTON      SDL_SCANCODE_W
#define JOYPAD_DOWN_BUTTON    SDL_SCANCODE_S
#define JOYPAD_RIGHT_BUTTON   SDL_SCANCODE_D
#define JOYPAD_LEFT_BUTTON    SDL_SCANCODE_A
#define JOYPAD_QUIT_BUTTON    SDL_SCANCODE_Q
#define JOYPAD_VOLUME_UP_BUTTON   SDL_SCANCODE_P
#define JOYPAD_VOLUME_DOWN_BUTTON SDL_SCANCODE_O

class Joypad : public Bus_obj {

  uint8_t JOYP;

  void    set_interrupt(Bus_obj*);
  bool    key_is_pressed(uint8_t);
  bool    update_JOYP();

public:

  Joypad(std::string, uint16_t);
  uint8_t read(uint16_t);
  void    write(uint16_t, uint8_t);
  void    step(Bus_obj*);

};

#endif // !__JOYPAD_H

