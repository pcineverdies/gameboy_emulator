#include "cartridge.h"
#include <iostream>
#include <filesystem>

/** Cartridge::read
    Read a data from a cartridge, referring to different
    methods depending on the type of used cartridge.

    @param addr uint16_t address to read
    @return uint8_t read byte
*/
uint8_t Cartridge::read(uint16_t addr){

  // Use the content of BROM_EN to know whether the BOOT ROM
  // should be used or not.
  if(addr < MMU_BOOT_SIZE and _using_boot_rom){
    if(_bus_to_read->read(MMU_BROM_EN_INIT_ADDR) != 0) _using_boot_rom = 0;
    return _BOOT_ROM[addr];
  }

  if(addr >= VRAM_INIT_ADDR and addr < VRAM_END_ADDR){
    return _VRAM[addr - VRAM_INIT_ADDR];
  }

  if     (MBC == MBC_ROM_ONLY)                    return rom_only_read(addr);
  else if(MBC >= MBC_1_INIT and MBC <= MBC_1_END) return MBC1_read(addr);
  else if(MBC >= MBC_3_INIT and MBC <= MBC_3_END) return MBC3_read(addr);
  else if(MBC >= MBC_5_INIT and MBC <= MBC_5_END) return MBC5_read(addr);

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
  else if(MBC >= MBC_5_INIT and MBC <= MBC_5_END) MBC5_write(addr, data);
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
  _using_boot_rom = 1;
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

  // Save file name
  _save_file_name = file_name + ".save";

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

  set_boot_rom();
  reset_save_data_ram();
}

/** Cartridge::set_boot_rom
    Set content of boot rom

*/
void Cartridge::set_boot_rom(){

  // Bootinx copyright free boot rom, thanks to hacktix: https://github.com/Hacktix/Bootix
  std::vector<uint8_t> dmg_boot_rom = {
    0x31, 0xfe, 0xff, 0x21, 0xff, 0x9f, 0xaf, 0x32, 0xcb, 0x7c, 0x20, 0xfa, 0x0e, 0x11, 0x21, 0x26,
    0xff, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0x32, 0xe2, 0x0c, 0x3e, 0x77, 0x32, 0xe2, 0x11,
    0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd, 0xb8, 0x00, 0x1a, 0xcb, 0x37, 0xcd, 0xb8, 0x00, 0x13,
    0x7b, 0xfe, 0x34, 0x20, 0xf0, 0x11, 0xcc, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20,
    0xf9, 0x21, 0x04, 0x99, 0x01, 0x0c, 0x01, 0xcd, 0xb1, 0x00, 0x3e, 0x19, 0x77, 0x21, 0x24, 0x99,
    0x0e, 0x0c, 0xcd, 0xb1, 0x00, 0x3e, 0x91, 0xe0, 0x40, 0x06, 0x10, 0x11, 0xd4, 0x00, 0x78, 0xe0,
    0x43, 0x05, 0x7b, 0xfe, 0xd8, 0x28, 0x04, 0x1a, 0xe0, 0x47, 0x13, 0x0e, 0x1c, 0xcd, 0xa7, 0x00,
    0xaf, 0x90, 0xe0, 0x43, 0x05, 0x0e, 0x1c, 0xcd, 0xa7, 0x00, 0xaf, 0xb0, 0x20, 0xe0, 0xe0, 0x43,
    0x3e, 0x83, 0xcd, 0x9f, 0x00, 0x0e, 0x27, 0xcd, 0xa7, 0x00, 0x3e, 0xc1, 0xcd, 0x9f, 0x00, 0x11,
    0x8a, 0x01, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x1b, 0x7a, 0xb3, 0x20, 0xf5, 0x18, 0x49, 0x0e,
    0x13, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xc9, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7,
    0xc9, 0x78, 0x22, 0x04, 0x0d, 0x20, 0xfa, 0xc9, 0x47, 0x0e, 0x04, 0xaf, 0xc5, 0xcb, 0x10, 0x17,
    0xc1, 0xcb, 0x10, 0x17, 0x0d, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0x3c, 0x42, 0xb9, 0xa5,
    0xb9, 0xa5, 0x42, 0x3c, 0x00, 0x54, 0xa8, 0xfc, 0x42, 0x4f, 0x4f, 0x54, 0x49, 0x58, 0x2e, 0x44,
    0x4d, 0x47, 0x20, 0x76, 0x31, 0x2e, 0x32, 0x00, 0x3e, 0xff, 0xc6, 0x01, 0x0b, 0x1e, 0xd8, 0x21,
    0x4d, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x01, 0xe0, 0x50,
  };

  _BOOT_ROM.resize(0x100);

  for(uint32_t i = 0; i < dmg_boot_rom.size(); i++)
    _BOOT_ROM[i] = dmg_boot_rom[i];

}

/** Cartridge::save_data_ram
    The content of the RAM is saved on the external file

*/
void Cartridge::save_data_ram(){

  std::ofstream stream(_save_file_name);

  // Reset access counter
  _ram_access_counter = 0;

  #ifdef __DEBUG
  printf("Content of RAM is being saved...\n");
  #endif

  if(stream.is_open()){
    for(auto& bank : _ram_banks){
      for(auto& byte : bank) stream << byte;
    }
  }
  else{
    throw std::invalid_argument("Save file " + this->name + " not opened correctly.");
  }
}

/** Cartridge::reset_save_data_ram
    Restore the content of the save file from memory, if it exists.
    The save file contains the content of the cartridge DRAM

*/
void Cartridge::reset_save_data_ram(){

  // Variables to handle reading
  std::ifstream stream(_save_file_name);
  char byte;

  // Internal variables
  uint64_t current_address = 0;
  uint32_t current_ram_bank = 0;

  // If it's the first time a game is run, or if the game does not have RAM, the
  // save file does not exist. In this case, we just return
  if(!std::filesystem::exists(_save_file_name)) return;

  // If the file is not of the expected size, then it might be correupted. In this case, we
  // do not use it.
  if(std::filesystem::file_size(_save_file_name) != _ram_bank_size * RAM_SIZE){
    std::cerr << "File " << _save_file_name << " does not have proper size and cannot be used. Corrupted?";
    return;
  }

  // Check if the file is properly opened
  if(stream.is_open()){

    // Read and store one byte at a time
    while(stream.get(byte)){
      current_ram_bank = current_address / RAM_SIZE;
      _ram_banks[current_ram_bank][current_address % RAM_SIZE] = byte;
      current_address++;
    }

  }
  else{
    throw std::invalid_argument("Save file " + this->name + " not opened correctly.");
  }

}
