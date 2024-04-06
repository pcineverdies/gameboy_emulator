#include "timer.h"
#include <cstdio>

/** Timer::read
    Read by from the timer at a given address

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t Timer::read(uint16_t addr){

  uint8_t res = 0;

  // Only 8 msbs of DIV are accessible
  if(addr == 0){
    res =  (DIV >> 8);
  }
  else if (addr == 1){
    res =  TIMA;
  }
  else if (addr == 2){
    res =  TMA;
  }
  else if (addr == 3){
    res =  (TAC | 0b11111000);
  }
  else{
    std::invalid_argument("Invalid address while acessing timer");
  }

  return res;
}

/** Timer::write
    Write of the registers

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void Timer::write(uint16_t addr, uint8_t data){

  if(addr == 0){
    DIV = 0;
  }
  else if (addr == 1){
    // No effect if interrupt is raised on the same T-cycle
    if(cycles_to_interrupt == 1) return;

    // Modify the value of TIMA
    TIMA = data;

    // Abort interrupt in case TIMA is written in the 4 T-cycles
    // after an overlow
    if(cycles_to_interrupt > 1) interrupt_aborted = 1;
  }
  else if (addr == 2){
    TMA = data;
  }
  // Only 3 lsbs are used
  else if (addr == 3){
    TAC = (data & 0x07);
  }
  else{
    std::invalid_argument("Invalid address while accessing timer");
  }
}

/** Timer::Timer
    Timer constructor, setting init_addr of the timer in the memory space
    The size of the object is fixed to 4.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus

*/
Timer::Timer(std::string name, uint16_t init_addr) : Bus_obj(name, init_addr, 4){
  TMA = 0;
  TIMA = 0;
  TAC = 0xF8;
  DIV = 0xAB;

  prev_AND_result = 0;
  cycles_to_interrupt = 0;
  interrupt_aborted = 0;
}

/** Timer::step
    Perform the step of the timer at each T-cycle, incrementing DIV and handling
    the other registers according to the TAC content.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void Timer::step(Bus_obj* bus){

  uint8_t bit_position;
  uint8_t bit_position_value;
  uint8_t bit_timer_enable;
  uint8_t current_AND_result;

  DIV += 1;

  bit_position = ((TAC & CLOCK_SELECT_MASK) == 0) ? 9 :
                 ((TAC & CLOCK_SELECT_MASK) == 1) ? 3 :
                 ((TAC & CLOCK_SELECT_MASK) == 2) ? 5 :
                                                    7 ;

  bit_timer_enable   = (TAC >> TIMER_ENABLE_POS) & 0x01;
  bit_position_value = (DIV >>     bit_position) & 0x01;

  // And between the timer enable and nth position of DIV, depending on TAC
  current_AND_result = bit_timer_enable & bit_position_value;

  if(cycles_to_interrupt > 0){

    // Send and interrupt if 4 T-cycles have passed after an overflow and
    // no aborted is obtained
    if(cycles_to_interrupt == 1 and interrupt_aborted == 0){
      set_interrupt(bus);
      TIMA = TMA;
    }
    cycles_to_interrupt--;
  }

  // In case we are not in a `delay for interrupt` situation
  if(cycles_to_interrupt == 0){

    // If falling edge wrt previous T-cycle
    if(prev_AND_result == 1 and current_AND_result == 0){

      // If overflow, enter in `delay for interrupt` mode
      if(TIMA == 0xff){
        cycles_to_interrupt = 4;
        interrupt_aborted = 0;
      }

      // Increment TIMA, possibly with overflow
      TIMA++;
    }
  }

  // Store current result
  prev_AND_result = current_AND_result;
}

/** Timer::set_interrupt
    Set the corresponding interrupt flag in the IF register.
    In the OOP approach that is followed in the project, first the
    register must be read, modified and written again. In reality,
    this operation is not time consuming, and can be performed during
    the timer step.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void Timer::set_interrupt(Bus_obj* bus){
  uint8_t interrupt_flag_value = bus->read(IF_ADDRESS);
  interrupt_flag_value |= IF_TIMER;
  bus->write(IF_ADDRESS, interrupt_flag_value);
}
