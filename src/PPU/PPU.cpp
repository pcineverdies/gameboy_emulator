#include "PPU.h"

// TOCHANGE SIZE
PPU::PPU(std::string name, uint16_t init_addr) : Bus_obj(name, init_addr, 1){
  this->display = new Display(SCREEN_WIDTH, SCREEN_HEIGHT, SCALE_FACTOR);
}


uint8_t PPU::read(uint16_t addr){
  return 0;
}

void PPU::write(uint16_t addr, uint8_t data){

}

void PPU::step(Bus_obj* bus){

}

void PPU::set_interrupt(Bus_obj* bus){

}

PPU::~PPU(){
  delete display;
}
