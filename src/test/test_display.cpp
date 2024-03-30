#include "../bus/bus.h"
#include "../cpu/cpu.h"
#include "../memory/memory.h"
#include "../memory/register.h"
#include "../IO/timer.h"
#include "../IO/serial.h"
#include "../IO/joypad.h"
#include "../PPU/PPU.h"

#define BUS_FREQUENCY     4194304
#define CPU_FREQUENCY     BUS_FREQUENCY/4
#define SERIAL_FREQUENCY  1
#define JOYPAD_FREQUENCY  256
#define IS_ROM 1

int main(int argc, char* argv[]) {

  if(argc != 2){
    throw std::invalid_argument("./gameboy ./path/to/ROM");
  }

  Bus bus("Bus", 0, 0xffff, BUS_FREQUENCY);

  Memory    rom_00(  "ROM_00",  0,      0x8000, IS_ROM);
  Memory    vram(    "VRAM",    0x8000, 0x2000);
  Memory    ext_ram( "EXT_RAM", 0xA000, 0x2000);
  Memory    wram(    "WRAM",    0xC000, 0x2000);
  // Emtpy region between 0xE000 and 0xFDFF
  Memory    oam(     "OAM",     0xFE00, 0x00A0);
  // Emtpy region between 0xE000 and 0xFDFF
  Joypad    joypad(  "JOYPAD",  0xFF00        ); // size is 1
  Serial    serial(  "SERIAL",  0xFF01        ); // size is 2
  // 0xff03 is emtpy
  Timer     timer(   "TIMER",   0xFF04        ); // size is 4
  // Empty between 0xFF08 and 0xFF0D
  Register  if_reg(  "IF_REG",  0xFF0F);
  // Audio between 0xFF10 and 0xFF3F
  PPU       ppu(     "PPU",     0xFF40        ); // size is 12
  Register  brom_en( "BROM_EN", 0xFF50        ); // size is 1
  // Emtpy region between 0xFF51 and 0xFF7F
  Memory    hram(    "HRAM",    0xFF80, 0x007F);
  Register  ie_ref(  "IE_REG",  0xFFFF);

  Cpu cpu("cpu", CPU_FREQUENCY);

  rom_00.init_from_file(0x00, argv[1]);

  bus.add_to_bus(&rom_00);
  bus.add_to_bus(&vram);
  bus.add_to_bus(&ext_ram);
  bus.add_to_bus(&wram);
  bus.add_to_bus(&oam);
  bus.add_to_bus(&joypad);
  bus.add_to_bus(&serial);
  bus.add_to_bus(&timer);
  bus.add_to_bus(&if_reg);
  bus.add_to_bus(&ppu);
  bus.add_to_bus(&brom_en);
  bus.add_to_bus(&hram);
  bus.add_to_bus(&ie_ref);
  bus.add_to_bus(&cpu);

  timer.set_frequency(BUS_FREQUENCY);
  serial.set_frequency(BUS_FREQUENCY);
  ppu.set_frequency(BUS_FREQUENCY);
  joypad.set_frequency(JOYPAD_FREQUENCY);

  while(1)
    bus.step(&bus);
}
