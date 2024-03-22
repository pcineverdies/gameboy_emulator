#include "../bus/bus.h"
#include "../memory/memory.h"
#include "../cpu/cpu.h"
#include "../cpu/registers.h"
#include <cassert>

int main(){

  {

    // TEST 1
    Bus    bus("Bus", 0, 0xffff, 8000);
    Memory ROM_00("ROM_00", 0, 0x1000);
    Memory HRAM("ROM_00", 0xff00, 0x0100);
    Cpu    cpu("cpu", 2000);
    bus.add_to_bus(&ROM_00);
    bus.add_to_bus(&HRAM);

    bus.write(0x100, 0x06);
    bus.write(0x101, 0xfa);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_B() == 0xfa);

    bus.write(0x102, 0xcb);
    bus.write(0x103, 0x30);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_B() == 0xaf);

    bus.write(0x104, 0xc3);
    bus.write(0x105, 0x00);
    bus.write(0x106, 0x02);
    cpu.step(&bus);
    cpu.step(&bus);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().PC == 0x0200);

    bus.write(0x200, 0x21);
    bus.write(0x201, 0x30);
    bus.write(0x202, 0x30);
    cpu.step(&bus);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_HL() == 0x3030);

    bus.write(0x203, 0x2b);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_HL() == 0x302f);

    bus.write(0xff0a, 0xee);

    bus.write(0x204, 0x0e);
    bus.write(0x205, 0x0a);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_C() == 0x0a);

    bus.write(0x206, 0xf2);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0xee);

    printf("-> TEST1 terminated succesfully\n");

  }

  {

    // TEST 2
    Bus    bus("Bus", 0, 0xffff, 8000);
    Memory ROM_00("ROM_00", 0, 0x1000);
    Memory HRAM("ROM_00", 0xff00, 0x0100);
    Cpu    cpu("cpu", 2000);
    bus.add_to_bus(&ROM_00);
    bus.add_to_bus(&HRAM);

    bus.write(0x100, 0x1e);
    bus.write(0x101, 0xf0);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_E() == 0xf0);

    bus.write(0x102, 0x1c);
    cpu.step(&bus);

    assert(cpu.get_registers().read_E() == 0xf1);

    bus.write(0x103, 0x1d);
    cpu.step(&bus);

    assert(cpu.get_registers().read_E() == 0xf0);

    bus.write(0x104, 0x35);
    cpu.step(&bus);
    cpu.step(&bus);
    cpu.step(&bus);
    assert(bus.read(cpu.get_registers().read_HL()) == 0xff);

    bus.write(0x105, 0x2f);
    cpu.step(&bus);
    assert(cpu.get_registers().read_A() == 0xff);

    bus.write(0x106, 0x3e);
    bus.write(0x107, 0x2f);
    cpu.step(&bus);
    cpu.step(&bus);
    assert(cpu.get_registers().read_A() == 0x2f);

    bus.write(0x108, 0x07);
    cpu.step(&bus);
    assert(cpu.get_registers().read_A() == 0b01011110);

    printf("-> TEST2 terminated succesfully\n");
  }

  {

    // TEST 3
    Bus    bus("Bus", 0, 0xffff, 8000);
    Memory ROM_00("ROM_00", 0, 0x1000);
    Memory HRAM("ROM_00", 0xff00, 0x0100);
    Cpu    cpu("cpu", 2000);
    bus.add_to_bus(&ROM_00);
    bus.add_to_bus(&HRAM);

    bus.write(0x100, 0x3e);
    bus.write(0x101, 0xca);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0xca);

    bus.write(0x102, 0x0e);
    bus.write(0x103, 0xf0);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_C() == 0xf0);

    bus.write(0x104, 0xB9);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0xca);
    assert(cpu.get_registers().get_Z() == 0);
    assert(cpu.get_registers().get_N() == 1);
    assert(cpu.get_registers().get_H() == 0);
    assert(cpu.get_registers().get_C() == 1);

    bus.write(0x105, 0xd6);
    bus.write(0x106, 0xff);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0xcb);
    assert(cpu.get_registers().get_Z() == 0);
    assert(cpu.get_registers().get_N() == 1);
    assert(cpu.get_registers().get_H() == 1);
    assert(cpu.get_registers().get_C() == 1);

    bus.write(0x107, 0xd6);
    bus.write(0x108, 0xcb);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0x00);
    assert(cpu.get_registers().get_Z() == 1);
    assert(cpu.get_registers().get_N() == 1);
    assert(cpu.get_registers().get_H() == 0);
    assert(cpu.get_registers().get_C() == 0);

    printf("-> TEST3 terminated succesfully\n");
  }

  {

    // TEST 4
    Bus    bus("Bus", 0, 0xffff, 8000);
    Memory ROM_00("ROM_00", 0, 0x1000);
    Memory HRAM("ROM_00", 0xff00, 0x0100);
    Cpu    cpu("cpu", 2000);
    bus.add_to_bus(&ROM_00);
    bus.add_to_bus(&HRAM);

    bus.write(0x100, 0x3e);
    bus.write(0x101, 0x16);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0x16);

    bus.write(0x102, 0x2e);
    bus.write(0x103, 0x19);
    cpu.step(&bus);
    cpu.step(&bus);

    assert(cpu.get_registers().read_L() == 0x19);

    bus.write(0x104, 0x85);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0x2f);

    bus.write(0x105, 0x27);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0x35);

    bus.write(0x106, 0x95);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0x1c);

    bus.write(0x107, 0x27);
    cpu.step(&bus);

    assert(cpu.get_registers().read_A() == 0x16);

    printf("-> TEST4 terminated succesfully\n");
  }

  {

    // TEST 5
    Bus    bus("Bus", 0, 0xffff, 8000);
    Memory ROM_00("ROM_00", 0, 0x1000);
    Memory HRAM("ROM_00", 0xff00, 0x0100);
    Cpu    cpu("cpu", 2000);
    bus.add_to_bus(&ROM_00);
    bus.add_to_bus(&HRAM);
    bus.add_to_bus(&cpu);

    for(int i = 0; i < 1000; i++) bus.step(&bus);

    assert(cpu.get_registers().PC == 0x01fa);

    printf("-> TEST5 terminated succesfully\n");
  }

  printf("-> All test terminated succesfully\n");
}
