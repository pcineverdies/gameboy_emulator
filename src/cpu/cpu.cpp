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

  if(_state == State::STATE_1){
    _opcode = fetch(bus);
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
    // Try to match the  within all the possible categories of instrcutions.
    execute_invalid(bus);
    execute_x8_lsm(bus);
    execute_x16_lsm(bus);
    execute_x8_alu(bus);
    execute_x16_alu(bus);
    execute_control_br(bus);
    execute_control_misc(bus);
  }
}

