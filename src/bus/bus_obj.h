#ifndef __BUS_OBJ_H
#define __BUS_OBJ_H

#include <cstdint>
#include <string>

// Pure virtual class for objects connected
// to a bus.
class Bus_obj {

protected:
  uint16_t init_addr;
  uint16_t last_addr;
  uint16_t size_addr;

  uint32_t frequency;

public:
  std::string name;

  Bus_obj(std::string, uint16_t, uint16_t);
  uint16_t get_size();
  uint16_t get_init_addr();
  uint16_t get_last_addr();
  void     set_frequency(uint32_t);
  uint32_t get_frequency();
  virtual uint8_t read(uint16_t) = 0;
  virtual void write(uint16_t, uint8_t) = 0;
  virtual void step(Bus_obj*) = 0;
  virtual ~Bus_obj() {};

};

#endif // !__BUS_OBJ_H
