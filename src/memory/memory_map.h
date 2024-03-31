#ifndef __MEMORY_MAP
#define __MEMORY_MAP

#define IE_ADDRESS 0xffff
#define IF_ADDRESS 0xff0f

#define IF_VBLANK 0b00000001
#define IF_LCD    0b00000010
#define IF_TIMER  0b00000100
#define IF_SERIAL 0b00001000
#define IF_JOYPAD 0b00010000

#define OAM_INIT_ADDR 0xFE00

#define PPU_BASE 0xff40
#define PPU_LCDC 0xff40
#define PPU_STAT 0xff41
#define PPU_SCY  0xff42
#define PPU_SCX  0xff43
#define PPU_LY   0xff44
#define PPU_LYC  0xff45
#define PPU_DMA  0xff46
#define PPU_BGP  0xff47
#define PPU_OBP0 0xff48
#define PPU_OBP1 0xff49
#define PPU_WY   0xff4a
#define PPU_WX   0xff4b

#define PPU_TILES_MAP_1 0x8000
#define PPU_TILES_MAP_0 0x9000

#define PPU_BG_MAP_1 0x9C00
#define PPU_BG_MAP_0 0x9800

#endif // !__MEMORY_MAP
