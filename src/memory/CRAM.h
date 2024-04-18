#ifndef __CRAM_H
#define __CRAM_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include "memory_map.h"
#include <cstring>
#include <fstream>
#include "../bus/bus_obj.h"

#define CRAM_BG_PALETTE 0
#define CRAM_OBJ_PALETTE 1

class CRAM : public Bus_obj  {

  std::vector<uint8_t> background_palette;
  std::vector<uint8_t> object_palette;

  uint8_t BCPS;
  uint8_t OCPS;

public:

            CRAM(std::string, uint16_t, uint16_t);
  uint8_t   read(uint16_t);
  void      write(uint16_t, uint8_t);
  void      step(Bus_obj*){}
  uint32_t  read_color_palette(uint8_t, uint8_t, uint8_t);
            ~CRAM(){}
};


#endif //__CRAM_H

