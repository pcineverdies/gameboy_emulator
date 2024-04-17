#include "PPU.h"
#include "PPU_def.h"
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <chrono>

extern gb_global_t gb_global;

/** PPU::reset
    Sets the initial values of the PPU registers

*/
void PPU::reset(){

  LCDC  = 0x91;
  STAT  = 0x85;
  SCY   = 0;
  SCX   = 0;
  LY    = 0;
  LYC   = 0;
  DMA   = 0xff;
  BGP   = 0xfc;
  OBP0  = 0;
  OBP1  = 0;
  WX    = 0;
  WY    = 0;

  _DMA_bytes_to_transfer = 0;
  _DMA_cycles_to_wait = 0;

  _OAM_SCAN_to_wait = 0;
  _OAM_SCAN_fetched = 0;
  _OAM_SCAN_addr = 0;

  _DRAWING_window_condition = 0;
  _DRAWING_window_line_counter = 0;

  for(int i = 0; i < OAM_BUFFER_SIZE_BYTE; i++) _OAM_SCAN_buffer[i] = 0;

  _state = State::STATE_MODE_2;

}

/** PPU::DMA_OAM_step
    Performs a step of the DMA operation. Each transfer
    requires 1 M-cycle (== 4 T-cycles).

    @param bus Bus_obj* pointer to a bus to use for reading and writing

*/
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

/** PPU::OAM_SCAN_step
    Perform a step of OAM scan (PPU mode 2), reading at most
    10 objects which have to be displayed on the current line

    @param bus Bus_obj* pointer to a bus to use for reading and writing

*/
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
  obj_Y_pos =       bus->read(current_address);
  obj_X_pos =       bus->read(current_address + 1);
  obj_tile_number = bus->read(current_address + 2);
  obj_sprite_flag = bus->read(current_address + 3);

  // Increment next address to use
  _OAM_SCAN_addr += 4;

  // If all the conditions are valid, then the object is to be added to the OAM buffer
  if( _OAM_SCAN_fetched != OAM_BUFFER_SIZE_OBJ and obj_X_pos > 0 and
    LY + 16 >= obj_Y_pos and LY + 16 < obj_Y_pos + get_sprite_height()){
    _OAM_SCAN_buffer[_OAM_SCAN_fetched * 4    ] = obj_Y_pos;
    _OAM_SCAN_buffer[_OAM_SCAN_fetched * 4 + 1] = obj_X_pos;
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

    // Reset OAM variables
    _OAM_SCAN_to_wait = 0;
    _OAM_SCAN_addr = 0;

    // If LY == WY at least once in the frame, then the window
    // can be drawn
    if(LY == WY) _DRAWING_window_condition = 1;

    // Clock cycles to wait for the PPU in Mode 3
    _DRAWING_to_wait = PPU_DRAWING_TO_WAIT;

    _state = State::STATE_MODE_3;
  }
}

/** PPU::DRAWING_step
    Perform a step of DRAWING (PPU mode 3), displaying the content
    of the current scanline

    @param bus Bus_obj* pointer to a bus to use for reading and writing

*/
void PPU::DRAWING_step(Bus_obj* bus){

  // Select which maps to use for background and window
  uint16_t background_map_address = (LCDC & PPU_LCDC_B_TILE_MAP_MASK) ? PPU_BG_MAP_1 : PPU_BG_MAP_0 ;
  uint16_t window_map_address     = (LCDC & PPU_LCDC_W_TILE_MAP_MASK) ? PPU_BG_MAP_1 : PPU_BG_MAP_0 ;

  // Indeces of the tiles to read
  uint16_t tile_index_x;
  uint16_t tile_index_y;

  // Number of the tile combining tile_index_x and tile_index_y
  uint16_t tile_number;

  // Address of the tile to use
  uint16_t tile_address;

  // 16 bits of the proper line to use within the tile
  uint8_t upper_tile;
  uint8_t lower_tile;

  // Id of the color which will be used
  uint8_t color_id_to_use;

  // RRGGBBAA representation of the displayed color, computer through
  // the color ID and the palette
  uint32_t color_to_use;

  // Stores which ids have been used for the background/window
  uint8_t background_colors[SCREEN_WIDTH * SCREEN_HEIGHT];

  // Bit mask
  uint8_t mask;

  // If window is to be used on the current pixel, and if window was used
  // at least once in the frame
  uint8_t using_window;
  uint8_t used_window = 0;

  _DRAWING_to_wait--;

  if(_DRAWING_to_wait != 0) return;

  // Background and window drawing

  // For each pixel in the line
  for(int x = 0; x < SCREEN_WIDTH; x++){

    // Is the window to be used? Window must be enabled,
    // the condition LY == WY was encountered in the current frame
    // and window should be currently visible
    using_window =  ((LCDC & PPU_LCDC_W_DISP_EN_MASK) and
                     (_DRAWING_window_condition) and
                     (x >= WX - 7)) ? 1 : 0;

    // Store if window was used for this line
    if(using_window) used_window = 1;

    // Window:
    // Horizontal tile index is (x - WX + 7) / 8
    //
    // Bakground:
    // Horizontal tile index is (SCX + x) / 8
    // anded with 0x1f to allow horizontal tile scrolling
    tile_index_x =  (using_window) ?
                     (x - WX + 7) / 8      :
                    ((SCX + x) / 8) & 0x1f ;

    // Window:
    // Vertical tile index is _DRAWING_window_line_counter / 8
    //
    // Background:
    // Vertical tile index is (LY + y) / 8,
    // anded with 0xff to allow vertical tile scrolling
    tile_index_y =  (using_window) ?
                    (_DRAWING_window_line_counter / 8) :
                    ((LY + SCY) & 0xff) / 8;

    // The offset of the tile index can be obtained by x + y * 32
    tile_number = bus->read(tile_index_y * 32 +
                            tile_index_x +
                            ((using_window) ? window_map_address : background_map_address));

    // Tile address is computed in different ways depending on LCDC
    tile_address =  ((LCDC & PPU_LCDC_T_DATA_SEL_MASK)) ? PPU_TILES_MAP_1 +              tile_number * 16 :
                                                          PPU_TILES_MAP_0 + (signed char)tile_number * 16 ;

    // Each tile is made of 16 bytes, 2 per row, for a total of 8 rows.
    //
    // Window:
    // We need to pick the row (_DRAWING_window_line_counter) % 8
    //
    // Background:
    // We need to pick the row (LY + SCY) % 8
    tile_address += (using_window) ?
                    2 * (_DRAWING_window_line_counter % 8) :
                    2 * ((LY + SCY) % 8);

    // Get the two tiles
    lower_tile = cart->read_vram(0, tile_address);
    upper_tile = cart->read_vram(0, tile_address + 1);

    // No pixel scrolling for window. Since pixels are displayed from left to
    // right, we must consider elements from left to right as well.
    mask = (using_window) ? 1 << (7 - ((x - WX + 7) % 8)) : 1 << (7 - ((x + SCX) % 8));

    // Extract color to use
    color_id_to_use = 0;
    if(lower_tile & mask) color_id_to_use |= 0x01;
    if(upper_tile & mask) color_id_to_use |= 0x02;
    color_to_use = get_color_from_palette(color_id_to_use, BGP);

    // If background and window are disabled, white is displayed
    if(!(LCDC & PPU_LCDC_BW_ENABLE_MASK)) color_to_use = PPU_PALETTE_WHITE;

    // Stores color to be displayed
    _DRAWING_display_matrix[x + LY * SCREEN_WIDTH] = color_to_use;

    // Stores id of the used color, in order to handle the priority of the sprites
    background_colors[x + LY * SCREEN_WIDTH]       = color_id_to_use;
  }

  // Objects drawing
  for(int x = 0; x < SCREEN_WIDTH; x++){

    // Break if sprites are disabled
    if(!(LCDC & PPU_LCDC_SPRITE_ENABLE_MASK)) break;

    // 16 or 8, depending on LCDC
    uint8_t obj_height = get_sprite_height();

    // A sprite with lower x coordinate has priority over a
    // sprite with higher x coordinate. By picking a sprite iff
    // no other with lower x was used, we are sure to respect this rule
    uint8_t last_x_coordinate = 0xff;

    // Consider all the objects
    for(int k = 0; k < 4 * _OAM_SCAN_fetched; k += 4){

      // Read data from OAM buffer
      uint8_t obj_y_pos       = _OAM_SCAN_buffer[k    ];
      uint8_t obj_x_pos       = _OAM_SCAN_buffer[k + 1];
      uint8_t obj_tile_number = _OAM_SCAN_buffer[k + 2];
      uint8_t obj_flags       = _OAM_SCAN_buffer[k + 3];

      // which palette to be used
      uint8_t palette_to_use;

      // object is not in the current pixel: skip object
      if(obj_x_pos > (x + 8) or (obj_x_pos + 8) <= x + 8) continue;

      // priority is 1 and id of the background was different from 0: skip object
      if(background_colors[x + LY * SCREEN_WIDTH] != 0 and (obj_flags & PPU_SPRITE_PRIO_MASK)) continue;

      // Tile map is always fixed for sprites
      tile_address = PPU_TILES_MAP_1;

      // 8 bits sprite and no Y flip
      if(obj_height == 8 and !(obj_flags & PPU_SPRITE_Y_FLIP_MASK))
        tile_address += obj_tile_number * 16 + 2 * (LY - obj_y_pos + 16);

      // 8 bits sprite and Y flip
      else if(obj_height == 8 and (obj_flags & PPU_SPRITE_Y_FLIP_MASK))
        tile_address += obj_tile_number * 16 + 2 * ( 7  - (LY - obj_y_pos + 16));

      // 16 bits sprite and no Y flip: start from tile number with msb reset
      else if(obj_height == 16 and !(obj_flags & PPU_SPRITE_Y_FLIP_MASK))
        tile_address += (obj_tile_number & 0xfffe) * 16 + 2 * ((LY - obj_y_pos + 16));

      // 16 bits sprite and Y flip: start from tile number with msb reset
      else
        tile_address += (obj_tile_number & 0xfffe) * 16 + 2 * ( 15 - (LY - obj_y_pos + 16));

      // Get tile
      lower_tile = cart->read_vram(0, tile_address);
      upper_tile = cart->read_vram(0, tile_address + 1);

      // Handle X flip
      mask = (obj_flags & PPU_SPRITE_X_FLIP_MASK) ? 1 << (x - obj_x_pos + 8)  : 1 << (7 - (x - obj_x_pos + 8));

      // Get color id to use
      color_id_to_use = 0;
      if(lower_tile & mask) color_id_to_use |= 0x01;
      if(upper_tile & mask) color_id_to_use |= 0x02;

      // an object with lower x coordinate was already drawn: skip object
      if(last_x_coordinate <= obj_x_pos or color_id_to_use == 0) continue;
      else last_x_coordinate = obj_x_pos;

      // Extract color to use
      palette_to_use = (obj_flags & PPU_SPRITE_PALETTE_NUMBER_MASK) ? OBP1 : OBP0;
      color_to_use = get_color_from_palette(color_id_to_use, palette_to_use);

      // Do not draw if color id is 0
      if(color_id_to_use != 0)
        _DRAWING_display_matrix[x + LY * SCREEN_WIDTH] = color_to_use;
    }
  }

  // Incremente the internal window counter if window was used in the current line
  if(used_window) _DRAWING_window_line_counter++;

  // Move to HBLANK
  _state = State::STATE_MODE_0;
  _HBLANK_padding_to_wait = 284;
}

/** PPU::HBLANK_step
    Perform a step of HBLANK (PPU mode 0), waiting for the next scanline

    @param bus Bus_obj* pointer to a bus to use for reading and writing

*/
void PPU::HBLANK_step(Bus_obj* bus){

  // Handle HBLANK as busy waiting until 456 per scanline have passed
  _HBLANK_padding_to_wait--;

  // If this condition is not true, we move either to the
  // next scanline or to VBLANK
  if(_HBLANK_padding_to_wait != 0) return;

  // Next scanline
  LY++;

  // Enter VBLANK
  if(LY == SCREEN_HEIGHT){
    set_vblank_interrupt(bus);
    _state = State::STATE_MODE_1;
    _DRAWING_window_condition = 0;
    _DRAWING_window_line_counter = 0;
    _VBLANK_padding_to_wait = VBLANK_LINE_WAIT;
  }
  // Enter OAM
  else{
    _state = State::STATE_MODE_2;
    _OAM_SCAN_to_wait = 0;
    _OAM_SCAN_fetched = 0;
    _OAM_SCAN_addr = 0;
  }

  return;
}


/** PPU::VBLANK_step
    Perform a step of VBLANK (PPU mode 0), waiting for the next frame

    @param bus Bus_obj* pointer to a bus to use for reading and writing

*/
void PPU::VBLANK_step(Bus_obj*){

  #ifdef __DEBUG
  // FPS counter variables
  static int counter = 0;
  static auto start = std::chrono::system_clock::now();
  #endif

  _VBLANK_padding_to_wait--;

  // Pseudo-scanline still to go
  if(_VBLANK_padding_to_wait != 0) return;

  // Next pseudo-scanline
  LY++;

  // Padding for next pseudo-scanline
  _VBLANK_padding_to_wait = VBLANK_LINE_WAIT;

  // If 10 pseudo-scanlines have been handled, move to new frame
  if(LY == SCREEN_HEIGHT + VBLANK_PSEUDO_LINES){

    // Update the screen with the current frame
    display->update(_DRAWING_display_matrix);

    #ifdef __DEBUG
    // FPS counting
    if(++counter % 60 == 0){
      auto elapsed = std::chrono::system_clock::now() - start;
      std::cout << "FPS\t" << 60*((float)(1000000000)/(elapsed.count())) << '\n';
      start =  std::chrono::system_clock::now();
    }
    #endif

    _state = State::STATE_MODE_2;
    LY = 0;
  }
}

/** PPU::is_PPU_on
    Use LCDC to check if PPU is on

    @return bool true if PPU is on

*/
bool PPU::is_PPU_on(){
  return (LCDC & PPU_LCDC_EN_MASK) ? true : false;
}

/** PPU::get_sprite_height
    Use LCDC to determine the height of the sprites

    @return uint8_t 8 or 16, depending on LCDC

*/
uint8_t PPU::get_sprite_height(){
  return (LCDC & PPU_LCDC_SPRITE_SIZE_MASK) ? 16 : 8;
}

/** PPU::STAT_handler
    At each PPU step, updates STAT and possibly raise an
    interrupt

    @param bus Bus_obj* pointer to a bus to use for reading and writing

*/
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

/** PPU::get_color_from_palette
    Given a tile and a palette, extract the RRGGBBAA format of the color
    to be used on the screen

    @param tile uint8_t tile to be used
    @param palette uint8_t palette to be used

*/
uint32_t PPU::get_color_from_palette(uint8_t tile, uint8_t palette){

  uint8_t index = (tile == 0) ? (palette & 0b00000011)      :
                  (tile == 1) ? (palette & 0b00001100) >> 2 :
                  (tile == 2) ? (palette & 0b00110000) >> 4 :
                                (palette & 0b11000000) >> 6 ;

  return  (index == 0) ? PPU_PALETTE_WHITE      :
          (index == 1) ? PPU_PALETTE_LIGHT_GREY :
          (index == 2) ? PPU_PALETTE_DARK_GREY  :
                         PPU_PALETTE_BLACK;

}
