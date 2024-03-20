#ifndef __REGISTERS_H
#define __REGISTERS_H

#include <cstdint>
#include <stdexcept>

// Class to collect all the registers of the CPU,
// together with some utilities functions
class Registers{

public:

  // GP Registers
  uint8_t registers[8];

  // Special registers
  uint16_t SP; uint16_t PC;

  // Utils functions
  uint8_t read_B();
  void    write_B(uint8_t);

  uint8_t read_C();
  void    write_C(uint8_t);

  uint16_t read_BC();
  void    write_BC(uint16_t);

  uint8_t read_D();
  void    write_D(uint8_t);

  uint8_t read_E();
  void    write_E(uint8_t);

  uint16_t read_DE();
  void    write_DE(uint16_t);

  uint8_t read_H();
  void    write_H(uint8_t);

  uint8_t read_L();
  void    write_L(uint8_t);

  uint16_t read_HL();
  uint16_t read_HL_i();
  uint16_t read_HL_d();
  void     write_HL(uint16_t);

  uint8_t read_F();
  void    write_F(uint8_t);

  uint8_t read_A();
  void    write_A(uint8_t);

  uint8_t get_Z();
  void    set_Z(uint8_t);

  uint8_t get_N();
  void    set_N(uint8_t);

  uint8_t get_H();
  void    set_H(uint8_t);

  uint8_t get_C();
  void    set_C(uint8_t);

};

#endif // !__REGISTERS_H

