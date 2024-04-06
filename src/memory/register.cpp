#include "register.h"

/** Register::read
    Read the register

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t Register::read(uint16_t addr){
  if(addr != 0)
    throw std::invalid_argument( "Address of provided to " + name + " over the limit\n" );

  return this->reg;
}

/** Register::write
    Write a byte in memory at a given address

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void Register::write(uint16_t addr, uint8_t data){
  if(addr != 0)
    throw std::invalid_argument( "Address of provided to " + name + " over the limit\n" );

  this->reg = data;
}

/** Register::Register
    Register constructor. Sets working frequency to 0.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus

*/
Register::Register(std::string name, uint16_t init_addr, uint8_t init_value) : Bus_obj(name, init_addr, 1){
  this->set_frequency(0);
  this->reg = init_value;
}

