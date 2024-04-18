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
    HDMA3 = (data | 0x80) & 0x9F;
  }
  else if(addr == 3){
    HDMA4 = data & 0xF0;
  }
  else if(addr == 4){
    if(_is_transfering == 0){
      _is_transfering = 1;
      _transfering_mode = (data & 0x80) ? HDMA_HBLANK_DMA : HDMA_GP_DMA;
      _transfer_length = data & 0x7F;
      _current_transfer = 0;
      _source_address = (HDMA1 << 8) | HDMA2;
      _cycles_to_wait = (data & 0x80) ? 0 : 32;
      _destination_address = (HDMA3 << 8) | HDMA4;
      _hblank_to_do = 1;
      HDMA5 = _transfer_length | ((data & 0x80) ? 0x80 : 0x00);
    }
    else{
      if(!(data & 0x80)){
        _is_transfering = 0;
        HDMA5 |= 0x80;
      }
    }

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

  _is_transfering = 0;
  _transfering_mode = 0;
  _transfer_length = 0;
  _current_transfer = 0;
  _cycles_to_wait = 0;
  _destination_address = 0;
  _source_address = 0;
  _hblank_to_do = 0;
}

/** HDMA::step
    Perform the step of the timer at each T-cycle

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void HDMA::step(Bus_obj* bus){
  uint8_t current_STAT = 0;
  uint8_t current_LY = 0;

  // In non-gbc mode no hdma is allowed. Also,
  // no step is required if transfering is being done
  if(gb_global.gbc_mode == 0 or !_is_transfering) return;

  // Case of general purpose DMA: cpu is stuck until the transfer
  // is completed. Each chungs of 16 bytes takes 32 cycles. To simplify
  // timing, 16 bytes are instantly transferred, and then 32 cycles are waited
  if(_transfering_mode == HDMA_GP_DMA){

    // Wait for some cycles
    if(_cycles_to_wait-- != 0) return;

    // In 32 cycles, HDMA is able to move 0x10 bytes
    for(int i = 0; i < 16; i++){
      bus->write(_destination_address + _current_transfer * 16 + i,
                 bus->read(_source_address + _current_transfer * 16 + i));
    }

    _current_transfer++;
    _cycles_to_wait = 32;

    if(_current_transfer > _transfer_length){
      _is_transfering = 0;
      HDMA5 = 0xFF;
    }
  }
  else{
    current_STAT  = bus->read(PPU_STAT);
    current_LY    = bus->read(PPU_LY);

    if(current_LY >= 144) return;
    if((current_STAT & 0x03) == 0x02) _hblank_to_do = 1;
    if((current_STAT & 0x03) != 0x00) return;

    if(_cycles_to_wait){
      if(--_cycles_to_wait == 0){
        _hblank_to_do = 0;
        HDMA5 = 0x80 | HDMA5;
      }
      return;
    }

    if(_hblank_to_do){

      // In 32 cycles, HDMA is able to move 0x10 bytes
      for(int i = 0; i < 16; i++){
        bus->write(_destination_address + _current_transfer * 16 + i,
                  bus->read(_source_address + _current_transfer * 16 + i));
      }

      _current_transfer++;
      _cycles_to_wait = 32;
      HDMA5 = (_transfer_length - _current_transfer);

      if(_current_transfer > _transfer_length){
        _is_transfering = 0;
        HDMA5 = 0xFF;
      }
    }
  }

}

