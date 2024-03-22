#include "cpu.h"


/** CPU::CPU
    In charge of initializing the registers with the expected values.

    @param frequency uint32_t Frequency of the cpu

*/
Cpu::Cpu(std::string name, uint32_t frequency) : Bus_obj(name, 0, 0){

  this->set_frequency(frequency);

  _state = State::STATE_1;

  registers.write_A(0x00);
  registers.write_F(0x00);
  registers.write_BC(0x00);
  registers.write_DE(0x00);
  registers.write_HL(0x00);

  registers.PC = 0x0100;
  registers.SP = 0xfffe;

  IME = 1;
  _ei_delayed = 0;
  _is_halted = 0;
  _halt_bug = 0;

}


/** CPU::fetch
    Read the memory at the current value of PC and update it.

    @param bus Bus_obj* pointer to a bus to use for reading
    @return uint8_t read value

*/
uint8_t Cpu::fetch(Bus_obj* bus){
  return bus->read(registers.PC++);
}


/** CPU::step
    Performs the initial step of an instraction, whose length might be
    more than one singe M-cycle. This length is handled within the bus access.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void Cpu::step(Bus_obj* bus){

  if(interrupt_handler(bus)){
    return;
  }

  if(_state == State::STATE_1){

    if(_ei_delayed == 1){
      IME = 1;
      _ei_delayed = 0;
    }

    _opcode = fetch(bus);

    if(_halt_bug == 1){
      registers.PC--;
      _halt_bug = 0;
    }

  }

  if(_opcode == CB_OPCODE){
    if(_state == State::STATE_1){
      _state = State::STATE_2;
    }
    else{
      _opcode = fetch(bus);
      execute_x8_rsb(bus);
      _state = State::STATE_1;
    }
  }
  else{
    // Try to match the opcode within all the possible categories of instrcutions.
    execute_invalid(bus);
    execute_x8_lsm(bus);
    execute_x16_lsm(bus);
    execute_x8_alu(bus);
    execute_x16_alu(bus);
    execute_control_br(bus);
    execute_control_misc(bus);
  }
}

/** CPU::interrupt_handler
    Handles interrupts before the fetch stage.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
bool Cpu::interrupt_handler(Bus_obj* bus){

  if(_state == State::STATE_1){

    uint8_t IE = read_IE(bus);
    uint8_t IF = read_IF(bus);

    if(IME == 0 or ((IE & IF) == 0)) return false;

    if(_is_halted){
      _is_halted = 0;
      registers.PC++;
    }

    IME = 0;

    if     (IE & IF & 0x01) _interrupt_to_handle = 0;
    else if(IE & IF & 0x02) _interrupt_to_handle = 1;
    else if(IE & IF & 0x04) _interrupt_to_handle = 2;
    else if(IE & IF & 0x08) _interrupt_to_handle = 3;
    else if(IE & IF & 0x10) _interrupt_to_handle = 4;

    write_IF(bus, IF & ~(1 << _interrupt_to_handle));
    _state = State::STATE_I_2;

    return true;
  }
  else if(_state == State::STATE_I_2){
    _state = State::STATE_I_3;

    return true;
  }
  else if(_state == State::STATE_I_3){
    bus->write(--registers.SP, registers.PC >> 8);
    _state = State::STATE_I_4;

    return true;
  }
  else if(_state == State::STATE_I_4){
    bus->write(--registers.SP, registers.PC & 0x00ff);
    _state = State::STATE_I_5;

    return true;
  }
  else if(_state == State::STATE_I_5){

    registers.PC = 0x40 + 0x08 * _interrupt_to_handle;

    _state = State::STATE_1;
    return true;
  }

  return false;

}
