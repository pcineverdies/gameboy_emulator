#include "memory.h"

/** Memory::read
    Read by from memory at a given address

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t Memory::read(uint16_t addr){
  if(addr >= size_addr)
    throw std::invalid_argument( "Address of provided to " + name + " over the limit\n" );

  return this->memory[addr];
}

/** Memory::write
    Write a byte in memory at a given address

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void Memory::write(uint16_t addr, uint8_t data){
  if(addr >= size_addr)
    throw std::invalid_argument( "Address of provided to " + name + " over the limit\n" );

  this->memory[addr] = data;
}

/** Memory::Memory
    Memory constructor

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus
    @param size uint16_t Size of the addressable space of the object

*/
Memory::Memory(std::string name, uint16_t init_addr, uint16_t size) : Bus_obj(name, init_addr, size){
  this->memory.resize(this->size_addr);
  for(int i = 0; i < this->size_addr; i++) this->memory[i] = 0;
}

