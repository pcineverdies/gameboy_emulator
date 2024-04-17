#include "CRAM.h"

/** CRAM::read
    Read by from CRAM at a given address

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t CRAM::read(uint16_t addr){
  uint8_t res = 0;
  uint8_t palette_addr = 0;

  if(addr >= size_addr)
    throw std::invalid_argument( "Address of provided to WRAM over the limit\n" );

  if     (addr == 0){
    res = BCPS;
  }
  else if(addr == 1){
    palette_addr = BCPS & 0b00111111;
    res = background_palette[palette_addr];
  }
  else if(addr == 2){
    res = OCPS;
  }
  else if(addr == 3){
    palette_addr = OCPS & 0b00111111;
    res = object_palette[palette_addr];
  }

  return res;
}

/** CRAM::write
    Write a byte to a given address

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void CRAM::write(uint16_t addr, uint8_t data){
  uint8_t palette_addr = 0;

  if(addr >= size_addr)
    throw std::invalid_argument( "Address of provided to WRAM over the limit\n" );

  if     (addr == 0){
    BCPS = data;
  }
  else if(addr == 1){
    palette_addr = BCPS & 0b00111111;
    background_palette[palette_addr] = data;
    if(BCPS & 0x80) BCPS = 0x80 | ((palette_addr + 1) % 64);
  }
  else if(addr == 2){
    OCPS = data;
  }
  else if(addr == 3){
    palette_addr = OCPS & 0b00111111;
    object_palette[palette_addr] = data;
    if(OCPS & 0x80) OCPS = 0x80 | ((palette_addr + 1) % 64);
  }
}

/** CRAM::CRAM
    CRAM constructor. Sets working frequency to 0.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus
    @param size uint16_t Size of the addressable space of the object

*/
CRAM::CRAM(std::string name, uint16_t init_addr, uint16_t size) : Bus_obj(name, init_addr, size){
  this->set_frequency(0);

  background_palette.resize(64);
  object_palette.resize(64);

  BCPS = 0;
  OCPS = 0;
}

/** CRAM::read_color_palette
    Return the color number [color_number] from palette number [palette_number],
    stored in format RGB555, for the [target] set of colors

    @param target uint8_t address bg or obj palette
    @param palette_number uint8_t Palette number to use
    @param color_number uint8_t Color number to use
    @return uint16_t color in format RGB555

*/
uint16_t CRAM::read_color_palette(uint8_t target, uint8_t palette_number, uint8_t color_number){
  uint8_t lower_byte;
  uint8_t upper_byte;
  uint16_t res_cpy  = 0;
  uint16_t res      = 0;
  uint8_t counter   = 15;

  if(target == CRAM_OBJ_PALETTE){
    lower_byte = object_palette[palette_number * 8 + color_number * 2    ];
    upper_byte = object_palette[palette_number * 8 + color_number * 2 + 1];
  }
  else{
    lower_byte = background_palette[palette_number * 8 + color_number * 2    ];
    upper_byte = background_palette[palette_number * 8 + color_number * 2 + 1];
  }

  // The color is in format RGB555 little-endian. We need to modify it in
  // RGB555 big-endian in order for SDL to use it
  res_cpy = upper_byte << 8 | lower_byte;

  // Reverse order of bits in the number
  while(counter != 0xff){
    if(res_cpy & (1 << counter)) res |= (1 << (15 - counter));
    counter--;
  }

  return res >> 1;
}

