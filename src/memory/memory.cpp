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
    Memory constructor. Sets working frequency to 0.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus
    @param size uint16_t Size of the addressable space of the object

*/
Memory::Memory(std::string name, uint16_t init_addr, uint16_t size) : Bus_obj(name, init_addr, size){
  this->set_frequency(0);
  this->memory.resize(this->size_addr);
  for(int i = 0; i < this->size_addr; i++) this->memory[i] = 0;
}

/** Memory::init_from_file
    Initialize the content of the memory using a file.
    The file is read byte per byte and moved to the inner memory.
    When the final address of the memory is reached or the EOF, the
    initialization terminates.

    @param init_addr uint16_t Initial address in which the file is moved
    @param file_name std::string Name of the file to use

*/
void Memory::init_from_file(uint16_t init_addr, std::string file_name){

  std::ifstream file;
  uint16_t current_address = init_addr;
  std::ifstream stream(file_name);

  if(stream.is_open()){
    char byte;
    while(stream.get(byte)){
      if(current_address > this->last_addr){
        break;
      }
      this->memory[current_address++] = byte;
    }
  }
  else{
    throw std::invalid_argument("File for " + this->name + " not opened correctly");
  }

}

