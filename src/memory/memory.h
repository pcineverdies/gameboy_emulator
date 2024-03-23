#ifndef __MEMORY_H
#define __MEMORY_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <fstream>
#include "../bus/bus_obj.h"

class Memory : public Bus_obj  {
  std::vector<uint8_t> memory;

public:
            Memory(std::string, uint16_t, uint16_t);
  uint8_t   read(uint16_t);
  void      write(uint16_t, uint8_t);
  void      step(Bus_obj*){}
  void      init_from_file(uint16_t, std::string);
            ~Memory(){}
};

#endif // !__MEMORY_H
