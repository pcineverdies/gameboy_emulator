#include "HDMA.h"
#include <cstdio>

extern struct gb_global_t gb_global;

/** HDMA::read
    Read by from the hdma at a given address

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t HDMA::read(uint16_t addr){

  uint8_t res = 0;

  if(gb_global.gbc_mode == 0) return 0xff;

  if     (addr == 0) res = HDMA1;
  else if(addr == 1) res = HDMA2;
  else if(addr == 2) res = HDMA3;
  else if(addr == 3) res = HDMA4;
  else if(addr == 4) res = HDMA5;
  else{
    throw std::invalid_argument("Not valid address for HRAM reading");
  }

  return res;
}

/** HDMA::write
    Write of the registers

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void HDMA::write(uint16_t addr, uint8_t data){

  if(gb_global.gbc_mode == 0) return;

  if (addr == 0){
    HDMA1 = data;
  }
  else if(addr == 1){
    HDMA2 = data & 0xF0;
  }
  else if(addr == 2){
    HDMA3 = data;
  }
  else if(addr == 3){
    HDMA4 = data & 0xF0;
  }
  else if(addr == 4){
    _is_transfering = 1;
    _transfering_mode = (data & 0x80) ? HDMA_HBLANK_DMA : HDMA_GP_DMA;
    _transfer_length = ((data & 0x7F) + 1) * 0x10;
    _current_transfer = 0;
    _source_address = (HDMA1 << 8) | HDMA2;
    _cycles_to_wait = 32;
    _destination_address = (HDMA3 << 8) | HDMA4;
    HDMA5 = 0x00;

    #ifdef __DEBUG
    if(_transfering_mode == HDMA_HBLANK_DMA)
      printf("init transfer HDMA HBLANK...\n");
    else
      printf("init transfer HDMA GB_DMA...\n");
    #endif // __DEBUG
  }
  else{
    throw std::invalid_argument("Not valid address for HRAM reading");
  }
}

/** HDMA::HDMA
    HDMA constructor, setting init_addr of the hdma in the memory space
    The size of the object is fixed to 5.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus

*/
HDMA::HDMA(std::string name, uint16_t init_addr) : Bus_obj(name, init_addr, 5){
  HDMA1 = 0;
  HDMA2 = 0;
  HDMA3 = 0;
  HDMA4 = 0;
  HDMA5 = 0x80; // Transfer not active
}

/** HDMA::step
    Perform the step of the timer at each T-cycle

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void HDMA::step(Bus_obj* bus){

  if(gb_global.gbc_mode == 0 or !_is_transfering) return;

  // Wait for some cycles
  if(_cycles_to_wait-- != 0) return;

  // In 32 cycles, HDMA is able to move 0x10 bytes
  for(int i = 0; i < 0x10; i++){
    bus->write(_destination_address + _current_transfer + i,
               bus->read(_source_address + _current_transfer + i));
  }

  _current_transfer += 0x10;
  _cycles_to_wait = 32;

  if(_current_transfer == _transfer_length){
    printf("Done transfer\n");
    _is_transfering = 0;
    HDMA5 = 0xFF;
  }

}

