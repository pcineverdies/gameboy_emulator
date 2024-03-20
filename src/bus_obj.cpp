#include "bus_obj.h"

Bus_obj::Bus_obj(std::string name, uint16_t init_addr, uint16_t size_addr){
  this->name = name;
  this->init_addr = init_addr;
  this->size_addr = size_addr;
  this->last_addr = init_addr + size_addr - 1;
}

uint16_t Bus_obj::get_size(){
  return this->size_addr;
}

uint16_t Bus_obj::get_init_addr(){
  return this->init_addr;
}


