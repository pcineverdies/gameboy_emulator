#ifndef __BUS_H
#define __BUS_H

#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "bus_obj.h"

class Bus : public Bus_obj{

  std::vector<Bus_obj*> bus_objects;

public:

  Bus(std::string, uint16_t, uint16_t);

  // Perform a read operation
  uint8_t read(uint16_t);

  // Perform a write operation
  void write(uint16_t, uint8_t);

  // Add element to the bus
  void add_to_bus(Bus_obj*);

  ~Bus(){}

};

#endif // !__BUS_H
