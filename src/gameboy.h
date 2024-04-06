#ifndef __GAMEBOY_H
#define __GAMEBOY_H

#include "bus/bus.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "memory/register.h"
#include "IO/timer.h"
#include "IO/serial.h"
#include "IO/joypad.h"
#include "memory/cartridge.h"
#include "PPU/PPU.h"
#include <string>

#define BUS_FREQUENCY     4194304
#define CPU_FREQUENCY     BUS_FREQUENCY/4
#define SERIAL_FREQUENCY  1
#define JOYPAD_FREQUENCY  1024

class Gameboy{

  Bus*        bus;
  Cartridge*  cart;
  Memory*     wram;
  Memory*     oam;
  Joypad*     joypad;
  Serial*     serial;
  Timer*      timer;
  Register*   if_reg;
  PPU*        ppu;
  Register*   brom_en;
  Memory*     hram;
  Register*   ie_ref;
  Cpu*        cpu;

public:

  Gameboy(std::string, uint8_t);
  void run();
  ~Gameboy();
};

#endif
