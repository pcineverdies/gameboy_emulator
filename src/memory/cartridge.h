#ifndef __CARTRIDGE_H
#define __CARTRIDGE_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <fstream>
#include "../bus/bus_obj.h"

#define ROM_SIZE (1 << 14)
#define RAM_SIZE (1 << 13)
#define RTC_SIZE 5

#define MBC_ROM_ONLY 0
#define MBC_1_INIT   0x01
#define MBC_1_END    0x03
#define MBC_3_INIT   0x0f
#define MBC_3_END    0x13

class Cartridge : public Bus_obj  {

    uint8_t  MBC;
    uint16_t _rom_bank_size;
    uint16_t _ram_bank_size;

    uint8_t _current_rom;
    uint8_t _current_ram;
    uint8_t _is_ram_enabled;
    uint8_t _banking_mode;

    uint8_t _RTC[RTC_SIZE];
    uint8_t _RTC_to_latch;

    std::vector<std::vector<uint8_t>> _rom_banks;
    std::vector<std::vector<uint8_t>> _ram_banks;

    std::vector<uint8_t> _VRAM;

    uint8_t rom_only_read(uint16_t);
    void    rom_only_write(uint16_t, uint8_t);
    uint8_t MBC1_read(uint16_t);
    void    MBC1_write(uint16_t, uint8_t);
    uint8_t MBC3_read(uint16_t);
    void    MBC3_write(uint16_t, uint8_t);
    void    set_RTC();

public:
            Cartridge(std::string, uint16_t, uint16_t);
  uint8_t   read(uint16_t);
  void      write(uint16_t, uint8_t);
  void      step(Bus_obj*){}
  void      init_from_file(std::string);
            ~Cartridge(){}
};

#endif // !__CARTRIDGE_H
