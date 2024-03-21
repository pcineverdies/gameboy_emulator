#include "registers.h"

/** Registers::read_B
    Return the value of B

    @return uint8_t read byte

*/
uint8_t Registers::read_B(){
  return registers[0];
}

/** Registers::write_B
    Modify the value of B

    @param data uint8_t data to write

*/
void Registers::write_B(uint8_t data){
  registers[0] = data;
}

/** Registers::read_C
    Return the value of C

    @return uint8_t read byte

*/
uint8_t Registers::read_C(){
  return registers[1];
}

/** Registers::write_C
    Modify the value of C

    @param data uint8_t data to write

*/
void Registers::write_C(uint8_t data){
  registers[1] = data;
}

/** Registers::read_BC
    Return the value of BC

    @return uint16_t read short

*/
uint16_t Registers::read_BC(){
  return (registers[0] << 8) | registers[1];
}

/** registers::write_BC
    modifies the value of BC

    @param data uint16_t data to write

*/
void Registers::write_BC(uint16_t data){
  registers[0] = data >> 8;
  registers[1] = data & 0x00ff;
}

/** Registers::read_D
    Return the value of D

    @return uint8_t read byte

*/
uint8_t Registers::read_D(){
  return registers[2];
}

/** Registers::write_D
    Modify the value of D

    @param data uint8_t data to write

*/
void Registers::write_D(uint8_t data){
  registers[2] = data;
}

/** Registers::read_E
    Return the value of e

    @return uint8_t read byte

*/
uint8_t Registers::read_E(){
  return registers[3];
}

/** Registers::write_E
    Modify the value of E

    @param data uint8_t data to write

*/
void Registers::write_E(uint8_t data){
  registers[3] = data;
}

/** Registers::read_DE
    Return the value of DE

    @return uint16_t read short

*/
uint16_t Registers::read_DE(){
  return (registers[2] << 8) | registers[3];
}

/** registers::write_DE
    modifies the value of DE

    @param data uint16_t data to write

*/
void Registers::write_DE(uint16_t data){
  registers[2] = data >> 8;
  registers[3] = data & 0x00ff;
}

/** Registers::read_H
    Return the value of H

    @return uint8_t read byte

*/
uint8_t Registers::read_H(){
  return registers[4];
}

/** Registers::write_H
    Modify the value of H

    @param data uint8_t data to write

*/
void Registers::write_H(uint8_t data){
  registers[4] = data;
}

/** Registers::read_L
    Return the value of L

    @return uint8_t read byte

*/

uint8_t Registers::read_L(){
  return registers[5];
}

/** Registers::write_L
    Modify the value of L

    @param data uint8_t data to write

*/
void Registers::write_L(uint8_t data){
  registers[5] = data;
}

/** Registers::read_HL
    Return the value of HL

    @return uint16_t read short

*/
uint16_t Registers::read_HL(){
  return (registers[4] << 8) | registers[5];
}

/** Registers::read_HL_i
    Return the value of HL and then increment it

    @return uint16_t read short

*/
uint16_t Registers::read_HL_i(){
  uint16_t res = read_HL();
  write_HL(res+1);
  return res;
}

/** Registers::read_HL_d
    Return the value of HL and then decrement it

    @return uint16_t read short

*/
uint16_t Registers::read_HL_d(){
  uint16_t res = read_HL();
  write_HL(res-1);
  return res;
}

/** registers::write_HL
    modifies the value of HL

    @param data uint16_t data to write

*/
void Registers::write_HL(uint16_t data){
  registers[4] = data >> 8;
  registers[5] = data & 0x00ff;
}

/** Registers::read_F
    Return the value of F

    @return uint8_t read byte

*/
uint8_t Registers::read_F(){
  return registers[6];
}

/** Registers::write_F
    Modify the value of F

    @param data uint8_t data to write

*/
void Registers::write_F(uint8_t data){
  registers[6] = data;
}

/** Registers::read_A
    Return the value of A

    @return uint8_t read byte

*/
uint8_t Registers::read_A(){
  return registers[7];
}

/** Registers::write_A
    Modify the value of A

    @param data uint8_t data to write

*/
void Registers::write_A(uint8_t data){
  registers[7] = data;
}

/** Registers::get_Z
    Get the value of the Z flag

    @return uint8_t read value

*/
uint8_t Registers::get_Z(){
  return read_F() & (1 << 7) ? 1 : 0;
}

/** Registers::set_Z
    Set the value of the Z flag

    @param data uint8_t Reset if data is 0, otherwise set

*/
void Registers::set_Z(uint8_t data){
  if(!data) registers[6] = registers[6] & ~(1 << 7);
  else      registers[6] = registers[6] |  (1 << 7);
}

/** Registers::get_N
    Get the value of the N flag

    @return uint8_t read value

*/
uint8_t Registers::get_N(){
  return read_F() & (1 << 6) ? 1 : 0;
}

/** Registers::set_N
    Set the value of the N flag

    @param data uint8_t Reset if data is 0, otherwise set

*/
void Registers::set_N(uint8_t data){
  if(!data) registers[6] = registers[6] & ~(1 << 6);
  else      registers[6] = registers[6] |  (1 << 6);
}

/** Registers::get_H
    Get the value of the H flag

    @return uint8_t read value

*/
uint8_t Registers::get_H(){
  return read_F() & (1 << 5) ? 1 : 0;
}

/** Registers::set_H
    Set the value of the H flag

    @param data uint8_t Reset if data is 0, otherwise set

*/
void Registers::set_H(uint8_t data){
  if(!data) registers[6] = registers[6] & ~(1 << 5);
  else      registers[6] = registers[6] |  (1 << 5);
}

/** Registers::get_C
    Get the value of the C flag

    @return uint8_t read value

*/
uint8_t Registers::get_C(){
  return read_F() & (1 << 4) ? 1 : 0;
}

/** Registers::set_C
    Set the value of the C flag

    @param data uint8_t Reset if data is 0, otherwise set

*/
void Registers::set_C(uint8_t data){
  if(!data) registers[6] = registers[6] & ~(1 << 4);
  else      registers[6] = registers[6] |  (1 << 4);
}

