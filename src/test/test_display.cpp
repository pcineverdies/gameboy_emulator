#include "../bus/bus.h"
#include "../cpu/cpu.h"
#include "../cpu/registers.h"
#include "../memory/memory.h"
#include "../memory/memory_map.h"
#include "../memory/register.h"
#include "../IO/timer.h"
#include "../IO/serial.h"
#include "../IO/joypad.h"
#include "../PPU/PPU.h"
#include <cassert>
#include <cstdint>

#define BUS_FREQUENCY 8192
#define CPU_FREQUENCY BUS_FREQUENCY/4

int main() {


  Bus bus("Bus", 0, 0xffff, BUS_FREQUENCY);

  Memory ROM_00(  "ROM_00",  0,      0x8000);
  Memory VRAM(    "VRAM",    0x8000, 0x2000);
  Memory RAM_H(   "RAM_H",   0xA000, 0x5F00);
  Joypad joypad(  "JOYPAD",  0xFF00        );
  Serial serial(  "SERIAL",  0xFF01        );
  PPU    ppu(     "PPU",     0xFF03        );
  Timer  timer(   "TIMER",   0xFF04        );
  Memory IO_HIGH( "IO_HIGH", 0xFF08, 0x0078);
  Memory HRAM(    "HRAM",    0xFF80, 0x0080);

  Cpu cpu("cpu", CPU_FREQUENCY);

  ROM_00.init_from_file(0x00, "ROM/gb-test-roms/cpu_instrs/individual/01-special.gb");

  bus.add_to_bus(&ROM_00);
  bus.add_to_bus(&HRAM);
  bus.add_to_bus(&VRAM);
  bus.add_to_bus(&RAM_H);
  bus.add_to_bus(&IO_HIGH);

  bus.add_to_bus(&cpu);

  bus.add_to_bus(&timer);
  timer.set_frequency(BUS_FREQUENCY);

  bus.add_to_bus(&serial);
  serial.set_frequency(BUS_FREQUENCY);

  bus.add_to_bus(&joypad);
  joypad.set_frequency(BUS_FREQUENCY / 4096);

  bus.add_to_bus(&ppu);
  ppu.set_frequency(BUS_FREQUENCY);

  bus.write(0xff44, 0x90);

  while(1)
    bus.step(&bus);
}
