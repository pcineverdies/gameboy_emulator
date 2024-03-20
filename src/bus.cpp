#include "bus.h"

Bus::Bus(std::string name, uint16_t init_addr, uint16_t size) : Bus_obj(name, init_addr, size){}

// TODO: Check non-overlapping addresses
void Bus::add_to_bus(Bus_obj* new_object){
  bus_objects.push_back(new_object);
}


uint8_t Bus::read(uint16_t addr){
  for(auto& bus_obj : bus_objects){

    uint16_t size = bus_obj->get_size();
    uint16_t init_addr = bus_obj->get_init_addr();

    if(addr >= init_addr && addr < init_addr + size)
      return bus_obj->read(addr - init_addr);
  }

  return 0;
}

void Bus::write(uint16_t addr, uint8_t data){

  for(auto& bus_obj : bus_objects){

    uint16_t size = bus_obj->get_size();
    uint16_t init_addr = bus_obj->get_init_addr();

    if(addr >= init_addr && addr < init_addr + size){
      bus_obj->write(addr - init_addr, data);
      return;
    }
  }

}

