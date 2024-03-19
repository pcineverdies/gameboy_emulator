#include "memory.h"

/** Memory::read
    Read by from memory at a given address

    @param addr uint16_t address to read
    @return uint8_t read byte
*/
uint8_t Memory::read(uint16_t addr){
  if(addr >= size)
    throw std::invalid_argument( "Address of provided to " + name + " over the limit\n" );

  return this->memory[addr];
}

/** Memory::write
    Write a byte in memory at a given address

    @param addr uint16_t address to use
    @param data uint8_t  byte to write
*/
void Memory::write(uint16_t addr, uint8_t data){
  if(addr >= size)
    throw std::invalid_argument( "Address of provided to " + name + " over the limit\n" );

  this->memory[addr] = data;
}

/** Memory::Memory
    Memory constructor

    @param size uint8_t  number of bytes in the memory
*/
Memory::Memory(std::string name, uint32_t size){
  this->name = name;
  this->size = size;
  this->memory.resize(size);
  for(int i = 0; i < size; i++) this->memory[i] = 0;
}

