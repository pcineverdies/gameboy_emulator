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
uint8_t get_xx(uint8_t opcode){
  return (opcode & 0b11000000) >> 6;
}

/** CPU::read_yyy
    Get the bits  5, 4 and 3 of an

    @param  uint8_t opcode
    @return uint8_t yyy

*/
uint8_t get_yyy(uint8_t opcode){
  return (opcode & 0b00111000) >> 3;
}

/** CPU::read_zzz
    Get the bits  2, 1 and 0 of an

    @param  uint8_t opcode
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

bool Cpu::get_jump_condition(uint8_t opcode){
  if(check_mask(opcode, "xxx00xxx") and registers.get_Z() == 0) return true;
  if(check_mask(opcode, "xxx01xxx") and registers.get_Z() == 1) return true;
  if(check_mask(opcode, "xxx10xxx") and registers.get_C() == 0) return true;
  if(check_mask(opcode, "xxx11xxx") and registers.get_C() == 1) return true;
                                                                return false;
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

void Cpu::print_state(){
  if(_state == State::STATE_1) std::cout << "STATE 1" << std::endl;
  if(_state == State::STATE_2) std::cout << "STATE 2" << std::endl;
  if(_state == State::STATE_3) std::cout << "STATE 3" << std::endl;
  if(_state == State::STATE_4) std::cout << "STATE 4" << std::endl;
  if(_state == State::STATE_5) std::cout << "STATE 5" << std::endl;
  if(_state == State::STATE_6) std::cout << "STATE 6" << std::endl;
}

/** CPU::step
    Performs the initial step of an instraction, whose length might be
    more than one singe M-cycle. This length is handled within the bus access.

    @param bus Bus* pointer to a bus to use for reading

*/
void Cpu::step(Bus* bus){

  print_state();

  if(_state == State::STATE_1){
    _opcode = fetch(bus);
  }

  if(_opcode == 0xcb){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else{
      _opcode = fetch(bus);
      execute_x8_rsb(bus);
      _state = State::STATE_1;
    }
  }

  // Try to match the  within all the possible categories of instrcutions.
  execute_invalid(bus);
  execute_x8_lsm(bus);
  execute_x16_lsm(bus);
  execute_x8_alu(bus);
  execute_x16_alu(bus);
  execute_control_br(bus);
  execute_control_misc(bus);
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
    @param  uint8_t opcode of the instruction to run

*/
void Cpu::execute_x8_lsm(Bus* bus){

  uint8_t xx  = get_xx(_opcode);
  uint8_t yyy = get_yyy(_opcode);
  uint8_t zzz = get_zzz(_opcode);

  if(check_mask(_opcode, "00xxx110")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      write_x8(bus, yyy, _u8);
      if(yyy == 6) _state = State::STATE_3;
      else         _state = State::STATE_1;
    }
    else if(_state == State::STATE_3){
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");
  }

  if(check_mask(_opcode, "00xxx010")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      if(yyy == 0b000) bus->write(registers.read_BC(),   registers.read_A());
      if(yyy == 0b010) bus->write(registers.read_DE(),   registers.read_A());
      if(yyy == 0b100) bus->write(registers.read_HL_i(), registers.read_A());
      if(yyy == 0b110) bus->write(registers.read_HL_d(), registers.read_A());
      if(yyy == 0b001) registers.write_A(bus->read(registers.read_BC()));
      if(yyy == 0b011) registers.write_A(bus->read(registers.read_DE()));
      if(yyy == 0b101) registers.write_A(bus->read(registers.read_HL_i()));
      if(yyy == 0b111) registers.write_A(bus->read(registers.read_HL_d()));
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");
  }

  if(xx == 0b01 and _opcode!= 0x76){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      write_x8(bus, yyy, read_x8(bus, zzz));
      if(yyy == 6 or zzz == 6) _state = State::STATE_3;
      else                     _state = State::STATE_1;
    }
    else if(_state == State::STATE_3){
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");
  }

  if(check_mask(_opcode, "111x0000")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      if(yyy == 0b101) bus->write(0xff00 + _u8, registers.read_A());
      if(yyy == 0b111) registers.write_A(bus->read(0xff00 + _u8));
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");
  }

  if(check_mask(_opcode, "111x0010")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      if(yyy == 0b100) bus->write(0xff00 + registers.read_C(), registers.read_A());
      if(yyy == 0b110) registers.write_A(bus->read(0xff00 + registers.read_C()));
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");
  }

  if(check_mask(_opcode, "111x1010")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u16 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u16 = _u16 | (fetch(bus) << 8);
      _state = State::STATE_4;
    }
    else if(_state == State::STATE_4){
      if(yyy == 0b101) bus->write(_u16, registers.read_A());
      if(yyy == 0b111) registers.write_A(bus->read(_u16));
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");
  }

}

/** CPU::execute_x16_lsm
    Collects the execution of all the instructions in the
    category lsm (Load/Store/Move 16 bits)

    @param bus Bus* pointer to a bus to use for reading
    @param  uint8_t opcode of the instruction to run

*/
void Cpu::execute_x16_lsm(Bus* bus){

  if(check_mask(_opcode, "00xx0001")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u8_2 = fetch(bus);
      if(_opcode == 0x01) registers.write_C(_u8), registers.write_B(_u8_2);
      if(_opcode == 0x11) registers.write_E(_u8), registers.write_D(_u8_2);
      if(_opcode == 0x21) registers.write_L(_u8), registers.write_H(_u8_2);
      if(_opcode == 0x31) registers.SP = (_u8_2 << 8 | _u8);
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");
  }

  if(check_mask(_opcode, "11xx0001")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = bus->read(registers.SP++);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u8_2 = bus->read(registers.SP++);
      if(_opcode == 0x01) registers.write_C(_u8), registers.write_B(_u8_2);
      if(_opcode == 0x11) registers.write_E(_u8), registers.write_D(_u8_2);
      if(_opcode == 0x21) registers.write_L(_u8), registers.write_H(_u8_2);
      if(_opcode == 0x31) registers.write_F(_u8), registers.write_A(_u8_2);
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction POP");
  }

  if(check_mask(_opcode, "11xx0101")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      if(_opcode == 0xc5) bus->write(--registers.SP, registers.read_B());
      if(_opcode == 0xd5) bus->write(--registers.SP, registers.read_D());
      if(_opcode == 0xe5) bus->write(--registers.SP, registers.read_H());
      if(_opcode == 0xf5) bus->write(--registers.SP, registers.read_A());
      _state = State::STATE_4;
    }
    else if(_state == State::STATE_4){
      if(_opcode == 0xc5) bus->write(--registers.SP, registers.read_C());
      if(_opcode == 0xd5) bus->write(--registers.SP, registers.read_E());
      if(_opcode == 0xe5) bus->write(--registers.SP, registers.read_L());
      if(_opcode == 0xf5) bus->write(--registers.SP, registers.read_F());
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction PUSH");
  }

  if(_opcode == 0x08){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u16 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u16 |= (fetch(bus) << 8);
      _state = State::STATE_4;
    }
    else if(_state == State::STATE_4){
      bus->write(_u16    , registers.SP & 0x00ff);
      _state = State::STATE_5;
    }
    else if(_state == State::STATE_5){
      bus->write(_u16 + 1, registers.SP >> 8);
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");
  }

  if(_opcode == 0xf9){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      registers.SP = registers.read_HL();
      _state = State::STATE_1;
    }
  }
  else std::runtime_error("Invalid state reached for instruction LD");
}

void Cpu::execute_x8_alu(Bus* bus){}

void Cpu::execute_x16_alu(Bus* bus){

  if(check_mask(_opcode, "00xxx011")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      if     (_opcode == 0x03) registers.write_BC(registers.read_BC() + 1);
      else if(_opcode == 0x12) registers.write_DE(registers.read_DE() + 1);
      else if(_opcode == 0x23) registers.write_HL(registers.read_HL() + 1);
      else if(_opcode == 0x33) registers.SP +=1;
      else if(_opcode == 0x0B) registers.write_BC(registers.read_BC() - 1);
      else if(_opcode == 0x1B) registers.write_DE(registers.read_DE() - 1);
      else if(_opcode == 0x2B) registers.write_HL(registers.read_HL() - 1);
      else if(_opcode == 0x3B) registers.SP -= 1;
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction INC/DEC");
  }

  if(check_mask(_opcode, "00xx1001")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u16 = registers.read_HL();

      if(_opcode == 0x09) _u16_2 = registers.read_BC();
      if(_opcode == 0x19) _u16_2 = registers.read_DE();
      if(_opcode == 0x29) _u16_2 = registers.read_HL();
      if(_opcode == 0x39) _u16_2 = registers.SP;

      registers.set_N(0);
      registers.set_H((_u16 & 0xfff) + (_u16_2 & 0xfff) > 0xfff);
      _u32 = _u16 + _u16_2;
      registers.set_C(_u32 & 0x10000);
      registers.write_HL(_u16 + _u16_2);

      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction ADD");
  }
  if(_opcode == 0xe8 or _opcode == 0xf8){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u16 = registers.SP;
      _u16_2 = _u8 | (_u8 & 0x80 ? 0xff00 : 0);
      registers.set_Z(0); registers.set_N(0);
      registers.set_H((_u16 & 0xf) + (_u16_2 & 0xf) > 0xf);
      registers.set_C((_u16 & 0xff) + (_u16_2 & 0xff) > 0xff);

      if(_opcode == 0xe8) registers.SP = _u16 + _u16_2, _state = State::STATE_4;
      else                registers.write_HL(_u16 + _u16_2), _state = State::STATE_1;

    }
    else if(_state == State::STATE_4){
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction ADD/LD");
  }
}

void Cpu::execute_control_br(Bus* bus ){

  if(check_mask(_opcode, "001xx000")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      _u16 = (_u8 & 0x80) ? _u8 | 0xff00 : _u8;
      if(get_jump_condition(_opcode)) _state = State::STATE_3;
      else                            _state = State::STATE_1;
    }
    else if(_state == State::STATE_3){
      registers.PC += _u16;
      _state = State::STATE_1;

    }
    else std::runtime_error("Invalid state reached for instruction JR cond");

  }

  if(_opcode == 0x18){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      _u16 = (_u8 & 0x80) ? _u8 | 0xff00 : _u8;
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      registers.PC += _u16;
      _state = State::STATE_1;

    }
    else std::runtime_error("Invalid state reached for instruction JR cond");
  }

  if(check_mask(_opcode, "110xx000")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      if(get_jump_condition(_opcode)) _state = State::STATE_3;
      else                            _state = State::STATE_1;
    }
    else if(_state == State::STATE_3){
      _u16 = bus->read(registers.SP++);
      _state = State::STATE_4;
    }
    else if(_state == State::STATE_4){
      _u16 = _u16 | (bus->read(registers.SP++) << 8);
      _state = State::STATE_5;
    }
    else if(_state == State::STATE_5){
      registers.PC = _u16;
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction RET cond");
  }

  if(check_mask(_opcode, "110xx010") or _opcode == 0xc3){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u16 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u16 = _u16 | (fetch(bus) << 8);
      if(get_jump_condition(_opcode) or _opcode == 0xc3) _state = State::STATE_4;
      else                                               _state = State::STATE_1;
    }
    else if(_state == State::STATE_4){
      internal_state(bus); registers.PC = _u16;
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction JP cond");
  }

  if(check_mask(_opcode, "110xx100") or _opcode == 0xcd){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u16 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u16 = _u16 | (fetch(bus) << 8);
      if(get_jump_condition(_opcode) or _opcode == 0xcd) _state = State::STATE_4;
      else                                               _state = State::STATE_1;
    }
    else if(_state == State::STATE_4){
      _state = State::STATE_5;
    }
    else if(_state == State::STATE_5){
      bus->write(--registers.SP, registers.PC >> 8);
      _state = State::STATE_6;
    }
    else if(_state == State::STATE_6){
      bus->write(--registers.SP, registers.PC & 0xff);
      registers.PC = _u16;
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction CALL cond");
  }

  if(check_mask(_opcode, "11xxx111")){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = (_opcode & 0b00111000) >> 3;
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      bus->write(--registers.SP, registers.PC >> 8);
      _state = State::STATE_4;
    }
    else if(_state == State::STATE_4){
      bus->write(--registers.SP, registers.PC & 0xff);
      registers.PC = _u8 * 8;
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction CALL cond");
  }

  if(_opcode == 0xc9 or _opcode == 0xd9){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u16 = bus->read(registers.SP++);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u16 = _u16 | (bus->read(registers.SP++) << 8);
      _state = State::STATE_4;
    }
    else if(_state == State::STATE_4){
      registers.PC = _u16;
      if(_opcode == 0xd9){} // TODO INTERRUPT
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction CALL cond");
  }
  if(_opcode == 0xe9){
    registers.PC = registers.read_HL();
  }
}

void Cpu::execute_control_misc(Bus* bus){}

/** CPU::execute_x8_rsb
    Collects the execution of all the instructions in the
    category x8 rsb (Rotate/Shift/Bit 8 bits)

    @param bus Bus* pointer to a bus to use for reading
    @param  uint8_t opcode of the instruction to run

*/
void Cpu::execute_x8_rsb(Bus* bus){
  uint8_t zzz = get_zzz(_opcode);
  uint8_t yyy = get_yyy(_opcode);
  uint8_t u8 = read_x8(bus, zzz);

  // Bit N and H are always reset in instructions which are not BIT, RES and SET
  if(check_mask(_opcode, "00xxxxxx")) registers.set_H(0), registers.set_N(0);

  if(check_mask(_opcode, "00000xxx")){ // RLC instruction
    registers.set_C(u8 & 0x80); registers.set_Z(u8 == 0);
    write_x8(bus, zzz, (u8 << 1) | (u8 >> 7));
  }
  if(check_mask(_opcode, "00001xxx")){   //RRC
    registers.set_C(u8 & 0x01); registers.set_Z(u8 == 0);
    write_x8(bus, zzz, (u8 >> 1) | (u8 << 7));
  }
  if(check_mask(_opcode, "00010xxx")){   // RL
    write_x8(bus, zzz, (u8 << 1) | (registers.get_C()));
    registers.set_Z(((u8 << 1) | registers.get_C()) == 0); registers.set_C(u8 & 0x80);
  }
  if(check_mask(_opcode, "00011xxx")){ // RR
    write_x8(bus, zzz, (u8 >> 1) | (registers.get_C() << 7));
    registers.set_Z(((u8 >> 1) | (registers.get_C() << 7)) == 0); registers.set_C(u8 & 0x01);
  }
  if(check_mask(_opcode, "00100xxx")){ // SLA
    registers.set_C(u8 & 0x80); registers.set_Z((u8 << 1) == 0);
    write_x8(bus, zzz, (u8 << 1));
  }
  if(check_mask(_opcode, "00101xxx")){  // SRA
    write_x8(bus, zzz, (u8 & 0x80) | (u8 >> 1));
    registers.set_Z(((u8 & 0x80) | (u8 >> 1)) == 0); registers.set_C(u8 & 0x01);
  }
  if(check_mask(_opcode, "00110xxx")){ // SWAP
    registers.set_Z(u8 == 0), registers.set_C(0);
    write_x8(bus, zzz, (u8 << 4) | (u8 >> 4));
  }
  if(check_mask(_opcode, "00111xxx")){  // SRL
    registers.set_C(u8 & 0x01); registers.set_Z((u8 >> 1) == 0);
    write_x8(bus, zzz, (u8 >> 1));
  }
  if(check_mask(_opcode, "01yyyxxx")){  // BIT
    registers.set_H(1), registers.set_N(0);
    registers.set_Z(u8 & (1 << yyy));
  }
  if(check_mask(_opcode, "1zyyyxxx")){ // RES/SET
    write_x8(bus, zzz, (_opcode & 0x40) ? (u8 | (1 << yyy)) : (u8 & ~(1 << yyy)));
  }
}

/** CPU::execute_invalid
    Check whether the provided instruction is invalid

    @param bus Bus* pointer to a bus to use for reading
    @param  uint8_t opcode of the instruction to run

*/
void Cpu::execute_invalid(Bus* bus){
  if(
    _opcode == 0xd3 or _opcode == 0xe3 or _opcode == 0xe4 or _opcode == 0xf4 or
    _opcode == 0xdb or _opcode == 0xeb or _opcode == 0xec or _opcode == 0xfc or
    _opcode == 0xdd or _opcode == 0xed or _opcode == 0xfd
  ) throw std::runtime_error("Parsed invalid ");
}
