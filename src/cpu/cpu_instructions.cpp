#include "cpu.h"
#include "opcode.h"

/** CPU::execute_invalid
    Check whether the provided instruction is invalid

    @param bus Bus_obj* pointer to a bus to use for reading
    @param uint8_t opcode of the instruction to run
    @return bool true if an instruction has been executed

*/
bool Cpu::execute_invalid(Bus_obj*){
  if(_opcode == INVALID_OPCODE_1  or _opcode == INVALID_OPCODE_2  or
    _opcode ==  INVALID_OPCODE_3  or _opcode == INVALID_OPCODE_4  or
    _opcode ==  INVALID_OPCODE_5  or _opcode == INVALID_OPCODE_6  or
    _opcode ==  INVALID_OPCODE_7  or _opcode == INVALID_OPCODE_8  or
    _opcode ==  INVALID_OPCODE_9  or _opcode == INVALID_OPCODE_10 or
    _opcode ==  INVALID_OPCODE_11
  ) throw std::runtime_error("Parsed invalid opcode");

  return false;

}

/** CPU::execute_x8_lsm
    Collects the execution of all the instructions in the
    category lsm (Load/Store/Move 8 bits)

    @param bus Bus_obj* pointer to a bus to use for reading
    @param  uint8_t opcode of the instruction to run
    @return bool true if an instruction has been executed

*/
bool Cpu::execute_x8_lsm(Bus_obj* bus){

  uint8_t xx  = get_xx(_opcode);
  uint8_t yyy = get_yyy(_opcode);
  uint8_t zzz = get_zzz(_opcode);

  // ============================================================================

  if(check_mask(_opcode, LD_r_U8_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);

      // One extra state if the operand is (HL)
      if(yyy == LH_INDEX){
        _state = State::STATE_3;
      }
      else{
        write_x8(bus, yyy, _u8);
        _state = State::STATE_1;
      }
    }
    else if(_state == State::STATE_3){
      write_x8(bus, yyy, _u8);
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD_r_U8");

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, LD_m_A_OPCODE)){
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
    else std::runtime_error("Invalid state reached for instruction LD_m_A");

    return true;
  }

  // ============================================================================

  if(xx == LD_r_r and _opcode!= HALT_OPCODE){
    if(_state == State::STATE_1){
      _u8 = read_x8(bus, zzz);

      // One extra state if operand was (HL). There is not combination of (HL) being
      // both input and output
      if(yyy == LH_INDEX or zzz == LH_INDEX){
        _state = State::STATE_2;
      }
      else{
        write_x8(bus, yyy, _u8);
      }
    }
    else if(_state == State::STATE_2){
      // Read (HL) at the second cycle for proper timing
      _u8 = read_x8(bus, zzz);
      write_x8(bus, yyy, _u8);
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD_r_r");

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, LD_ff00_u8_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      // No sign extension for this operand: addressable space is [0xff00, 0xffff]
      if(yyy == 0b100){
        bus->write(0xff00 + _u8, registers.read_A());
      }
      else{
        registers.write_A(bus->read(0xff00 + _u8));
      }
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD_ff00_u8");

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, LD_ff00_C_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      if(yyy == 0b100){
        bus->write(0xff00 + registers.read_C(), registers.read_A());
      }
      else{
        registers.write_A(bus->read(0xff00 + registers.read_C()));
      }
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, LD_u16_A_OPCODE)){
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
      if(yyy == 0b101){
        bus->write(_u16, registers.read_A());
      }
      else{
        registers.write_A(bus->read(_u16));
      }
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD_u16_A");

    return true;
  }

  // ============================================================================

  return false;

}

/** CPU::execute_x16_lsm
    Collects the execution of all the instructions in the
    category lsm (Load/Store/Move 16 bits)

    @param bus Bus_obj* pointer to a bus to use for reading
    @param  uint8_t opcode of the instruction to run
    @return bool true if an instruction has been executed

*/
bool Cpu::execute_x16_lsm(Bus_obj* bus){

  // ============================================================================

  if(check_mask(_opcode, LD_r16_u16_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u8_2 = fetch(bus);
      if(_opcode == LD_BC_u16_OPCODE) registers.write_C(_u8), registers.write_B(_u8_2);
      if(_opcode == LD_DE_u16_OPCODE) registers.write_E(_u8), registers.write_D(_u8_2);
      if(_opcode == LD_HL_u16_OPCODE) registers.write_L(_u8), registers.write_H(_u8_2);
      if(_opcode == LD_SP_u16_OPCODE) registers.SP = (_u8_2 << 8 | _u8);
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD");

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, POP_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = bus->read(registers.SP++);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u8_2 = bus->read(registers.SP++);
      if(_opcode == POP_BC_OPCODE) registers.write_C(_u8), registers.write_B(_u8_2);
      if(_opcode == POP_DE_OPCODE) registers.write_E(_u8), registers.write_D(_u8_2);
      if(_opcode == POP_HL_OPCODE) registers.write_L(_u8), registers.write_H(_u8_2);
      if(_opcode == POP_AF_OPCODE) registers.write_F(_u8), registers.write_A(_u8_2);
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction POP");

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, PUSH_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      if(_opcode == PUSH_BC_OPCODE) bus->write(--registers.SP, registers.read_B());
      if(_opcode == PUSH_DE_OPCODE) bus->write(--registers.SP, registers.read_D());
      if(_opcode == PUSH_HL_OPCODE) bus->write(--registers.SP, registers.read_H());
      if(_opcode == PUSH_AF_OPCODE) bus->write(--registers.SP, registers.read_A());
      _state = State::STATE_4;
    }
    else if(_state == State::STATE_4){
      if(_opcode == PUSH_BC_OPCODE) bus->write(--registers.SP, registers.read_C());
      if(_opcode == PUSH_DE_OPCODE) bus->write(--registers.SP, registers.read_E());
      if(_opcode == PUSH_HL_OPCODE) bus->write(--registers.SP, registers.read_L());
      if(_opcode == PUSH_AF_OPCODE) bus->write(--registers.SP, registers.read_F());
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction PUSH");

    return true;
  }

  // ============================================================================

  if(_opcode == LD_u16_SP_OPCODE){
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
      bus->write(_u16, registers.SP & 0x00ff);
      _state = State::STATE_5;
    }
    else if(_state == State::STATE_5){
      bus->write(_u16 + 1, registers.SP >> 8);
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction LD_u16_SP");

    return true;
  }

  // ============================================================================

  if(_opcode == LD_SP_HL_OPCODE){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      registers.SP = registers.read_HL();
      _state = State::STATE_1;
    }

    return true;
  }
  else std::runtime_error("Invalid state reached for instruction LD_SP_HL");

  // ============================================================================

  return false;

}

/** CPU::execute_x8_alu
    Decodes and runs all the instructions in the category x8_alu

    @param bus Bus_obj* pointer to a bus to use for reading
    @return bool true if an instruction has been executed

*/
bool Cpu::execute_x8_alu(Bus_obj* bus){

  uint8_t yyy = get_yyy(_opcode);
  uint8_t zzz = get_zzz(_opcode);
  uint8_t A = registers.read_A();

  // Most opcodes perform the same operation on different operands. The operation
  // is distinguished thankts to the bits from 3 to 5. For this rason, a point to
  // member function is defined so that all the operations can be handled at once.
  uint8_t (Cpu::*f_alu) (uint8_t, uint8_t) =
    (yyy == ALU_ADD_YYY) ? &Cpu::add_x8 : (yyy == ALU_ADC_YYY) ? &Cpu::adc_x8 :
    (yyy == ALU_SUB_YYY) ? &Cpu::sub_x8 : (yyy == ALU_SBC_YYY) ? &Cpu::sbc_x8 :
    (yyy == ALU_AND_YYY) ? &Cpu::and_x8 : (yyy == ALU_XOR_YYY) ? &Cpu::xor_x8 :
    (yyy == ALU_OR_YYY) ? &Cpu::or_x8   :                        &Cpu::cp_x8;

  // ============================================================================

  if(check_mask(_opcode, ALU_INC_DEC_OPCODE)){
    if(yyy != LH_INDEX){
      _u8 = read_x8(bus, yyy);
      write_x8(bus, yyy, inc_dec_x8(_opcode, _u8));
    }
    else{
      if(_state == State::STATE_1){
        _state = State::STATE_2;
      }
      else if(_state == State::STATE_2){
        _u8 = read_x8(bus, yyy);
        _state = State::STATE_3;
      }
      else if(_state == State::STATE_3){
        write_x8(bus, yyy, inc_dec_x8(_opcode, _u8));
        _state = State::STATE_1;
      }
      else std::runtime_error("Invalid state reached for instruction INC");
    }

    return true;
  }

  // ============================================================================

  if(_opcode == RLCA_OPCODE){
    registers.set_Z(0);
    registers.set_N(0);
    registers.set_H(0);
    registers.set_C(A & 0x80);
    registers.write_A(A << 1 | A >> 7);

    return true;
  }

  // ============================================================================

  if(_opcode == RLA_OPCODE){
    registers.write_A(A << 1 | registers.get_C());
    registers.set_Z(0);
    registers.set_N(0);
    registers.set_H(0);
    registers.set_C(A & 0x80);

    return true;
  }

  // ============================================================================

  if(_opcode == DAA_OPCODE){
    uint8_t correction_value = 0;

    if(registers.get_H() or (registers.get_N() == 0 and (registers.read_A() & 0xf) > 9)){
      correction_value |= 0x06;
    }
    if(registers.get_C() or (registers.get_N() == 0 and (registers.read_A() > 0x99))){
      correction_value |= 0x60;
      registers.set_C(1);
    }

    registers.write_A(registers.read_A() + (registers.get_N() ? -correction_value : correction_value));

    registers.set_H(0);
    registers.set_Z(registers.read_A() == 0);

    return true;
  }

  // ============================================================================

  if(_opcode == SCF_OPCODE){
    registers.set_N(0);
    registers.set_H(0);
    registers.set_C(1);

    return true;
  }

  // ============================================================================

  if(_opcode == RRCA_OPCODE){
    registers.write_A(A >> 1 | A << 7);
    registers.set_Z(0);
    registers.set_N(0);
    registers.set_H(0);
    registers.set_C(A & 0x01);

    return true;
  }

  // ============================================================================

  if(_opcode == RRA_OPCODE){
    registers.write_A(A >> 1 | registers.get_C() << 7);
    registers.set_Z(0);
    registers.set_N(0);
    registers.set_H(0);
    registers.set_C(A & 0x01);

    return true;
  }

  // ============================================================================

  if(_opcode == CPL_OPCODE){
    registers.write_A(~A);
    registers.set_N(1);
    registers.set_H(1);

    return true;
  }

  // ============================================================================

  if(_opcode == CCF_OPCODE){
    registers.set_N(0);
    registers.set_H(0);
    registers.set_C(registers.get_C() == 1 ? 0 : 1);

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, ALU_OPCODE)){
    if(zzz != LH_INDEX){
      _u8 = read_x8(bus, zzz);
      registers.write_A((this->*f_alu)(registers.read_A(), _u8));
    }
    else{
      if(_state == State::STATE_1){
        _state = State::STATE_2;
      }
      else if(_state == State::STATE_2){
        _u8 = read_x8(bus, zzz);
        registers.write_A((this->*f_alu)(registers.read_A(), _u8));
        _state = State::STATE_1;
      }
      else std::runtime_error("Invalid state reached for instruction ALU");
    }

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, ALU_u8_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      registers.write_A((this->*f_alu)(registers.read_A(), _u8));
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction ALU_u8");

    return true;
  }

  // ============================================================================

  return false;

}

/** CPU::execute_x16_alu
    Decodes and runs all the instructions in the category x16_alu

    @param bus Bus_obj* pointer to a bus to use for reading
    @return bool true if an instruction has been executed

*/
bool Cpu::execute_x16_alu(Bus_obj* bus){

  // ============================================================================

  if(check_mask(_opcode, ALU_16_INC_DEC_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      if     (_opcode == INC_BC_OPCODE) registers.write_BC(registers.read_BC() + 1);
      else if(_opcode == INC_DE_OPCODE) registers.write_DE(registers.read_DE() + 1);
      else if(_opcode == INC_HL_OPCODE) registers.write_HL(registers.read_HL() + 1);
      else if(_opcode == INC_SP_OPCODE) registers.SP +=1;
      else if(_opcode == DEC_BC_OPCODE) registers.write_BC(registers.read_BC() - 1);
      else if(_opcode == DEC_DE_OPCODE) registers.write_DE(registers.read_DE() - 1);
      else if(_opcode == DEC_HL_OPCODE) registers.write_HL(registers.read_HL() - 1);
      else if(_opcode == DEC_SP_OPCODE) registers.SP -= 1;
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction ALU_16_INC_DEC");

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, ALU_16_r_r_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u16 = registers.read_HL();

      if(_opcode == ADD_HL_BC_OPCODE) _u16_2 = registers.read_BC();
      if(_opcode == ADD_HL_DE_OPCODE) _u16_2 = registers.read_DE();
      if(_opcode == ADD_HL_HL_OPCODE) _u16_2 = registers.read_HL();
      if(_opcode == ADD_HL_SP_OPCODE) _u16_2 = registers.SP;
      _u32 = _u16 + _u16_2;

      registers.set_N(0);
      registers.set_H((_u16 & 0xfff) + (_u16_2 & 0xfff) > 0xfff);
      registers.set_C((_u32 & 0x10000) ? 1 : 0);
      registers.write_HL(_u16 + _u16_2);

      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction ADD");

    return true;
  }

  // ============================================================================

  if(_opcode == ADD_SP_i8_OPCODE or _opcode == LD_HL_SP_i8_OPCODE){
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
      registers.set_Z(0);
      registers.set_N(0);
      registers.set_H((_u16 & 0xf) + (_u16_2 & 0xf) > 0xf);
      registers.set_C((_u16 & 0xff) + (_u16_2 & 0xff) > 0xff);

      if(_opcode == ADD_SP_i8_OPCODE){
        _state = State::STATE_4;
      }
      else{
        registers.write_HL(_u16 + _u16_2);
        _state = State::STATE_1;
      }
    }
    else if(_state == State::STATE_4){
      registers.SP = _u16 + _u16_2;
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction ADD/LD");

    return true;
  }

  // ============================================================================

  return false;
}

/** CPU::execute_control_br
    Decodes and runs all the instructions in the category control_br

    @param bus Bus_obj* pointer to a bus to use for reading
    @return bool true if an instruction has been executed

*/
bool Cpu::execute_control_br(Bus_obj* bus ){

  // ============================================================================

  if(check_mask(_opcode, JR_COND_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u8 = fetch(bus);
      _u16 = (_u8 & 0x80) ? _u8 | 0xff00 : _u8;
      if(get_jump_condition(_opcode)){
        _state = State::STATE_3;
      }
      else{
        _state = State::STATE_1;
      }
    }
    else if(_state == State::STATE_3){
      registers.PC += _u16;
      _state = State::STATE_1;

    }
    else std::runtime_error("Invalid state reached for instruction JR cond");

    return true;
  }

  // ============================================================================

  if(_opcode == JR_i8_OPCODE){
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

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, RET_COND_OPCODE)){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      if(get_jump_condition(_opcode)){
        _state = State::STATE_3;
      }
      else{
        _state = State::STATE_1;
      }
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

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, JP_COND_OPCODE) or _opcode == JP_OPCODE){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u16 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u16 = _u16 | (fetch(bus) << 8);
      if(get_jump_condition(_opcode) or _opcode == JP_OPCODE){
        _state = State::STATE_4;
      }
      else{
        _state = State::STATE_1;
      }
    }
    else if(_state == State::STATE_4){
      registers.PC = _u16;
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction JP cond");

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, CALL_COND_OPCODE) or _opcode == CALL_OPCODE){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else if(_state == State::STATE_2){
      _u16 = fetch(bus);
      _state = State::STATE_3;
    }
    else if(_state == State::STATE_3){
      _u16 = _u16 | (fetch(bus) << 8);
      if(get_jump_condition(_opcode) or _opcode == CALL_OPCODE){
        _state = State::STATE_4;
      }
      else{
        _state = State::STATE_1;
      }
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

    return true;
  }

  // ============================================================================

  if(check_mask(_opcode, RST_OPCODE)){
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

    return true;
  }

  // ============================================================================

  // The only difference is that RETI sets IME. No context switching in GBC
  // when returing from an interrupt
  if(_opcode == RET_OPCODE or _opcode == RETI_OPCODE){
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

      // RETI must set IME
      if(_opcode == RETI_OPCODE){
        IME = 1;
      }
      _state = State::STATE_1;
    }
    else std::runtime_error("Invalid state reached for instruction CALL cond");

    return true;
  }

  // ============================================================================

  if(_opcode == JP_HL_OPCODE){
    registers.PC = registers.read_HL();

    return true;
  }

  // ============================================================================

  return false;

}

/** CPU::execute_control_misc
    Decodes and runs all the instructions in the category control_misc

    @param bus Bus_obj* pointer to a bus to use for reading
    @return bool true if an instruction has been executed

*/
bool Cpu::execute_control_misc(Bus_obj* bus){

  // ============================================================================

  if(_opcode == NOP_OPCODE){
    return true;
  }

  // ============================================================================

  if(_opcode == STOP_OPCODE){
    return true; // Not used in DMG
  }

  // ============================================================================

  if(_opcode == HALT_OPCODE){
    halt_handler(bus);
    return true;
  }

  // ============================================================================

  if(_opcode == DI_OPCODE){
    IME = 0;

    return true;
  }

  // ============================================================================

  if(_opcode == EI_OPCODE){
    _ei_delayed = 2;

    return true;
  }

  // ============================================================================

  return false;
}

/** CPU::execute_x8_rsb
    Collects the execution of all the instructions in the
    category x8 rsb (Rotate/Shift/Bit 8 bits)

    @param bus Bus_obj* pointer to a bus to use for reading
    @param  uint8_t opcode of the instruction to run

*/
void Cpu::execute_x8_rsb(Bus_obj* bus){
  uint8_t zzz = get_zzz(_opcode);
  uint8_t yyy = get_yyy(_opcode);

  /*
   * rsb instructions which do not involve (HL) takes 2 M-cycles;
   * instructions with (HL) require either 3 or 4 M-cycles:
   *
   * In case of 4 M-cycles (read and modify (HL))
   * - fetch 1
   * - fetch 2
   * - read (HL)
   * - write (HL)
   *
   * In case of 3 M-cycles (read(HL))
   * - fetch 1
   * - fetch 2
   * - read (HL) and modify flags
   *
   * This sequence is handled through the states STATE_CB_X
   * */

  // If not LH_INDEX, then this is the second and last step of the CP operation.
  if(zzz != LH_INDEX){
      _u8 = read_x8(bus, zzz);
    _state = State::STATE_CB_4;
  }
  else{

    // Second stage of instructions using (LH); either they are made of 4
    // or 3 M-cycles
    if(_state == State::STATE_CB_2){
      if(check_mask(_opcode, CB_BIT_OPCODE)) _state = State::STATE_CB_4;
      else                                   _state = State::STATE_CB_3;
      return;
    }

    // If they are made of 4 M-cycles, in the third cycle you read (HL)
    if(_state == State::STATE_CB_3){
      _u8 = read_x8(bus, zzz);
      _state = State::STATE_CB_4;
      return;
    }
  }

  // Perform the operation and write the result back, either to the appropriate
  // register or to (HL)
  if(_state == State::STATE_CB_4){

    // If an instruction using (HL) is made of 3 M-cycles, the last cycle is
    // also the one in which it fetches the operand.
    if(check_mask(_opcode, CB_BIT_OPCODE))
      _u8 = read_x8(bus, zzz);

    // Bit N and H are always reset in instructions which are not BIT, RES and SET
    if(check_mask(_opcode, CB_BLOCK0_OPCODE)){
      registers.set_H(0);
      registers.set_N(0);
    }

    // ============================================================================

    if(check_mask(_opcode, CB_RLC_OPCODE)){
      registers.set_C(_u8 & 0x80);
      registers.set_Z(_u8 == 0);
      write_x8(bus, zzz, (_u8 << 1) | (_u8 >> 7));
    }

    // ============================================================================

    if(check_mask(_opcode, CB_RRC_OPCODE)){
      registers.set_C(_u8 & 0x01);
      registers.set_Z(_u8 == 0);
      write_x8(bus, zzz, (_u8 >> 1) | (_u8 << 7));
    }

    // ============================================================================

    if(check_mask(_opcode, CB_RL_OPCODE)){
      write_x8(bus, zzz, (_u8 << 1) | (registers.get_C()));
      registers.set_Z((((_u8 << 1) | registers.get_C()) & 0xff) == 0);
      registers.set_C(_u8 & 0x80);
    }

    // ============================================================================

    if(check_mask(_opcode, CB_RR_OPCODE)){
      write_x8(bus, zzz, (_u8 >> 1) | (registers.get_C() << 7));
      registers.set_Z(((_u8 >> 1) | (registers.get_C() << 7)) == 0);
      registers.set_C(_u8 & 0x01);
    }

    // ============================================================================

    if(check_mask(_opcode, CB_SLA_OPCODE)){
      registers.set_C(_u8 & 0x80);
      registers.set_Z(((_u8 << 1) & 0xff) == 0);
      write_x8(bus, zzz, (_u8 << 1));
    }

    // ============================================================================

    if(check_mask(_opcode, CB_SRA_OPCODE)){
      write_x8(bus, zzz, (_u8 & 0x80) | (_u8 >> 1));
      registers.set_Z(((_u8 & 0x80) | (_u8 >> 1)) == 0);
      registers.set_C(_u8 & 0x01);
    }

    // ============================================================================

    if(check_mask(_opcode, CB_SWAP_OPCODE)){
      registers.set_Z(_u8 == 0);
      registers.set_C(0);
      write_x8(bus, zzz, (_u8 << 4) | (_u8 >> 4));
    }

    // ============================================================================

    if(check_mask(_opcode, CB_SRL_OPCODE)){
      registers.set_C(_u8 & 0x01);
      registers.set_Z((_u8 >> 1) == 0);
      write_x8(bus, zzz, (_u8 >> 1));
    }

    // ============================================================================

    if(check_mask(_opcode, CB_BIT_OPCODE)){
      registers.set_H(1);
      registers.set_N(0);
      registers.set_Z(!(_u8 & (1 << yyy)));
    }

    // ============================================================================

    if(check_mask(_opcode, CB_SET_RES_OPCODE)){
      write_x8(bus, zzz, (_opcode & 0x40) ? (_u8 | (1 << yyy)) : (_u8 & ~(1 << yyy)));
    }

    // ============================================================================

    _state = State::STATE_1;
  }
}

/** CPU::inc_dec_x8
    Implement the ALU INC/DEC operations on operands on 8 bits.
    Takes care of updating the flags.

    @param opcode uint8_t opcode of the instruction, to distinguish between inc and dec
    @param u8 uint8_t operands to increment or decrement
    @return uint8_t result of the operation

*/
uint8_t Cpu::inc_dec_x8(uint8_t opcode, uint8_t u8){
  registers.set_Z(opcode & 1 ? ((u8 - 1) & 0xff) == 0 : ((u8 + 1) & 0xff) == 0);
  registers.set_N(opcode & 1 ? 1 : 0);
  registers.set_H((opcode & 1) ? (u8 & 0xf) == 0 : (u8 & 0xf) == 0xf);
  return (opcode & 1 ? u8 - 1 : u8 + 1);

}

/** CPU::add_x8
    Implement the ALU ADD operation on operands on 8 bits.
    Takes care of updating the flags.

    @param op1 uint8_t First operand to use
    @param op2 uint8_t Second operand to use
    @return uint8_t result of the operation

*/
uint8_t Cpu::add_x8(uint8_t op1, uint8_t op2){
  uint16_t res = op1 + op2;
  registers.set_Z((res & 0xff) == 0);
  registers.set_N(0);
  registers.set_H((op1 & 0xf) + (op2 & 0xf) > 0xf);
  registers.set_C((res & 0x0100) ? 1 : 0);
  return res & 0xff;

}

/** CPU::adc_x8
    Implement the ALU ADC operation on operands on 8 bits.
    Takes care of updating the flags.

    @param op1 uint8_t First operand to use
    @param op2 uint8_t Second operand to use
    @return uint8_t result of the operation

*/
uint8_t Cpu::adc_x8(uint8_t op1, uint8_t op2){
  uint16_t res = op1 + op2 + registers.get_C();
  registers.set_Z((res & 0xff) == 0);
  registers.set_N(0);
  registers.set_H((op1 & 0xf) + (op2 & 0xf) + registers.get_C() > 0xf);
  registers.set_C((res & 0x0100) ? 1 : 0);
  return res & 0xff;

}

/** CPU::sub_x8
    Implement the ALU SUB operation on operands on 8 bits.
    Takes care of updating the flags.

    @param op1 uint8_t First operand to use
    @param op2 uint8_t Second operand to use
    @return uint8_t result of the operation

*/
uint8_t Cpu::sub_x8(uint8_t op1, uint8_t op2){
  uint8_t res = op1 - op2;
  registers.set_Z(res == 0);
  registers.set_N(1);
  registers.set_H((op1 & 0xf) < (op2 & 0xf));
  registers.set_C(op1 < op2);
  return res;

}

/** CPU::sbc_x8
    Implement the ALU SBC operation on operands on 8 bits.
    Takes care of updating the flags.

    @param op1 uint8_t First operand to use
    @param op2 uint8_t Second operand to use
    @return uint8_t result of the operation

*/
uint8_t Cpu::sbc_x8(uint8_t op1, uint8_t op2){
  uint8_t res = op1 + ~op2 + !registers.get_C();
  registers.set_Z(res == 0);
  registers.set_N(1);
  registers.set_H((op1 & 0xf) < ((op2 & 0xf) + registers.get_C()));
  registers.set_C((uint16_t)op1 < (uint16_t)(op2 + registers.get_C()));
  return res;

}

/** CPU::and_x8
    Implement the ALU AND operation on operands on 8 bits.
    Takes care of updating the flags.

    @param op1 uint8_t First operand to use
    @param op2 uint8_t Second operand to use
    @return uint8_t result of the operation

*/
uint8_t Cpu::and_x8(uint8_t op1, uint8_t op2){
  uint8_t res = op1 & op2;
  registers.set_Z(res == 0);
  registers.set_N(0); registers.set_H(1); registers.set_C(0);
  return res;

}

/** CPU::xor_x8
    Implement the ALU XOR operation on operands on 8 bits.
    Takes care of updating the flags.

    @param op1 uint8_t First operand to use
    @param op2 uint8_t Second operand to use
    @return uint8_t result of the operation

*/
uint8_t Cpu::xor_x8(uint8_t op1, uint8_t op2){
  uint8_t res = op1 ^ op2;
  registers.set_Z(res == 0);
  registers.set_N(0); registers.set_H(0); registers.set_C(0);
  return res;

}

/** CPU::or_x8
    Implement the ALU OR operation on operands on 8 bits.
    Takes care of updating the flags.

    @param op1 uint8_t First operand to use
    @param op2 uint8_t Second operand to use
    @return uint8_t result of the operation

*/
uint8_t Cpu::or_x8(uint8_t op1, uint8_t op2){
  uint8_t res = op1 | op2;
  registers.set_Z(res == 0);
  registers.set_N(0); registers.set_H(0); registers.set_C(0);
  return res;

}

/** CPU::cp_x8
    Implement the ALU CP operation on operands on 8 bits.
    Takes care of updating the flags.

    @param op1 uint8_t First operand to use
    @param op2 uint8_t Second operand to use
    @return uint8_t result of the operation

*/
uint8_t Cpu::cp_x8(uint8_t op1, uint8_t op2){
  registers.set_Z(op1 == op2);
  registers.set_N(1);
  registers.set_H((op1 & 0xf) < (op2 & 0xf));
  registers.set_C(op1 < op2);
  return op1;

}

/** CPU::halt_handler
    Handles the halt instruction, taking care of the the different
    behaviors depending whether IME is set or not

    @param bus Bus_obj* pointer to a bus to use for reading (might invoke an interrupt handler)

*/
void Cpu::halt_handler(Bus_obj* bus){
  uint8_t IE = read_IE(bus);
  uint8_t IF = read_IF(bus);

  // If IME is 1, enter halt mode and continue executing
  // the current instruction until a new interrupt
  if(IME == 1){
    _is_halted = 1;
    registers.PC--;
    return;
  }

  // If IME == 0: if it was already in halt mode, exit once that
  // a new interrupt it raised. Otherwise stay in halt mode.
  if(_is_halted == 1){
    if(IF & IE & 0x1f){
      _is_halted = 0;
    }
    else{
      registers.PC--;
    }
    return;
  }

  // If IME == 0 and it was not in halt mode, then enter halt
  // mode in case no interrupt is present
  if(_is_halted == 0 and !(IF & IE & 0x1f)){
    _is_halted = 1;
    registers.PC--;
    return;
  }

  // If an interrupt is present, then:
  if(_is_halted == 0 and (IF & IF & 0x1f)){

    // handle the interrupt even if the previous instruction was `ie`
    if(_ei_delayed){
      IME = 1;
      _ei_delayed = 0;
      registers.PC--;
      interrupt_handler(bus);
    }
    // else enter halt bug mode (next interrupt is handled but PC not incremented)
    else{
      _halt_bug = 1;
    }
  }

}
