#ifndef __TIMER_H
#define __TIMER_H

#include "../bus/bus_obj.h"
#include "../memory/memory_map.h"
#include <cstdint>
#include <string>
#include <stdexcept>

#define CLOCK_SELECT_MASK 0x03
#define TIMER_ENABLE_POS 2

class Timer : public Bus_obj {

  uint16_t DIV;
  uint8_t TIMA;
  uint8_t TMA;
  uint8_t TAC;

  uint8_t prev_AND_result;
  uint8_t cycles_to_interrupt;
  uint8_t interrupt_aborted;

public:

  Timer(std::string, uint16_t);
  uint8_t read(uint16_t);
  void    write(uint16_t, uint8_t);
  void    step(Bus_obj*);
  void    set_interrupt(Bus_obj*);

};

#endif // !__TIMER_H
