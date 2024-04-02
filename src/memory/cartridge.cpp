#include "cartridge.h"
#include "memory_map.h"

/** Cartridge::read
    Read a data from a cartridge, referring to different
    methods depending on the type of used cartridge.

    @param addr uint16_t address to read
    @return uint8_t read byte
*/
uint8_t Cartridge::read(uint16_t addr){

  if(addr >= VRAM_INIT_ADDR and addr < VRAM_END_ADDR){
    return _VRAM[addr - VRAM_INIT_ADDR];
  }

  if     (MBC == MBC_ROM_ONLY)                    return rom_only_read(addr);
  else if(MBC >= MBC_1_INIT and MBC <= MBC_1_END) return MBC1_read(addr);
  else if(MBC >= MBC_3_INIT and MBC <= MBC_3_END) return MBC3_read(addr);

  else throw std::runtime_error(
    "The current MBC is not supported by the emulator"
  );
}

/** Cartridge::write
    Write a data to a cartridge, referring to different
    methods depending on the type of used cartridge.

    @param addr uint16_t address to use
    @param data uint8_t  byte to write
*/
void Cartridge::write(uint16_t addr, uint8_t data){

  if(addr >= VRAM_INIT_ADDR and addr < VRAM_END_ADDR){
    _VRAM[addr - VRAM_INIT_ADDR] = data;
    return;
  }

  if     (MBC == MBC_ROM_ONLY)                    rom_only_write(addr, data);
  else if(MBC >= MBC_1_INIT and MBC <= MBC_1_END) MBC1_write(addr, data);
  else if(MBC >= MBC_3_INIT and MBC <= MBC_3_END) MBC3_write(addr, data);
  else throw std::runtime_error(
    "The current MBC is not supported by the emulator"
  );
}

/** Cartridge::Cartridge
    Cartridge constructor

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus
    @param size uint16_t Size of the addressable space of the object

*/
Cartridge::Cartridge(std::string name, uint16_t init_addr, uint16_t size) : Bus_obj(name, init_addr, size){
  _VRAM.resize(RAM_SIZE);
  _is_ram_enabled = 0;
  _banking_mode = 0;
  _current_rom = 1;
  _RTC_to_latch = 0;
}

/** Cartridge::init_from_file
    Initialize the content of the cartridge using a file.

    @param init_addr uint16_t Initial address in which the file is moved
    @param file_name std::string Name of the file to use

*/
void Cartridge::init_from_file(std::string file_name){

  // Variables to handle reading
  std::ifstream file;
  std::ifstream stream(file_name);
  char byte;

  // Internal variables
  uint64_t current_address = 0;
  uint32_t current_rom_bank = 0;

  if(stream.is_open()){

    while(stream.get(byte)){
      current_rom_bank = current_address / ROM_SIZE;

      // Add a new bank of rom if necessary
      if(_rom_banks.size() == current_rom_bank){
        _rom_banks.push_back(std::vector<uint8_t>(ROM_SIZE));
      }

      // Read from file and store in the current address of the current bank
      _rom_banks[current_rom_bank][current_address % ROM_SIZE] = byte;

      // Write the MBC value if address is reached
      if(current_address == MBC_HEADER_ADDR){
        MBC = byte;
      }

      // Write the rom bank size value if address is reached
      if(current_address == ROM_SIZE_HEADER_ADDR){
        _rom_bank_size = (1 << (byte + 1));
      }

      // Write the ram bank size value if address is reached
      if(current_address == RAM_SIZE_HEADER_ADDR){
        _ram_bank_size = (byte == 0) ? 0  :
                         (byte == 2) ? 1  :
                         (byte == 3) ? 4  :
                         (byte == 4) ? 16 :
                                       8  ;
      }

      current_address++;
    }

  }
  else{
    throw std::invalid_argument(
      "File for " + this->name + " not opened correctly.\nThe path might be wrong!"
    );
  }

  // Double check of the used rom banks with respect to what was read from the header
  if(current_rom_bank + 1 != _rom_bank_size){
    throw std::runtime_error("Mismatch between rom banks used and read\n");
  }

  // Create ram banks
  for(int i = 0; i < _ram_bank_size; i++)
    _ram_banks.push_back(std::vector<uint8_t>(RAM_SIZE));
}

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
    _is_ram_enabled = ((data & 0xf) == 0x0a) ? 1 : 0;
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
    _is_ram_enabled = ((data & 0xf) == 0x0a) ? 1 : 0;
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
    if(_current_ram < MBC3_RAM_END_ADDR) _ram_banks[_current_ram % _ram_bank_size][addr % RAM_SIZE] = data;
    if(_current_ram >= MBC3_RTC_INIT_ADDR and _current_ram < MBC3_RTC_END_ADDR){
      _RTC[_current_ram - MBC3_RTC_INIT_ADDR] = data;
    }
  }
}

/** Cartridge::set_RTC
    Set the current RTC

    TODO

*/
void Cartridge::set_RTC(){}
