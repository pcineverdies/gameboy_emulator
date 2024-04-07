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

#define VRAM_INIT_ADDR 0x8000
#define VRAM_END_ADDR  0xA000

#define ROM_B00_INIT_ADDR 0x0000
#define ROM_B00_END_ADDR 0x4000
#define ROM_BNN_INIT_ADDR 0x4000
#define ROM_BNN_END_ADDR 0x8000
#define RAM_BNN_INIT_ADDR 0xA000
#define RAM_BNN_END_ADDR 0xC000

#define MBC_HEADER_ADDR       0x147
#define ROM_SIZE_HEADER_ADDR  0x148
#define RAM_SIZE_HEADER_ADDR  0x149

#define MBC_WRITE1_INIT_ADDR  0x0000
#define MBC_WRITE1_END_ADDR   0x2000

#define MBC_WRITE2_INIT_ADDR  0x2000
#define MBC_WRITE2_END_ADDR   0x4000

#define MBC_WRITE3_INIT_ADDR  0x4000
#define MBC_WRITE3_END_ADDR   0x6000

#define MBC_WRITE5_INIT_ADDR  0x6000
#define MBC_WRITE5_END_ADDR   0x8000

#define MBC_WRITE4_INIT_ADDR  0xa000
#define MBC_WRITE4_END_ADDR   0xc000

#define MBC3_RAM_END_ADDR 0x04
#define MBC3_RTC_INIT_ADDR 0x08
#define MBC3_RTC_END_ADDR  0x0d

#define ROM_SIZE (1 << 14)
#define RAM_SIZE (1 << 13)
#define RTC_SIZE 5

#define MBC_ROM_ONLY 0
#define MBC_1_INIT   0x01
#define MBC_1_END    0x03
#define MBC_3_INIT   0x0f
#define MBC_3_END    0x13

#define MMU_BOOT_INIT_ADDR      0x0000
#define MMU_BOOT_SIZE           0x0100
#define MMU_CART_INIT_ADDR      0x0000
#define MMU_CART_SIZE           0xC000
#define MMU_WRAM_INIT_ADDR      0xC000
#define MMU_WRAM_SIZE           0x2000
#define MMU_OAM_INIT_ADDR       0xFE00
#define MMU_OAM_SIZE            0x00A0
#define MMU_JOYPAD_INIT_ADDR    0xFF00
#define MMU_SERIAL_INIT_ADDR    0xFF01
#define MMU_TIMER_INIT_ADDR     0xFF04
#define MMU_IF_REG_INIT_ADDR    0xFF0F
#define MMU_IF_REG_INIT_VAL     0xE1
#define MMU_PPU_INIT_ADDR       0xFF40
#define MMU_BROM_EN_INIT_ADDR   0xFF50
#define MMU_HRAM_INIT_ADDR      0xFF80
#define MMU_HRAM_SIZE           0x007F
#define MMU_IE_REG_INIT_ADDR    0xFFFF
#define MMU_IE_REG_INIT_VAL     0x00


#endif // !__MEMORY_MAP
