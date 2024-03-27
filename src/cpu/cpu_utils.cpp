#include "cpu.h"

/*
 * Due to the structure of the  table, it is efficient to divide
 * the  into three parts: xx_yyy_zzz. This simplifies decoding.
 * */

/** CPU::read_xx
    Get the bits 7 and 6 of an

    @param  uint8_t opcode
    @return uint8_t xx

*/
uint8_t Cpu::get_xx(uint8_t opcode){
  return (opcode & 0b11000000) >> 6;
}

/** CPU::read_yyy
    Get the bits  5, 4 and 3 of an

    @param  uint8_t opcode
    @return uint8_t yyy

*/
uint8_t Cpu::get_yyy(uint8_t opcode){
  return (opcode & 0b00111000) >> 3;
}

/** CPU::read_zzz
    Get the bits  2, 1 and 0 of an

    @param  uint8_t opcode
    @return uint8_t zzz

*/
uint8_t Cpu::get_zzz(uint8_t opcode){
  return (opcode & 0b00000111);
}

/** CPU::check_mask
    Given an 8 bits character mask it checks whether the data has the same structure.
    Though this function looks a bit awkward in this context, it is simple to
    translate in hardware given a fixed mask.

    @param data uint8_t data to check the mask with
    @param mask std::string mask made of `0`, `1` and `x`.
    @return bool correctness of the mask

*/
bool Cpu::check_mask(uint8_t data, std::string mask){
  if(mask.length() != 8) throw std::invalid_argument("Mask size is not 8");

  for(int i = 0; i < 8; i++){
    if(mask[7-i] == '0' and (data & (1 << i)) != 0) return false;
    if(mask[7-i] == '1' and (data & (1 << i)) == 0) return false;
  }

  return true;
}

/** CPU::get_jump_condition
    Given an opcode, checks whether the jump condition is valid,
    considering the current value of the Z and C flags

    @param opcode uint8_t opcode of the current instruction
    @return bool condition verified

*/
bool Cpu::get_jump_condition(uint8_t opcode){
  if(check_mask(opcode, "xxx00xxx") and registers.get_Z() == 0) return true;
  if(check_mask(opcode, "xxx01xxx") and registers.get_Z() == 1) return true;
  if(check_mask(opcode, "xxx10xxx") and registers.get_C() == 0) return true;
  if(check_mask(opcode, "xxx11xxx") and registers.get_C() == 1) return true;
                                                                return false;
}

/** CPU::get_registers
    Return a copy of the class storing the current registers of the CPU.
    This is used for debug purposes.

    @return Registers current instances of the registers

*/
Registers Cpu::get_registers(){
  return registers;
}

/** CPU::read_x8
    The following operation is performed:
      if index is 0, return the value of B
      if index is 1, return the value of C
      if index is 2, return the value of D
      if index is 3, return the value of E
      if index is 4, return the value of H
      if index is 5, return the value of L
      if index is 6, return the value of (HL)
      if index is 7, return the value of A

    @param bus Bus_obj* pointer to a bus to use for reading
    @param index uint8_t index of the register to access
    @return uint8_t read value

*/
uint8_t Cpu::read_x8(Bus_obj* bus, uint8_t index){

  if(index >= 8) throw std::runtime_error("Register index is not valid");

  if(index != 6) return registers.registers[index];
                 return bus->read(registers.read_HL());
}

/** CPU::write_x8
    The following operation is performed:
      if index is 0, write the value of B with data
      if index is 1, write the value of C with data
      if index is 2, write the value of D with data
      if index is 3, write the value of E with data
      if index is 4, write the value of H with data
      if index is 5, write the value of L with data
      if index is 6, write the value of (HL) with data
      if index is 7, write the value of A with data

    @param bus Bus_obj* pointer to a bus to use for reading
    @param index uint8_t index of the register to access
    @param data uint8_t data to write

*/
void Cpu::write_x8(Bus_obj* bus, uint8_t index, uint8_t data){
  if(index >= 8) throw std::runtime_error("Register index is not valid");

  if(index != 6) registers.registers[index] = data;
  else           bus->write(registers.read_HL(), data);

}

/** CPU::read_IE
    Read the content of the memory-mapped register IE
    (it does not require an extra cycle)

    @param bus Bus_obj* pointer to a bus to use for reading
    @return uint8_t value of the register

*/
uint8_t Cpu::read_IE(Bus_obj* bus){
  return bus->read(IE_ADDRESS);
}

/** CPU::read_IF
    Read the content of the memory-mapped register IF
    (it does not require an extra cycle)

    @param bus Bus_obj* pointer to a bus to use for reading
    @return uint8_t value of the register

*/
uint8_t Cpu::read_IF(Bus_obj* bus){
  return bus->read(IF_ADDRESS);
}

/** CPU::write_IE
    Write the content of the memory-mapped register IE
    (it does not require an extra cycle)

    @param bus Bus_obj* pointer to a bus to use for writing
    @param data uint8_t data to write

*/
void Cpu::write_IE(Bus_obj* bus, uint8_t data){
  bus->write(IE_ADDRESS, data);
}

/** CPU::write_IF
    Write the content of the memory-mapped register IF
    (it does not require an extra cycle)

    @param bus Bus_obj* pointer to a bus to use for writing
    @param data uint8_t data to write

*/
void Cpu::write_IF(Bus_obj* bus, uint8_t data){
  bus->write(IF_ADDRESS, data);
}

/** CPU::print_status
    Print status of the CPU according to peach's format

    @param bus Bus_obj* pointer to a bus to use for writing

*/
void Cpu::print_status(Bus_obj* bus){
  printf(
    "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
    registers.read_A(),
    registers.read_F(),
    registers.read_B(),
    registers.read_C(),
    registers.read_D(),
    registers.read_E(),
    registers.read_H(),
    registers.read_L(),
    registers.SP,
    registers.PC,
    bus->read(registers.PC),
    bus->read(registers.PC + 1),
    bus->read(registers.PC + 2),
    bus->read(registers.PC + 3)
  );
}

/** CPU::print_serial
    Reads the content of the serial register (memory mapped at 0xff01)
    and prints it on the screen if the value is different from the previous one.
    This is usefule for an early-debugging phase, though it cannot be
    used to properly get the content to be sent on the serial port.

    @param bus Bus_obj* pointer to a bus to use for writing

*/
void Cpu::print_serial(Bus_obj* bus){
  // Remembers previous read character
  static char prev = 0;

  // Read current character
  char val = bus->read(0xff01);

  // Print if different
  if(val != prev){
    printf("%c", val);
    prev = val;
  }
}
