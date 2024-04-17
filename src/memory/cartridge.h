#ifndef __CARTRIDGE_H
#define __CARTRIDGE_H

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <fstream>
#include "../bus/bus_obj.h"
#include "memory_map.h"
#include "../utils/gb_global_t.h"
#include "../memory/memory.h"

// How many writings to perform on cartidge ram
// before saving the content to disc
#define RAM_ACCESS_COUNTER_MAX 500000

class Cartridge : public Bus_obj  {

    uint8_t  MBC;
    uint16_t _rom_bank_size;
    uint16_t _ram_bank_size;

    uint8_t _current_rom;
    uint8_t _current_rom_up;
    uint8_t _current_ram;
    uint8_t _is_ram_enabled;
    uint8_t _banking_mode;

    uint8_t _RTC[RTC_SIZE];
    uint8_t _RTC_to_latch;

    uint8_t _using_boot_rom;

    std::string _save_file_name;
    uint32_t _ram_access_counter;

    std::vector<std::vector<uint8_t>> _rom_banks;
    std::vector<std::vector<uint8_t>> _ram_banks;

    std::vector<uint8_t> _VRAM_0;
    std::vector<uint8_t> _VRAM_1;
    std::vector<uint8_t> _BOOT_ROM;

    uint8_t rom_only_read(uint16_t);
    void    rom_only_write(uint16_t, uint8_t);
    uint8_t MBC1_read(uint16_t);
    void    MBC1_write(uint16_t, uint8_t);
    uint8_t MBC3_read(uint16_t);
    void    MBC3_write(uint16_t, uint8_t);
    uint8_t MBC5_read(uint16_t);
    void    MBC5_write(uint16_t, uint8_t);
    void    set_RTC();
    void    set_boot_rom();
    void    save_data_ram();
    void    reset_save_data_ram();

public:

  // Use a reference to the bus to perform the reading
  // of the brom_en register.
  Bus_obj* _bus_to_read;

            Cartridge(std::string, uint16_t, uint16_t);
  uint8_t   read(uint16_t);
  void      write(uint16_t, uint8_t);
  void      step(Bus_obj*){}
  void      init_from_file(std::string);
  uint8_t   read_vram(uint8_t, uint16_t);
            ~Cartridge(){}
};

#endif // !__CARTRIDGE_H
