#include "bus.h"

Bus::Bus(){
  ROM_00 = new Memory("ROM00", ROM00_SIZE);
  ROM_NN = new Memory("ROMNN", ROMNN_SIZE);
  VRAM = new Memory("VRAM", VRAM_SIZE);
  ERAM = new Memory("ERAM", ERAM_SIZE);
  WRAM_00 = new Memory("WRAM00", WRAM00_SIZE);
  WRAM_NN = new Memory("WRAMNN", WRAMNN_SIZE);
  OAM = new Memory("OAM", OAM_SIZE);
  IO = new Memory("IO", IO_SIZE);
  HRAM = new Memory("HRAM", HRAM_SIZE);
  IE = new Memory("IE", IE_SIZE);
}

Memory* Bus::memory_to_address(uint16_t& addr){

  if (addr >= ROM00_START and addr <= ROM00_END){
    addr -= ROM00_START;
    return ROM_00;
  }

  if (addr >= ROMNN_START and addr <= ROMNN_END){
    addr -= ROMNN_START;
    return ROM_NN;
  }

  if (addr >= VRAM_START and addr <= VRAM_END){
    addr -= VRAM_START;
    return VRAM;
  }

  if (addr >= ERAM_START and addr <= ERAM_END){
    addr -= ERAM_START;
    return ERAM;
  }

  if (addr >= WRAM00_START and addr <= WRAM00_END){
    addr -= WRAM00_START;
    return WRAM_00;
  }

  if (addr >= WRAMNN_START and addr <= WRAMNN_END){
    addr -= WRAMNN_START;
    return WRAM_NN;
  }

  if (addr >= OAM_START and addr <= OAM_END){
    addr -= OAM_START;
    return OAM;
  }

  if (addr >= IO_START and addr <= IO_END){
    addr -= IO_START;
    return IO;
  }

  if (addr >= HRAM_START and addr <= HRAM_END){
    addr -= HRAM_START;
    return HRAM;
  }

  if (addr >= IE_START and addr <= IE_END){
    addr -= IE_START;
    return IE;
  }

  return nullptr;
}

uint8_t Bus::read(uint16_t addr){
  Memory* memory_to_use = memory_to_address(addr);
  if(!memory_to_use) throw std::invalid_argument("Cannot access this area of address space");
  return memory_to_use->read(addr);
}

void Bus::write(uint16_t addr, uint8_t data){

  Memory* memory_to_use = memory_to_address(addr);

  if(!memory_to_use) throw std::invalid_argument("Cannot access this area of address space");
  return memory_to_use->write(addr, data);
}

Bus::~Bus(){
  delete ROM_00;
  delete ROM_NN;
  delete VRAM;
  delete ERAM;
  delete WRAM_00;
  delete WRAM_NN;
  delete OAM;
  delete IO;
  delete HRAM;
  delete IE;
}
