#ifndef __CPU_H
#define __CPU_H

#include "registers.h"
#include "../bus.h"
#include <stdexcept>
#include <stdio.h>

class Cpu{

  // Registers available for the cpu
  Registers registers;

  // Fetch function
  uint8_t fetch(Bus*);

  // Decode and execute functions
  void execute_invalid(Bus*, uint8_t);
  void execute_x8_lsm(Bus*, uint8_t);
  void execute_x16_lsm(Bus*, uint8_t);
  void execute_x8_alu(Bus*, uint8_t);
  void execute_x16_alu(Bus*, uint8_t);
  void execute_control_br(Bus*, uint8_t);
  void execute_control_misc(Bus*, uint8_t);
  void execute_x8_rsb(Bus*, uint8_t);

  void internal_cycle(Bus*);

  uint8_t read_x8(Bus*, uint8_t);
  void    write_x8(Bus*, uint8_t, uint8_t);

public:

  // Constructor
  Cpu();

  // Execute instruction, to run at 1MiHz
  void step(Bus*);

  // Get all the registers (debug purposes)
  Registers get_registers();

};

#endif // __CPU_H

