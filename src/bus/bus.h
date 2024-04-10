#ifndef __BUS_H
#define __BUS_H

#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "bus_obj.h"
#include "../PPU/PPU_def.h"

#define BUS_STEP_SIZE 4
#define FPS 60

class Bus : public Bus_obj{

  /*
   * By having an array of pointer to abstract objects, we can
   * keep track of all the objects attached to the bus. Each object
   * will contain its starting address and size. This is not accurate
   * with respsect to the architecture, but just a way to keep track
   * of the information. If the size of an object is 0, that object is
   * not addressable (for instance, the cpu)
   * */
  std::vector<Bus_obj*> bus_objects;

  /*
   * For each object, the values of frequency, init_addr and size
   * are cached, so that we do not need to call the corresponding methods
   * each single time. This leads to a massive improvements in terms of
   * performances.
   *
   * */
  std::vector<uint32_t> frequency_cache;
  std::vector<uint32_t> init_addr_cache;
  std::vector<uint32_t> size_cache;

  // Takes care of couting the current clock cycle.
  uint32_t current_cc;

  // How many cycles have been passed since last frame randered
  uint32_t current_cycles_counting;

  // Last initial SDL tick since new frame was rendered
  uint32_t initial_tick_frame;

public:

  Bus(std::string, uint16_t, uint16_t, uint32_t);

  // Perform a read operation
  uint8_t read(uint16_t);

  // Perform a write operation
  void write(uint16_t, uint8_t);

  // Add element to the bus
  void add_to_bus(Bus_obj*);

  // Step for all the attached elements
  void step(Bus_obj*);

  ~Bus(){}

};

#endif // !__BUS_H
