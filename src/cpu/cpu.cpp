#include "cpu.h"

/*
 * Due to the structure of the opcode table, it is efficient to divide
 * the opcode into three parts: xx_yyy_zzz. This simplifies decoding.
 * */

/** CPU::read_xx
    Get the bits 7 and 6 of an opcode

    @param opcode uint8_t opcode
    @return uint8_t xx

*/
uint8_t get_xx(uint8_t opcode){
  return (opcode & 0b11000000) >> 6;
}

/** CPU::read_yyy
    Get the bits  5, 4 and 3 of an opcode

    @param opcode uint8_t opcode
    @return uint8_t yyy

*/
uint8_t get_yyy(uint8_t opcode){
  return (opcode & 0b00111000) >> 3;
}

/** CPU::read_zzz
    Get the bits  2, 1 and 0 of an opcode

    @param opcode uint8_t opcode
    @return uint8_t zzz

*/
uint8_t get_zzz(uint8_t opcode){
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
bool check_mask(uint8_t data, std::string mask){
  if(mask.length() != 8) throw std::invalid_argument("Mask size is not 8");

  for(int i = 0; i < 8; i++){
    if(mask[7-i] == '0' and (data & (1 << i)) != 0) return false;
    if(mask[7-i] == '1' and (data & (1 << i)) == 0) return false;
  }

  return true;
}

/** CPU::CPU
    In charge of initializing the registers with the expected values.

*/
Cpu::Cpu(){
  registers.write_A(0x00);
  registers.write_F(0x00);
  registers.write_BC(0x00);
  registers.write_DE(0x00);
  registers.write_HL(0x00);

  registers.PC = 0x0100;
  registers.SP = 0xfffe;

}


/** CPU::fetch
    Read the memory at the current value of PC and update it.

    @param bus Bus* pointer to a bus to use for reading
    @return uint8_t read value

*/
uint8_t Cpu::fetch(Bus* bus){
  return bus->read(registers.PC++);
}

/** CPU::get_registers
    Return a copy of the class storing the current registers of the CPU.
    This is used for debug purposes.

    @return Registers current instances of the registers

*/
Registers Cpu::get_registers(){
  return registers;
}

/** CPU::step
    Performs the initial step of an instraction, whose length might be
    more than one singe M-cycle. This length is handled within the bus access.

    @param bus Bus* pointer to a bus to use for reading

*/
void Cpu::step(Bus* bus){

  uint8_t opcode = fetch(bus);

  // 0xCB case
  if(opcode == 0xcb){
    opcode = fetch(bus);
    execute_x8_rsb(bus, opcode);
  }
  else{
    // Try to match the opcode within all the possible categories
    // of instrcutions.
    execute_invalid(bus, opcode);
    execute_x8_lsm(bus, opcode);
    execute_x16_lsm(bus, opcode);
    execute_x8_alu(bus, opcode);
    execute_x16_alu(bus, opcode);
    execute_control_br(bus, opcode);
    execute_control_misc(bus, opcode);
  }
}

/** CPU::internal_state
    Some instructions spend 1 M-cycle without accessing the memory. In order
    to mimic this behavior, a fake reading is executed, so that no operation
    is perfromed, but the bus can still performs the T-cycles of the other elements.

    @param bus Bus* pointer to a bus to use for reading

*/
void internal_state(Bus* bus){
  bus->read(0x0000);
}

/** CPU::read_x8
    The following operation is performed:
      if index is 0, return the value of B
      if index is 1, return the value of C
      if index is 2, return the value of D
      if index is 3, return the value of E
      if index is 4, return the value of H
      if index is 5, return the value of L
      if index is A, return the value of (HL)
      if index is A, return the value of A

    @param bus Bus* pointer to a bus to use for reading
    @param index uint8_t index of the register to access
    @return uint8_t read value

*/
uint8_t Cpu::read_x8(Bus* bus, uint8_t index){

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
      if index is A, write the value of (HL) with data
      if index is A, write the value of A with data

    @param bus Bus* pointer to a bus to use for reading
    @param index uint8_t index of the register to access
    @param data uint8_t data to write

*/
void Cpu::write_x8(Bus* bus, uint8_t index, uint8_t data){
  if(index >= 8) throw std::runtime_error("Register index is not valid");

  if(index != 6) registers.registers[index] = data;
  else           bus->write(registers.read_HL(), data);

}

/** CPU::execute_x8_lsm
    Collects the execution of all the instructions in the
    category lsm (Load/Store/Move 8 bits)

    @param bus Bus* pointer to a bus to use for reading
    @param opcode uint8_t opcode of the instruction to run

*/
void Cpu::execute_x8_lsm(Bus* bus, uint8_t opcode){
  uint8_t u8;
  uint16_t u16;
  uint8_t xx = get_xx(opcode);
  uint8_t yyy = get_yyy(opcode);
  uint8_t zzz = get_zzz(opcode);

  if(xx == 0b00 and zzz == 0b110){
    u8 = fetch(bus);
    write_x8(bus, yyy, u8);
  }

  if(xx == 0b00 and zzz == 0b010){
    if(yyy == 0b000) bus->write(registers.read_BC(),   registers.read_A());
    if(yyy == 0b010) bus->write(registers.read_DE(),   registers.read_A());
    if(yyy == 0b100) bus->write(registers.read_HL_i(), registers.read_A());
    if(yyy == 0b110) bus->write(registers.read_HL_d(), registers.read_A());
    if(yyy == 0b001) registers.write_A(bus->read(registers.read_BC()));
    if(yyy == 0b011) registers.write_A(bus->read(registers.read_DE()));
    if(yyy == 0b101) registers.write_A(bus->read(registers.read_HL_i()));
    if(yyy == 0b111) registers.write_A(bus->read(registers.read_HL_d()));
  }

  if(xx == 0b01 and opcode != 0x76){
    write_x8(bus, yyy, read_x8(bus, zzz));
  }

  if(check_mask(opcode, "111x0000")){
    u8 = fetch(bus);
    if(yyy == 0b101) bus->write(0xff00 + u8, registers.read_A());
    if(yyy == 0b111) registers.write_A(bus->read(0xff00 + u8));
  }

  if(check_mask(opcode, "111x0010")){
    if(yyy == 0b100) bus->write(0xff00 + registers.read_C(), registers.read_A());
    if(yyy == 0b110) registers.write_A(bus->read(0xff00 + registers.read_C()));
  }

  if(check_mask(opcode, "111x1010")){
    u16 = fetch(bus);
    u16 = u16 | (fetch(bus) << 8);

    if(yyy == 0b101) bus->write(u16, registers.read_A());
    if(yyy == 0b111) registers.write_A(bus->read(u16));
  }

}

/** CPU::execute_x16_lsm
    Collects the execution of all the instructions in the
    category lsm (Load/Store/Move 16 bits)

    @param bus Bus* pointer to a bus to use for reading
    @param opcode uint8_t opcode of the instruction to run

*/
void Cpu::execute_x16_lsm(Bus* bus, uint8_t opcode){
  uint8_t u8_1;
  uint8_t u8_2;
  uint16_t u16;

  if(check_mask(opcode, "00xx0001")){
    u8_1 = fetch(bus); u8_2 = fetch(bus);
    if(opcode == 0x01) registers.write_C(u8_1), registers.write_B(u8_2);
    if(opcode == 0x11) registers.write_E(u8_1), registers.write_D(u8_2);
    if(opcode == 0x21) registers.write_L(u8_1), registers.write_H(u8_2);
    if(opcode == 0x31) registers.SP = (u8_2 << 8 | u8_1);
  }

  if(check_mask(opcode, "11xx0001")){
    u8_1 = bus->read(registers.SP++); u8_2 = bus->read(registers.SP++);
    if(opcode == 0x01) registers.write_C(u8_1), registers.write_B(u8_2);
    if(opcode == 0x11) registers.write_E(u8_1), registers.write_D(u8_2);
    if(opcode == 0x21) registers.write_L(u8_1), registers.write_H(u8_2);
    if(opcode == 0x31) registers.write_F(u8_1), registers.write_A(u8_2);
  }

  if(check_mask(opcode, "11xx0101")){
    internal_state(bus);
    if(opcode == 0xc5) bus->write(--registers.SP, registers.read_B()), bus->write(--registers.SP, registers.read_C());
    if(opcode == 0xd5) bus->write(--registers.SP, registers.read_D()), bus->write(--registers.SP, registers.read_E());
    if(opcode == 0xe5) bus->write(--registers.SP, registers.read_H()), bus->write(--registers.SP, registers.read_L());
    if(opcode == 0xf5) bus->write(--registers.SP, registers.read_A()), bus->write(--registers.SP, registers.read_F());
  }

  if(opcode == 0x08){
    u16 = fetch(bus); u16 |= (fetch(bus) << 8);
    bus->write(u16    , registers.SP & 0x00ff);
    bus->write(u16 + 1, registers.SP >> 8);
  }

  if(opcode == 0xf9){
    internal_state(bus);
    registers.SP = registers.read_HL();
  }
}

void Cpu::execute_x8_alu(Bus* bus, uint8_t opcode){}

void Cpu::execute_x16_alu(Bus* bus, uint8_t opcode){}

void Cpu::execute_control_br(Bus* bus, uint8_t opcode){}

void Cpu::execute_control_misc(Bus* bus, uint8_t opcode){}

void Cpu::execute_x8_rsb(Bus* bus, uint8_t opcode){}

/** CPU::execute_invalid
    Check whether the provided instruction is invalid

    @param bus Bus* pointer to a bus to use for reading
    @param opcode uint8_t opcode of the instruction to run

*/
void Cpu::execute_invalid(Bus* bus, uint8_t opcode){
  if(
    opcode == 0xd3 or opcode == 0xe3 or opcode == 0xe4 or opcode == 0xf4 or
    opcode == 0xdb or opcode == 0xeb or opcode == 0xec or opcode == 0xfc or
    opcode == 0xdd or opcode == 0xed or opcode == 0xfd
  ) throw std::runtime_error("Parsed invalid opcode");
}
