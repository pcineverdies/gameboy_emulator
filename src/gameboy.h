#ifndef __GAMEBOY_H
#define __GAMEBOY_H

#include "bus/bus.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "memory/WRAM.h"
#include "memory/register.h"
#include "IO/timer.h"
#include "IO/serial.h"
#include "APU/APU.h"
#include "IO/joypad.h"
#include "memory/cartridge.h"
#include "memory/CRAM.h"
#include "PPU/PPU.h"
#include "utils/gb_global_t.h"
#include <string>

#define BUS_FREQUENCY     4194304
#define CPU_FREQUENCY     BUS_FREQUENCY/4
#define SERIAL_FREQUENCY  1
#define JOYPAD_FREQUENCY  1024

class Gameboy{

  Bus*        bus;
  Cartridge*  cart;
  WRAM*       wram;
  CRAM*       cram;
  Memory*     oam;
  Joypad*     joypad;
  Serial*     serial;
  Timer*      timer;
  Register*   if_reg;
  PPU*        ppu;
  APU*        apu;
  Register*   brom_en;
  Memory*     hram;
  Register*   ie_reg;
  Register*   svbk_reg;
  Register*   vbk_reg;
  Cpu*        cpu;

public:

  Gameboy(std::string, uint8_t);
  void run();
  ~Gameboy();
};

#endif
