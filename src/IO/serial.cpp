#include "serial.h"
#include <cstdio>

/** Serial::read
    Read one of the serial registers

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t Serial::read(uint16_t addr){

  uint8_t res;

  if(addr == 0){
    res = SB;
  }
  else if (addr == 1){
    res =  SC;
  }
  else{
    std::invalid_argument("Invalid address while acessing serial");
  }

  return res;
}

/** Serial::write
    Write of the registers

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void Serial::write(uint16_t addr, uint8_t data){

  if(addr == 0){
    SB = data;
  }
  else if(addr == 1){
    SC = data;

    // Print on the screen the data which will be sent
    // Debug purposes TODO remove this functionality
    if(data & 0x80) printf("%c", SB);
  }
  else{
    std::invalid_argument("Invalid address while accessing serial");
  }
}

/** Serial::Serial
    Serial constructor, setting init_addr of the serial in the memory space
    The size of the object is fixed to 2.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus

*/
Serial::Serial(std::string name, uint16_t init_addr) : Bus_obj(name, init_addr, 2){
  SC = 0;
  SB = 0;
}

/** Serial::step
    Perform the step of the serial at each T-cycle.
    TBD

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void Serial::step(Bus_obj* bus){}

/** Serial::set_interrupt
    Set the corresponding interrupt flag in the IF register.
    In the OOP approach that is followed in the project, first the
    register must be read, modified and written again. In reality,
    this operation is not time consuming, and can be performed during
    the serial step.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void Serial::set_interrupt(Bus_obj* bus){
  uint8_t interrupt_flag_value = bus->read(IF_ADDRESS);
  interrupt_flag_value |= IF_SERIAL;
  bus->write(IF_ADDRESS, interrupt_flag_value);
}
