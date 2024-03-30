#ifndef __PPU_DEF_H
#define __PPU_DEF_H

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 144
#define SCALE_FACTOR  2

#define PPU_SIZE 10

#define OAM_BUFFER_SIZE_OBJ     10
#define OAM_BUFFER_SIZE_BYTE    OAM_BUFFER_SIZE_OBJ * 4
#define OAM_SIZE                OAM_BUFFER_SIZE_BYTE * 4

#define VBLANK_LINE_WAIT 456
#define VBLANK_PSEUDO_LINES 10

#define STAT_LYC_INTERRUPT_MASK     0b01000000
#define STAT_M2_INTERRUPT_MASK      0b00100000
#define STAT_M1_INTERRUPT_MASK      0b00010000
#define STAT_M0_INTERRUPT_MASK      0b00001000
#define STAT_COINC_MASK             0b00000100
#define STAT_PPU_MODE_MASK          0b00000011

#define PPU_LCDC_EN_MASK            0b10000000
#define PPU_LCDC_W_TILE_MAP_MASK    0b01000000
#define PPU_LCDC_W_DISP_EN_MASK     0b00100000
#define PPU_LCDC_T_DATA_SEL_MASK    0b00010000
#define PPU_LCDC_B_TILE_MAP_MASK    0b00001000
#define PPU_LCDC_SPRITE_SIZE_MASK   0b00000100
#define PPU_LCDC_SPRITE_ENABLE_MASK 0b00000010
#define PPU_LCDC_BW_ENABLE_MASK     0b00000001

#define PPU_SPRITE_PRIO_MASK            0b10000000
#define PPU_SPRITE_Y_FLIP_MASK          0b01000000
#define PPU_SPRITE_X_FLIP_MASK          0b00100000
#define PPU_SPRITE_PALETTE_NUMBER_MASK  0b00010000

#define PPU_PALETTE_WHITE 0xffffffff
#define PPU_PALETTE_LIGHT_GREY 0xa9a9a9ff
#define PPU_PALETTE_DARK_GREY 0x545454ff
#define PPU_PALETTE_BLACK 0x000000ff

#endif // !__PPU_DEF_H
