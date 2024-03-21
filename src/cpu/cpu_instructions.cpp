#include "cpu.h"

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
      registers.PC = _u16;
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

