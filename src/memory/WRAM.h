#ifndef __WRAM_H
#define __WRAM_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include "memory_map.h"
#include <cstring>
#include <fstream>
#include "../bus/bus_obj.h"

class WRAM : public Bus_obj  {
  std::vector<std::vector<uint8_t>> memory;

public:

  /*
   * WRAM requires to read the bus while performing
   * a reading/writing operation, in order to get the
   * current value of SVBK register.
   * */
  Bus_obj* _bus_to_read;

            WRAM(std::string, uint16_t, uint16_t);
  uint8_t   read(uint16_t);
  void      write(uint16_t, uint8_t);
  void      step(Bus_obj*){}
            ~WRAM(){}
};

#endif // __WRAM_H
