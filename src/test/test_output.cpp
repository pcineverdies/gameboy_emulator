#include "../bus/bus.h"
#include "../cpu/cpu.h"
#include "../cpu/registers.h"
#include "../memory/memory.h"
#include "../memory/memory_map.h"
#include "../memory/register.h"
#include "../IO/timer.h"
#include <cassert>
#include <cstdint>

#define BUS_FREQUENCY 8192
#define CPU_FREQUENCY BUS_FREQUENCY/4

int main() {


  Bus bus("Bus", 0, 0xffff, BUS_FREQUENCY);

  Memory ROM_00(  "ROM_00", 0,      0xFF00);
  Memory IO_LOW(  "IO_LOW", 0xFF00, 0x0004);
  Timer  timer(   "Timer",  0xFF04        );
  Memory IO_HIGH( "IO_LOW", 0xFF08, 0x0078);
  Memory HRAM(    "HRAM",   0xFF80, 0x0080);

  Cpu cpu("cpu", CPU_FREQUENCY);

  ROM_00.init_from_file(0x00, "ROM/gb-test-roms/instr_timing/instr_timing.gb");

  bus.add_to_bus(&ROM_00);
  bus.add_to_bus(&HRAM);
  bus.add_to_bus(&IO_HIGH);
  bus.add_to_bus(&IO_LOW);

  bus.add_to_bus(&cpu);

  bus.add_to_bus(&timer);
  timer.set_frequency(BUS_FREQUENCY);

  bus.write(0xff44, 0x90);

  while(1) bus.step(&bus);
}
