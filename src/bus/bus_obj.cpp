#include "bus_obj.h"

/** Bus_obj::Bus_obj
    Constructs the object

    @param name std::string Name of the object (debug purposes)
    @param init_addr uint16_t First address of the object on the bus
    @param size_addr uint16_t Size of the object

*/
Bus_obj::Bus_obj(std::string name, uint16_t init_addr, uint16_t size_addr){
  this->frequency = 0;
  this->name = name;
  this->init_addr = init_addr;
  this->size_addr = size_addr;
  this->last_addr = init_addr + size_addr - 1;
}

/** Bus_obj::set_frequency
    Set the working frequency of the object. 0 for asynchrounous objects.

    @param frequency uint16_t Frequency to use

*/
void Bus_obj::set_frequency(uint16_t frequency){
  this->frequency = frequency;
}

/** Bus_obj::get_frequency
    Return the working frequency of the object

    @return uint16_t frequency

*/
uint16_t Bus_obj::get_frequency(){
  return this->frequency;
}

/** Bus_obj::get_size
    Return the size of the address space of the object

    @return uint8_t size

*/
uint16_t Bus_obj::get_size(){
  return this->size_addr;
}

/** Bus_obj::get_init_addr
    Return the first address of the object connected to the bus

    @return uint8_t intial address

*/
uint16_t Bus_obj::get_init_addr(){
  return this->init_addr;
}

/** Bus_obj::get_last_addr
    Return the last address of the object connected to the bus

    @return uint8_t last address

*/
uint16_t Bus_obj::get_last_addr(){
  return this->last_addr;
}


