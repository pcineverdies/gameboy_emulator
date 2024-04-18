#include "WRAM.h"

/** WRAM::read
    Read by from wram at a given address, using
    the corrent bank for addresses 0xD000-0xDFFF

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t WRAM::read(uint16_t addr){
  uint8_t res = 0;
  uint8_t bank_to_use = 0;

  if(addr >= size_addr)
    throw std::invalid_argument( "Address of provided to WRAM over the limit\n" );

  if(addr < MMU_BANK_WRAM_SIZE){
    res = memory[bank_to_use][addr];
  }
  else{
    bank_to_use = _bus_to_read->read(MMU_SVBK_REG_INIT_ADDR) & 0b00000111;
    if(bank_to_use == 0) bank_to_use = 1;
    res = memory[bank_to_use][addr - MMU_BANK_WRAM_SIZE];
  }

  return res;
}

/** WRAM::write
    Write a byte to a given address, using
    the corrent bank for addresses 0xD000-0xDFFF

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void WRAM::write(uint16_t addr, uint8_t data){
  uint8_t bank_to_use = 0;

  if(addr >= size_addr)
    throw std::invalid_argument( "Address of provided to WRAM over the limit\n" );

  if(addr < MMU_BANK_WRAM_SIZE){
    memory[bank_to_use][addr] = data;
  }
  else{
    bank_to_use = _bus_to_read->read(MMU_SVBK_REG_INIT_ADDR) & 0b00000111;
    if(bank_to_use == 0) bank_to_use = 1;
    memory[bank_to_use][addr - MMU_BANK_WRAM_SIZE] = data;
  }
}

/** WRAM::WRAM
    WRAM constructor. Sets working frequency to 0.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus
    @param size uint16_t Size of the addressable space of the object

*/
WRAM::WRAM(std::string name, uint16_t init_addr, uint16_t size) : Bus_obj(name, init_addr, size){
  this->set_frequency(0);

  memory.resize(MMU_BANK_WRAM_NUMBER);
  for(auto& bank : memory) bank.resize(MMU_BANK_WRAM_SIZE);
}
