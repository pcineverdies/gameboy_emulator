#include "PPU.h"
#include "PPU_def.h"
#include <cstdint>
#include <cstdio>

void PPU::reset(){

  LCDC = 0;
  STAT = 0;
  SCY = 0;
  SCX = 0;
  LY = 0;
  LYC = 0;
  DMA = 0;
  BGP = 0;
  OBP0 = 0;
  OBP1 = 0;
  WX = 0;
  WY = 0;

  _DMA_bytes_to_transfer = 0;
  _DMA_cycles_to_wait = 0;

  _OAM_SCAN_to_wait = 0;
  _OAM_SCAN_fetched = 0;
  _OAM_SCAN_addr = 0;
  for(int i = 0; i < OAM_BUFFER_SIZE_BYTE; i++) _OAM_SCAN_buffer[i] = 0;

  _state = State::STATE_MODE_2;

}

void PPU::DMA_OAM_step(Bus_obj* bus){

  uint16_t src_addr;
  uint16_t dst_addr;
  uint8_t  byte_addr;

  // Nothing to transfer
  if(_DMA_bytes_to_transfer == 0) return;

  // Some cycles to wait
  if(_DMA_cycles_to_wait != 0){
    _DMA_cycles_to_wait--;
    return;
  }

  // Since internal timing of DMA operation is unknown, a possibility is
  // to consider both the reading and the writing happening in the same
  // T-cycle. This is clearly impossible from a bus perspective

  // Next byte to write
  byte_addr = OAM_SIZE - _DMA_bytes_to_transfer;
  // Source address
  src_addr  = (DMA << 8) | byte_addr;
  // Destination address
  dst_addr  = OAM_INIT_ADDR | byte_addr;

  // Perform the DMA operation
  bus->write(dst_addr, bus->read(src_addr));

  // Setup next byte to transfer
  if(_DMA_bytes_to_transfer-- != 0) _DMA_cycles_to_wait = 3;

}

void PPU::OAM_SCAN_step(Bus_obj* bus){

  uint16_t current_address;
  uint8_t obj_Y_pos;
  uint8_t obj_X_pos;
  uint8_t obj_tile_number;
  uint8_t obj_sprite_flag;

  // The analysis of ecah object in OAM "takes" 2 cycles
  if(_OAM_SCAN_to_wait != 0){
    _OAM_SCAN_to_wait--;
    return;
  }

  // Address from OAM memory to use
  current_address = OAM_INIT_ADDR | _OAM_SCAN_addr;

  // Fetch the 4 bytes
  obj_X_pos =       bus->read(current_address);
  obj_Y_pos =       bus->read(current_address + 1);
  obj_tile_number = bus->read(current_address + 2);
  obj_sprite_flag = bus->read(current_address + 3);

  // Increment next address to use
  _OAM_SCAN_addr += 4;

  // If all the conditions are valid, then the object is to be added to the OAM buffer
  if( _OAM_SCAN_fetched != OAM_BUFFER_SIZE_OBJ and
      obj_X_pos > 0 and
      LY + 16 >= obj_Y_pos and
      LY + 16 < obj_Y_pos + get_sprite_height())
  {
    _OAM_SCAN_buffer[_OAM_SCAN_fetched * 4    ] = obj_X_pos;
    _OAM_SCAN_buffer[_OAM_SCAN_fetched * 4 + 1] = obj_Y_pos;
    _OAM_SCAN_buffer[_OAM_SCAN_fetched * 4 + 2] = obj_tile_number;
    _OAM_SCAN_buffer[_OAM_SCAN_fetched * 4 + 3] = obj_sprite_flag;

    // One more element added to the buffer
    _OAM_SCAN_fetched++;
  }

  // How many cycles to wait before next fetch
  _OAM_SCAN_to_wait = 1;

  // If address is 160, then the whole OAM memory was covered, and we can move
  // to the next phase
  if(_OAM_SCAN_addr == OAM_SIZE){
    _OAM_SCAN_to_wait = 0;
    _OAM_SCAN_addr = 0;

    _DRAWING_to_wait = 172;

    _state = State::STATE_MODE_3;
  }
}

void PPU::DRAWING_step(Bus_obj* bus){

  uint16_t background_map_address = (LCDC & PPU_LCDC_B_TILE_MAP_MASK) ? 0x9C00 : 0x9800;
  uint16_t window_map_address     = (LCDC & PPU_LCDC_W_TILE_MAP_MASK) ? 0x9C00 : 0x9800;
  uint16_t tile_index;
  uint16_t tile_index_offset;
  uint16_t tile_number;
  uint16_t tile_address;
  uint8_t upper_tile;
  uint8_t lower_tile;

  _DRAWING_to_wait--;

  if(_DRAWING_to_wait != 0) return;

  // For each pixel in the line
  for(int x = 0; x < SCREEN_WIDTH; x++){

    tile_index_offset = ((x + SCX) & 0x1f);
    tile_index_offset += (32 * (((LY + SCY) & 0xFF) / 8));
    tile_index_offset &= 0x3ff;
    tile_index = background_map_address + tile_index_offset;
    tile_number = bus->read(tile_index);

    if(LCDC & PPU_LCDC_T_DATA_SEL_MASK)
      tile_address = 0x8000 + tile_number * 16;
    else
      tile_address = 0x9000 + (signed char)tile_number * 16;

    tile_address+= (2 * ((LY + SCY) % 8));
    lower_tile = bus->read(tile_address);
    upper_tile = bus->read(tile_address + 1);

    uint8_t mask = 1 << (7 - (x%8));

    if((lower_tile & mask) and (upper_tile & mask)) display->update(x, LY,   0x000000ff);
    if(!(lower_tile & mask) and (upper_tile & mask)) display->update(x, LY,  0x5a5a5aff);
    if((lower_tile & mask) and !(upper_tile & mask)) display->update(x, LY,  0x808080ff);
    if(!(lower_tile & mask) and !(upper_tile & mask)) display->update(x, LY, 0xffffffff);

  }

  _state = State::STATE_MODE_0;
  _HBLANK_padding_to_wait = 284;
}

void PPU::HBLANK_step(Bus_obj* bus){

  // Handle HBLANK as busy waiting until 456 per scanline have passed
  _HBLANK_padding_to_wait--;

  // If this condition is not true, we move either to the
  // next scanline or to VBLANK
  if(_HBLANK_padding_to_wait != 0) return;

  // Next scanline
  LY++;

  if(LY == SCREEN_HEIGHT){
    _state = State::STATE_MODE_1;

    set_vblank_interrupt(bus);

    _VBLANK_padding_to_wait = VBLANK_LINE_WAIT;
  }
  else{

    _state = State::STATE_MODE_2;

    _OAM_SCAN_to_wait = 0;
    _OAM_SCAN_fetched = 0;
    _OAM_SCAN_addr = 0;
  }

  return;
}

void PPU::VBLANK_step(Bus_obj* bus){

  static int counter = 0;
  _VBLANK_padding_to_wait--;

  // Pseudo-scanline still to go
  if(_VBLANK_padding_to_wait != 0) return;

  // Next pseudo-scanline
  LY++;

  // Padding for next pseudo-scanline
  _VBLANK_padding_to_wait = VBLANK_LINE_WAIT;

  // If 10 pseudo-scanlines have been handled, move to new frame
  if(LY == SCREEN_HEIGHT + VBLANK_PSEUDO_LINES){
    counter++;
    if(counter % 60 == 0) printf("BV: %d\n", counter);
    _state = State::STATE_MODE_2;
    LY = 0;
  }
}

bool PPU::is_PPU_on(){
  return (LCDC & PPU_LCDC_EN_MASK) ? true : false;
}

uint8_t PPU::get_sprite_height(){
  return (LCDC & PPU_LCDC_SPRITE_SIZE_MASK) ? 16 : 8;
}

void PPU::STAT_handler(Bus_obj* bus){

  // If an interrupt had been fired but one condition is still active,
  // no more conditions can be fired. If this variable is false after the
  // conditions are checked, then on the next CC a new interrupt can be fired
  bool found_interrupt = false;

  // Fire if LYC == LY
  if(STAT & STAT_LYC_INTERRUPT_MASK){
    if(LY == LYC) found_interrupt = true;
  }

  // Fire if in state 2
  if(STAT & STAT_M2_INTERRUPT_MASK){
    if(_state == State::STATE_MODE_2) found_interrupt = true;
  }

  // Fire if in state 1
  if(STAT & STAT_M1_INTERRUPT_MASK){
    if(_state == State::STATE_MODE_1) found_interrupt = true;
  }

  // Fire if in state 0
  if(STAT & STAT_M0_INTERRUPT_MASK){
    if(_state == State::STATE_MODE_0) found_interrupt = true;
  }

  // Fire only if an interrupt was found and STAT can be fired
  if(found_interrupt and _STAT_can_fire){
    _STAT_can_fire = false;
    set_stat_interrupt(bus);
  }

  // Make fire possible again
  if(!found_interrupt) _STAT_can_fire = true;

  // Handle coincidence bit (LY == LYC)
  if(LY == LYC) STAT |=   STAT_COINC_MASK;
  else          STAT &= (~STAT_COINC_MASK);

  // Handle PPU mode bits
  if(_state == State::STATE_MODE_0) STAT = (STAT & 0b11111100) | 0b00;
  if(_state == State::STATE_MODE_1) STAT = (STAT & 0b11111100) | 0b01;
  if(_state == State::STATE_MODE_2) STAT = (STAT & 0b11111100) | 0b10;
  if(_state == State::STATE_MODE_3) STAT = (STAT & 0b11111100) | 0b11;

}
