#include "bus.h"

/** Bus::Bus
    Constructor of the class. It just calls the parent constructor.
    By having a bus which is Bus_obj, we can have a hierarchy of buses.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus
    @param size uint16_t Size of the addressable space of the object

*/
Bus::Bus(std::string name, uint16_t init_addr, uint16_t size) : Bus_obj(name, init_addr, size){}

/** Bus::add_to_bus
    Add the pointer of an object to the list of handled objects. It
    checks that the requested addresses were not used by a previous object

    There are 4 overlap situations (this is the usual probelm of ranges of dates):

    1.  Prev:            |------------------|
        New:        |---------------|

    2.  Prev:        |---------------|
        New:            |------------------|

    3.  Prev:                |--------|
        New:            |------------------|

    4.  Prev:            |------------------|
        New:                |--------|

    Each of these sitations have to lead to an error.

    @param new_object Bus_obj* object to add to the bus

*/
void Bus::add_to_bus(Bus_obj* new_object){

  for(auto& obj : bus_objects){
    uint16_t N_i = new_object->get_init_addr();
    uint16_t N_l = new_object->get_last_addr();
    uint16_t P_i = obj->get_init_addr();
    uint16_t P_l = obj->get_last_addr();

    if((N_i <= P_i and N_l >= P_i) or (N_i >= P_i and N_i <= P_l) or (N_i >= P_i and N_l <= P_l) or (N_i <= P_i and P_l <= N_l))
      throw std::invalid_argument("Overlap in address space");
  }

  bus_objects.push_back(new_object);
}


/** Bus::read
    Read by from memory at a given address.
    Try all the objects connected to the bus.

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t Bus::read(uint16_t addr){
  for(auto& bus_obj : bus_objects){

    uint16_t size = bus_obj->get_size();
    uint16_t init_addr = bus_obj->get_init_addr();

    if(addr >= init_addr && addr < init_addr + size)
      return bus_obj->read(addr - init_addr);
  }

  return 0;
}

/** Memory::write
    Write a byte in memory at a given address.
    Try with all the object connected to the bus.

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
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

