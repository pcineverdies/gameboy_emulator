#ifndef __CPU_H
#define __CPU_H

#include "opcode.h"
#include "registers.h"
#include "../memory/memory_map.h"
#include "../bus/bus.h"
#include "../bus/bus_obj.h"
#include <stdexcept>
#include <stdio.h>
#include <cstring>


class Cpu : public Bus_obj{

  enum class State{
    // Standard execution states
    STATE_1, STATE_2, STATE_3, STATE_4, STATE_5, STATE_6,

    // Interrupt handling states
    STATE_I_2, STATE_I_3, STATE_I_4, STATE_I_5,

    // CB handling states
    STATE_CB_2, STATE_CB_3, STATE_CB_4
  };

  // Registers available for the cpu
  Registers registers;

  // IME flags and variables to handle interrupts and halt state
  uint8_t IME;
  uint8_t _ei_delayed;
  uint8_t _is_halted;
  uint8_t _halt_bug;


  // Internal registers multi-cycle instructions
  enum State _state;
  uint8_t    _opcode;
  uint8_t    _u8;
  uint8_t    _u8_2;
  uint16_t   _u16;
  uint16_t   _u16_2;
  uint32_t   _u32;
  uint8_t    _interrupt_to_handle;

  // Fetch function
  uint8_t fetch(Bus_obj*);

  // Decode and execute functions
  void execute_invalid(Bus_obj*);
  void execute_x8_lsm(Bus_obj*);
  void execute_x16_lsm(Bus_obj*);
  void execute_x8_alu(Bus_obj*);
  void execute_x16_alu(Bus_obj*);
  void execute_control_br(Bus_obj*);
  void execute_control_misc(Bus_obj*);
  void execute_x8_rsb(Bus_obj*);
  bool interrupt_handler(Bus_obj*);
  void halt_handler(Bus_obj*);

  // Internal functions for common operations
  uint8_t read_x8(Bus_obj*, uint8_t);
  void    write_x8(Bus_obj*, uint8_t, uint8_t);
  bool    get_jump_condition(uint8_t);
  uint8_t get_xx(uint8_t);
  uint8_t get_yyy(uint8_t);
  uint8_t get_zzz(uint8_t);
  bool    check_mask(uint8_t, std::string);
  void    print_status(Bus_obj*);
  void    print_serial(Bus_obj*);

  uint8_t read_IE(Bus_obj*);
  uint8_t read_IF(Bus_obj*);
  void write_IE(Bus_obj*, uint8_t);
  void write_IF(Bus_obj*, uint8_t);

  // ALU instructions
  uint8_t inc_dec_x8(uint8_t, uint8_t);
  uint8_t add_x8(uint8_t, uint8_t);
  uint8_t adc_x8(uint8_t, uint8_t);
  uint8_t sub_x8(uint8_t, uint8_t);
  uint8_t sbc_x8(uint8_t, uint8_t);
  uint8_t and_x8(uint8_t, uint8_t);
  uint8_t xor_x8(uint8_t, uint8_t);
  uint8_t or_x8(uint8_t, uint8_t);
  uint8_t cp_x8(uint8_t, uint8_t);

public:

  // Constructor
  Cpu(std::string, uint32_t);

  // Execute instruction
  void step(Bus_obj*);

  // Compliance with parent class, not employed
  uint8_t read(uint16_t a){return 0;}
  void write(uint16_t a, uint8_t d){}

  // Get all the registers (debug purposes)
  Registers get_registers();

};

#endif // __CPU_H

