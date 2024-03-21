#ifndef __CPU_H
#define __CPU_H

#include "registers.h"
#include "../bus.h"
#include <stdexcept>
#include <stdio.h>


class Cpu{

  enum class State{ STATE_1, STATE_2, STATE_3, STATE_4, STATE_5, STATE_6 };

  // Registers available for the cpu
  Registers registers;

  // Fetch function
  uint8_t fetch(Bus*);


  // Internal registers multi-cycle instructions
  enum State _state;
  uint8_t _opcode;
  uint8_t  _u8;
  uint8_t  _u8_2;
  uint16_t _u16;
  uint16_t _u16_2;
  uint32_t _u32;

  // Decode and execute functions
  void execute_invalid(Bus*);
  void execute_x8_lsm(Bus*);
  void execute_x16_lsm(Bus*);
  void execute_x8_alu(Bus*);
  void execute_x16_alu(Bus*);
  void execute_control_br(Bus*);
  void execute_control_misc(Bus*);
  void execute_x8_rsb(Bus*);

  void internal_cycle(Bus*);

  // Internal functions for common operations
  uint8_t read_x8(Bus*, uint8_t);
  void    write_x8(Bus*, uint8_t, uint8_t);
  bool    get_jump_condition(uint8_t);

public:

  // Constructor
  Cpu();

  // Execute instruction, to run at 1MiHz
  void step(Bus*);

  // Get all the registers (debug purposes)
  Registers get_registers();
  void print_state();

};

#endif // __CPU_H

