#ifndef __SERIAL_H
#define __SERIAL_H

#include "../bus/bus_obj.h"
#include "../memory/memory_map.h"
#include <cstdint>
#include <string>
#include <stdexcept>

class Serial : public Bus_obj {

  uint8_t SB;
  uint8_t SC;

public:

  Serial(std::string, uint16_t);
  uint8_t read(uint16_t);
  void    write(uint16_t, uint8_t);
  void    step(Bus_obj*);
  void    set_interrupt(Bus_obj*);

};

#endif // !__SERIAL_H
