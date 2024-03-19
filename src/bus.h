#ifndef __BUS_H
#define __BUS_H

#include <cstdint>
#include "memory.h"
#include <stdexcept>
#include <iostream>

#define ROM00_START 0x0000
#define ROM00_END   0x3FFF
#define ROM00_SIZE  0x4000

#define ROMNN_START 0x4000
#define ROMNN_END   0x7FFF
#define ROMNN_SIZE  0x4000

#define VRAM_START  0x8000
#define VRAM_END    0x9FFF
#define VRAM_SIZE   0x2000

#define ERAM_START  0xA000
#define ERAM_END    0xBFFF
#define ERAM_SIZE   0x2000

#define WRAM00_START 0xC000
#define WRAM00_END   0xCFFF
#define WRAM00_SIZE  0x1000

#define WRAMNN_START 0xD000
#define WRAMNN_END   0xDFFF
#define WRAMNN_SIZE  0x1000

#define OAM_START    0xFE00
#define OAM_END      0xFE9F
#define OAM_SIZE     0x00A0

#define IO_START     0xFF00
#define IO_END       0xFF7F
#define IO_SIZE      0x0080

#define HRAM_START   0xFF80
#define HRAM_END     0xFFFE
#define HRAM_SIZE    0x007f

#define IE_START     0xFFFF
#define IE_END       0xFFFF
#define IE_SIZE      0x0001

class Bus{

  Memory* memory_to_address(uint16_t&);

public:

  Memory* ROM_00;
  Memory* ROM_NN;
  Memory* VRAM;
  Memory* ERAM;
  Memory* WRAM_00;
  Memory* WRAM_NN;
  Memory* OAM;
  Memory* IO;
  Memory* HRAM;
  Memory* IE;

  Bus();

  // Perform a read operation
  uint8_t read(uint16_t);

  // Perform a write operation
  void write(uint16_t, uint8_t);

  ~Bus();

};

#endif // !__BUS_H
