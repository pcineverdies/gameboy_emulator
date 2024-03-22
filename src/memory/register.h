#ifndef __REGISER_H
#define __REGISTER_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <cstring>
#include "../bus/bus_obj.h"

class Register : public Bus_obj  {

  uint8_t reg;

public:
            Register(std::string, uint16_t);
  uint8_t   read(uint16_t);
  void      write(uint16_t, uint8_t);
  void      step(Bus_obj*){}
            ~Register(){}
};

#endif // !__REGISTER_H
