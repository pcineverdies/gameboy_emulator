#ifndef __REGISTER_H
#define __REGISTER_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <cstring>
#include "../bus/bus_obj.h"

class Register : public Bus_obj  {

  uint8_t reg;
  uint8_t _available_bits;

public:
            Register(std::string, uint16_t, uint8_t,uint8_t = 0);
  uint8_t   read(uint16_t);
  void      write(uint16_t, uint8_t);
  void      step(Bus_obj*){}
            ~Register(){}
};

#endif // !__REGISTER_H
