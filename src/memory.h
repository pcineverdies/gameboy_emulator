#ifndef __MEMORY_H
#define __MEMORY_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <cstring>
#include "bus_obj.h"

class Memory : public Bus_obj  {
  std::vector<uint8_t> memory;

public:
            Memory(std::string, uint16_t, uint16_t);
  uint8_t   read(uint16_t);
  void      write(uint16_t, uint8_t);
            ~Memory(){}
};

#endif // !__MEMORY_H
