#ifndef __MEMORY_H
#define __MEMORY_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <cstring>

class Memory {
  uint32_t size;
  std::vector<uint8_t> memory;
  std::string name;

public:
            Memory(std::string, uint32_t);
  uint8_t   read(uint16_t);
  void      write(uint16_t, uint8_t);
};

#endif // !__MEMORY_H
