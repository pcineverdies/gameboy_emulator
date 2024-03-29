#include "PPU.h"
#include <cstdint>
#include <stdexcept>

PPU::PPU(std::string name, uint16_t init_addr) : Bus_obj(name, init_addr, 12){
  this->display = new Display(SCREEN_WIDTH, SCREEN_HEIGHT, SCALE_FACTOR);
  reset();
}


uint8_t PPU::read(uint16_t addr){
  uint8_t res = 0;
  if     (addr == (PPU_LCDC - PPU_BASE)) res = LCDC;
  else if(addr == (PPU_STAT - PPU_BASE)) res = STAT;
  else if(addr == (PPU_SCY  - PPU_BASE)) res = SCY;
  else if(addr == (PPU_SCX  - PPU_BASE)) res = SCX;
  else if(addr == (PPU_LY   - PPU_BASE)) res = LY;
  else if(addr == (PPU_LYC  - PPU_BASE)) res = LYC;
  else if(addr == (PPU_DMA  - PPU_BASE)) res = DMA;
  else if(addr == (PPU_BGP  - PPU_BASE)) res = BGP;
  else if(addr == (PPU_OBP0 - PPU_BASE)) res = OBP0;
  else if(addr == (PPU_OBP1 - PPU_BASE)) res = OBP1;
  else if(addr == (PPU_WX   - PPU_BASE)) res = WX;
  else if(addr == (PPU_WY   - PPU_BASE)) res = WY;
  else std::invalid_argument("Incorrect address for PPU\n");

  return res;
}

void PPU::write(uint16_t addr, uint8_t data){

  if     (addr == (PPU_LCDC - PPU_BASE)) LCDC = data;

  // STAT's 3 lsbs are read only
  else if(addr == (PPU_STAT - PPU_BASE)) STAT = (data & 0b01111000) | (STAT & 0b00000111);
  else if(addr == (PPU_SCY  - PPU_BASE)) SCY = data;
  else if(addr == (PPU_SCX  - PPU_BASE)) SCX = data;

  // LY is read only
  else if(addr == (PPU_LY   - PPU_BASE)) return;
  else if(addr == (PPU_LYC  - PPU_BASE)) LYC  = data;

  // Starts DMA transaction on next T-cycle
  else if(addr == (PPU_DMA  - PPU_BASE)){
    // Can only use addresses between 0x0000 and 0xdf00 as source
    DMA  = (data > 0xdf) ? 0xdf : data;
    // It's always 160 bytes to transfer = 40 objects, 4 bytes each
    _DMA_bytes_to_transfer = 0xA0;
    // 1 dma operation requires 1 M-cycle = 4 T-cycles
    _DMA_cycles_to_wait = 0;
  }
  else if(addr == (PPU_BGP  - PPU_BASE)) BGP  = data;
  else if(addr == (PPU_OBP0 - PPU_BASE)) OBP0 = data;
  else if(addr == (PPU_OBP1 - PPU_BASE)) OBP1 = data;
  else if(addr == (PPU_WX   - PPU_BASE)) WX   = data;
  else if(addr == (PPU_WY   - PPU_BASE)) WY   = data;
  else std::invalid_argument("Incorrect address for PPU\n");

}

/** PPU::step
    Perform the step of the PPU at each T-cycle.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void PPU::step(Bus_obj* bus){

  if(!is_PPU_on()){
    display->clear(0xffffffff);
    // reset();
    return;
  }

  DMA_OAM_step(bus);

  if      (_state == State::STATE_MODE_2) OAM_SCAN_step(bus);
  else if (_state == State::STATE_MODE_3) DRAWING_step(bus);
  else if (_state == State::STATE_MODE_0) HBLANK_step(bus);
  else if (_state == State::STATE_MODE_1) VBLANK_step(bus);
  else std::runtime_error("Incorect state for PPU");

  STAT_handler(bus);
}

/** PPU::set_vblank_interrupt
    Set the corresponding interrupt flag in the IF register.
    In the OOP approach that is followed in the project, first the
    register must be read, modified and written again. In reality,
    this operation is not time consuming, and can be performed during
    the PPU step.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void PPU::set_vblank_interrupt(Bus_obj* bus){
  uint8_t interrupt_flag_value = bus->read(IF_ADDRESS);
  interrupt_flag_value |= IF_VBLANK;
  bus->write(IF_ADDRESS, interrupt_flag_value);
}

/** PPU::set_vblank_interrupt
    Set the corresponding interrupt flag in the IF register.
    In the OOP approach that is followed in the project, first the
    register must be read, modified and written again. In reality,
    this operation is not time consuming, and can be performed during
    the PPU step.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void PPU::set_stat_interrupt(Bus_obj* bus){
  uint8_t interrupt_flag_value = bus->read(IF_ADDRESS);
  interrupt_flag_value |= IF_LCD;
  bus->write(IF_ADDRESS, interrupt_flag_value);
}

/** PPU::~PPU
    Destroys the SDL2 display object

*/
PPU::~PPU(){
  delete display;
}

