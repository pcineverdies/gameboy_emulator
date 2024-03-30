#ifndef __PPU_H
#define __PPU_H

#include "../bus/bus_obj.h"
#include "display.h"
#include "PPU_def.h"
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include "../memory/memory_map.h"

class PPU : public Bus_obj {

  enum class State{
    STATE_MODE_2, STATE_MODE_3, STATE_MODE_0, STATE_MODE_1
  };

  Display* display;

  // PPU Registers
  uint8_t LCDC;
  uint8_t STAT;
  uint8_t SCY;
  uint8_t SCX;
  uint8_t LY;
  uint8_t LYC;
  uint8_t DMA;
  uint8_t BGP;
  uint8_t OBP0;
  uint8_t OBP1;
  uint8_t WX;
  uint8_t WY;

  // Internal variables
  enum State _state;

  bool     _STAT_can_fire;

  uint8_t  _DMA_bytes_to_transfer;
  uint8_t  _DMA_cycles_to_wait;

  uint8_t  _OAM_SCAN_to_wait;
  uint8_t  _OAM_SCAN_fetched;
  uint8_t  _OAM_SCAN_addr;
  uint8_t  _OAM_SCAN_buffer[OAM_BUFFER_SIZE_BYTE];

  uint8_t  _DRAWING_to_wait;
  uint32_t _DRAWING_display[SCREEN_WIDTH * SCREEN_HEIGHT];
  uint8_t  _DRAWING_window_condition;
  uint8_t  _DRAWING_window_line_counter;
  uint32_t _DRAWING_display_matrix[SCREEN_HEIGHT * SCREEN_WIDTH];

  uint16_t _HBLANK_padding_to_wait;
  uint16_t _VBLANK_padding_to_wait;

  // Internal functions
  void DMA_OAM_step(Bus_obj*);
  void OAM_SCAN_step(Bus_obj*);
  void DRAWING_step(Bus_obj*);
  void HBLANK_step(Bus_obj*);
  void VBLANK_step(Bus_obj*);
  void set_vblank_interrupt(Bus_obj*);
  void set_stat_interrupt(Bus_obj*);
  bool is_PPU_on();
  void reset();
  void STAT_handler(Bus_obj*);
  uint8_t get_sprite_height();
  uint32_t get_color_from_palette(uint8_t, uint8_t);

public:

  PPU(std::string, uint16_t);
  uint8_t read(uint16_t);
  void    write(uint16_t, uint8_t);
  void    step(Bus_obj*);
  ~PPU();

};

#endif // !__PPU_H
