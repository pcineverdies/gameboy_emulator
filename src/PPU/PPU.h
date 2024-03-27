#ifndef __PPU_H
#define __PPU_H

#include "../bus/bus_obj.h"
#include "display.h"
#include "PPU_def.h"
#include <cstdint>
#include <cstring>
#include <cstdio>

class PPU : public Bus_obj {

  Display* display;

public:

  PPU(std::string, uint16_t);
  uint8_t read(uint16_t);
  void    write(uint16_t, uint8_t);
  void    step(Bus_obj*);
  void    set_interrupt(Bus_obj*);
  ~PPU();

};

#endif // !__PPU_H
