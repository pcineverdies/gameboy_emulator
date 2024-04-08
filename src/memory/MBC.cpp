#include "cartridge.h"

/** Cartridge::rom_only_read
    Read the corresponding value from the only available bank

    @param addr uint16_t address to read
    @return uint8_t read byte
*/
uint8_t Cartridge::rom_only_read(uint16_t addr){
  if(addr >= VRAM_INIT_ADDR) throw std::runtime_error("Address not available in ROM_ONLY cartridges");
  return _rom_banks[addr / ROM_SIZE][addr % ROM_SIZE];
}

/** Cartridge::rom_only_write
    Do nothing - writings are skipped

*/
void Cartridge::rom_only_write(uint16_t, uint8_t){ return; }

/** Cartridge::MBC1_read
    Perform different accesses depending on the provided address,
    with respect to the MBC1 cartridge specifications.

    @param addr uint16_t address to read
    @return uint8_t read byte
*/
uint8_t Cartridge::MBC1_read(uint16_t addr){

  // First bank or $10, $20... if banking mode 1 is used
  if(addr < ROM_B00_END_ADDR){
    if(_banking_mode == 0) return _rom_banks[0][addr];
    else                   return _rom_banks[(_current_ram << 5) % _rom_bank_size][addr % ROM_SIZE];
  }
  // Other banks
  else if(addr >= ROM_BNN_INIT_ADDR and addr < ROM_BNN_END_ADDR){
    return _rom_banks[((_current_ram << 5) + _current_rom) % _rom_bank_size][addr % ROM_SIZE];
  }
  // Ram reading if available
  else if(addr >= RAM_BNN_INIT_ADDR and addr < RAM_BNN_END_ADDR){
    if(!_ram_bank_size or !_is_ram_enabled) return 0xff;
    if(_banking_mode == 0) return _ram_banks[0           ][addr % RAM_SIZE];
    else                   return _ram_banks[_current_ram][addr % RAM_SIZE];
  }

  throw std::runtime_error("Address not available in MBC1 cartridges");

}

/** Cartridge::MBC1_write
    Perform different accesses depending on the provided address,
    with respect to the MBC1 cartridge specifications.

    @param addr uint16_t address to write
    @param data uint8_t  byte to write

*/
void Cartridge::MBC1_write(uint16_t addr, uint8_t data){

  // Enable ram
  if(addr < MBC_WRITE1_END_ADDR){
    if((data & 0x0f) == 0x0a){
      // printf("Enabling RAM\n");
      _is_ram_enabled = 1;
    }
    else{
      // printf("Disabling RAM\n");
      _is_ram_enabled = 0;
      save_data_ram();
    }
  }
  // Choose the rom to use
  if(addr >= MBC_WRITE2_INIT_ADDR and addr < MBC_WRITE2_END_ADDR){
    _current_rom = ((data & 0x1f) == 0) ? 1 : (data & 0x1f);
  }
  // Choose the ram to use
  if(addr >= MBC_WRITE3_INIT_ADDR and addr < MBC_WRITE2_END_ADDR){
    _current_ram = (data & 0x03);
  }
  // Write ram
  if(addr >= MBC_WRITE4_INIT_ADDR){
    if(!_ram_bank_size or !_is_ram_enabled) return;
    if(_banking_mode == 0) _ram_banks[0                            ][addr % RAM_SIZE] = data;
    else                   _ram_banks[_current_ram % _ram_bank_size][addr % RAM_SIZE] = data;
    if(++_ram_access_counter > RAM_ACCESS_COUNTER_MAX) save_data_ram();
  }

}

/** Cartridge::MBC3_read
    Perform different accesses depending on the provided address,
    with respect to the MBC3 cartridge specifications.

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t Cartridge::MBC3_read(uint16_t addr){

  // Bank 0 rom
  if(addr < ROM_B00_END_ADDR){
    return _rom_banks[0][addr];
  }
  // Bank N rom
  else if(addr >= ROM_BNN_INIT_ADDR and addr < ROM_BNN_END_ADDR){
    return _rom_banks[_current_rom % _rom_bank_size][addr % ROM_SIZE];
  }
  // Use ram or RTC
  else if(addr >= RAM_BNN_INIT_ADDR and addr < RAM_BNN_END_ADDR){
    if(!_is_ram_enabled){
      return 0xff;
    }
    if(_is_ram_enabled and _current_ram < MBC3_RAM_END_ADDR and _ram_bank_size == 0){
      return 0xff;
    }
    if(_current_ram < MBC3_RAM_END_ADDR){
      return _ram_banks[_current_ram][addr % RAM_SIZE];
    }
    else if(_current_ram >= MBC3_RTC_INIT_ADDR and _current_ram < MBC3_RTC_END_ADDR){
      return _RTC[_current_ram - MBC3_RTC_INIT_ADDR];
    }
    return 0xff;
  }

  throw std::runtime_error("Address not available in MBC3 cartridges");
}

/** Cartridge::MBC3_write
    Perform different accesses depending on the provided address,
    with respect to the MBC3 cartridge specifications.

    @param addr uint16_t address to write
    @param data uint8_t  byte to write

*/
void Cartridge::MBC3_write(uint16_t addr, uint8_t data){

  // Ram enable
  if(addr < MBC_WRITE1_END_ADDR){
    if((data & 0x0f) == 0x0a){
      _is_ram_enabled = 1;
    }
    else{
      _is_ram_enabled = 0;
      save_data_ram();
    }
  }
  // Pick current rom
  if(addr >= MBC_WRITE2_INIT_ADDR and addr < MBC_WRITE2_END_ADDR){
    _current_rom = ((data & 0x7f) == 0) ? 1 : (data & 0x7f);
  }
  // Pick current ram
  if(addr >= MBC_WRITE3_INIT_ADDR and addr < MBC_WRITE3_END_ADDR){
    _current_ram = data;
  }
  // Latch RTC if sequence of 0 -> 1 is written
  if(addr >= MBC_WRITE5_INIT_ADDR and addr < MBC_WRITE5_END_ADDR){
    if(data == 0x01 and _RTC_to_latch == 1) set_RTC();
    _RTC_to_latch = (data == 0x00) ? 1 : 0;
  }
  // Write ram
  if(addr >= MBC_WRITE4_INIT_ADDR){
    if(!_is_ram_enabled) return;
    if(_is_ram_enabled and _current_ram < MBC3_RAM_END_ADDR and _ram_bank_size == 0) return;
    if(_current_ram < MBC3_RAM_END_ADDR)_ram_banks[_current_ram % _ram_bank_size][addr % RAM_SIZE] = data;
    if(_current_ram >= MBC3_RTC_INIT_ADDR and _current_ram < MBC3_RTC_END_ADDR){
      _RTC[_current_ram - MBC3_RTC_INIT_ADDR] = data;
    }
    if(++_ram_access_counter > RAM_ACCESS_COUNTER_MAX) save_data_ram();
  }
}

/** Cartridge::set_RTC
    Set the current RTC

    TODO

*/
void Cartridge::set_RTC(){}

/** Cartridge::MBC5_read
    Perform different accesses depending on the provided address,
    with respect to the MBC5 cartridge specifications.

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t Cartridge::MBC5_read(uint16_t addr){

  uint16_t rom_to_use = _current_rom | (_current_rom_up << 8);

  // Bank 0 rom
  if(addr < ROM_B00_END_ADDR){
    return _rom_banks[0][addr];
  }
  // Bank N rom
  else if(addr >= ROM_BNN_INIT_ADDR and addr < ROM_BNN_END_ADDR){
    return _rom_banks[rom_to_use % _rom_bank_size][addr % ROM_SIZE];
  }
  // Use RAM (no RTC available in MBC5)
  else if(addr >= RAM_BNN_INIT_ADDR and addr < RAM_BNN_END_ADDR){
    if(!_is_ram_enabled){
      return 0xff;
    }
    if(_ram_bank_size == 0){
      return 0xff;
    }
    if(_current_ram < _ram_bank_size){
      return _ram_banks[_current_ram][addr % RAM_SIZE];
    }
    return 0xff;
  }

  throw std::runtime_error("Address not available in MBC5 cartridges");
}

/** Cartridge::MBC5_write
    Perform different accesses depending on the provided address,
    with respect to the MBC5 cartridge specifications.

    @param addr uint16_t address to write
    @param data uint8_t  byte to write

*/
void Cartridge::MBC5_write(uint16_t addr, uint8_t data){

  if(addr >= MBC5_WRITE1_INIT_ADDR and addr < MBC5_WRITE1_END_ADDR){
    if(data == 0x00){
      _is_ram_enabled = 0;
      save_data_ram();
    }
    else if(data == 0x0a){
      _is_ram_enabled = 1;
    }
  }
  if(addr >= MBC5_WRITE2_INIT_ADDR and addr < MBC5_WRITE2_END_ADDR){
    _current_rom = data;
  }
  if(addr >= MBC5_WRITE3_INIT_ADDR and addr < MBC5_WRITE3_END_ADDR){
    _current_rom_up = data & 0x01;
  }
  if(addr >= MBC5_WRITE4_INIT_ADDR and addr < MBC5_WRITE4_END_ADDR){
    _current_ram = data;
  }
  if(addr >= MBC_WRITE4_INIT_ADDR){
    if(!_is_ram_enabled) return;
    if(_is_ram_enabled and _ram_bank_size == 0) return;
    _ram_banks[_current_ram % _ram_bank_size][addr % RAM_SIZE] = data;
    if(++_ram_access_counter > RAM_ACCESS_COUNTER_MAX) save_data_ram();
  }
}

