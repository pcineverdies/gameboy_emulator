#include "cpu.h"

/** CPU::CPU
    In charge of initializing the registers with the expected values.

    @param frequency uint32_t Frequency of the cpu

*/
Cpu::Cpu(std::string name, uint32_t frequency) : Bus_obj(name, 0, 0){

  this->set_frequency(frequency);

  _state = State::STATE_1;

  registers.write_A(0x01);
  registers.write_F(0xb0);
  registers.write_B(0x00);
  registers.write_C(0x13);
  registers.write_D(0x00);
  registers.write_E(0xd8);
  registers.write_H(0x01);
  registers.write_L(0x4d);

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

    //print_status(bus);
    _opcode = fetch(bus);

    if(_halt_bug == 1){
      registers.PC--;
      _halt_bug = 0;
    }

  }

  if(_opcode == CB_OPCODE){
    if(_state == State::STATE_1){
      _state = State::STATE_CB_2;
      return;
    }
  }

  if(_state == State::STATE_CB_2 or _state == State::STATE_CB_3 or _state == State::STATE_CB_4){
    if(_state == State::STATE_CB_2){
      _opcode = fetch(bus);
    }
    execute_x8_rsb(bus);
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
    @return bool whether the interrupt is handled or not

*/
bool Cpu::interrupt_handler(Bus_obj* bus){

  // If in fetch stage, check if an interrupt is to be handled or not
  if(_state == State::STATE_1){

    uint8_t IE = read_IE(bus);
    uint8_t IF = read_IF(bus);

    // No interrupt to handle
    if(IME == 0 or ((IE & IF) == 0)) return false;

    // Out from halted state
    if(_is_halted){
      _is_halted = 0;
      registers.PC++;
    }

    IME = 0;

    // TODO: This check should be handled in a later stage of
    // the interrupt. Some games might not work due to this.
    if     (IE & IF & IF_VBLANK) _interrupt_to_handle = 0; // Vblank
    else if(IE & IF & IF_LCD)    _interrupt_to_handle = 1; // LCD
    else if(IE & IF & IF_TIMER)  _interrupt_to_handle = 2; // Timer
    else if(IE & IF & IF_SERIAL) _interrupt_to_handle = 3; // Serial
    else if(IE & IF & IF_JOYPAD) _interrupt_to_handle = 4; // Joypad

    // Remove the interrupt request from IF
    write_IF(bus, IF & ~(1 << _interrupt_to_handle));
    _state = State::STATE_I_2;

    return true;
  }

  // Skip one state
  else if(_state == State::STATE_I_2){
    _state = State::STATE_I_3;

    return true;
  }

  // Write the msbs of PC on the stack
  else if(_state == State::STATE_I_3){
    bus->write(--registers.SP, registers.PC >> 8);
    _state = State::STATE_I_4;

    return true;
  }

  // Write the lsbs of PC on the stack
  else if(_state == State::STATE_I_4){
    bus->write(--registers.SP, registers.PC & 0x00ff);
    _state = State::STATE_I_5;

    return true;
  }

  // Modify the program counter according to the interrupt that is handled.
  else if(_state == State::STATE_I_5){

    registers.PC = 0x40 + 0x08 * _interrupt_to_handle;

    _state = State::STATE_1;
    return true;
  }

  return false;

}
