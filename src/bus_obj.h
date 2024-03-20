#ifndef __BUS_OBJ_H
#define __BUS_OBJ_H

#include <cstdint>
#include <string>

class Bus_obj {

protected:
  uint16_t init_addr;
  uint16_t last_addr;
  uint16_t size_addr;
  std::string name;

public:
  Bus_obj(std::string, uint16_t, uint16_t);
  uint16_t get_size();
  uint16_t get_init_addr();
  virtual uint8_t read(uint16_t) = 0;
  virtual void write(uint16_t, uint8_t) = 0;
  virtual ~Bus_obj() {};

};

#endif // !__BUS_OBJ_H
