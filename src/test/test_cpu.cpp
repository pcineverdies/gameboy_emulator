#include "../bus.h"
#include "../memory.h"
#include "../cpu/cpu.h"
#include "../cpu/registers.h"
#include <cassert>

int main(){

  {

    // TEST 1
    Bus    bus("Bus", 0, 0xffff);
    Memory ROM_00("ROM_00", 0, 0x1000);
    Memory HRAM("ROM_00", 0xff00, 0x0100);
    Cpu    cpu;
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


  }


}
