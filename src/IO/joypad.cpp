#include "joypad.h"
#include <cstdint>

extern struct gb_global_t gb_global;

/** Joypad::read
    Read the JOYP register

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t Joypad::read(uint16_t addr){

  if(addr != 0)
    std::invalid_argument("Invalid address while acessing joypad");

  update_JOYP();
  return JOYP;
}

/** Joypad::write
    Write of the registers

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void Joypad::write(uint16_t addr, uint8_t data){

  if(addr != 0)
    std::invalid_argument("Invalid address while acessing joypad");

  // Only bits 4 and 5 can be set or reset
  if(data & JOYPAD_DP_MASK) JOYP |=  JOYPAD_DP_MASK;
  else                      JOYP &= ~JOYPAD_DP_MASK;

  if(data & JOYPAD_SB_MASK) JOYP |=  JOYPAD_SB_MASK;
  else                      JOYP &= ~JOYPAD_SB_MASK;

}

/** JOYP::JOYP
    Joypad constructor, setting init_addr of the joypad in the memory space
    The size of the object is fixed to 1.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus

*/
Joypad::Joypad(std::string name, uint16_t init_addr) : Bus_obj(name, init_addr, 1){
  JOYP = 0xcf;
}

/** Joypad::step
    Perform the step of the joypad, modifying JOYP and possibly raising
    an interrupt.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void Joypad::step(Bus_obj* bus){
  if(this->update_JOYP()) set_interrupt(bus);
}

/** Joypad::set_interrupt
    Set the corresponding interrupt flag in the IF register.
    In the OOP approach that is followed in the project, first the
    register must be read, modified and written again. In reality,
    this operation is not time consuming, and can be performed during
    the joypad step.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void Joypad::set_interrupt(Bus_obj* bus){
  uint8_t interrupt_flag_value = bus->read(IF_ADDRESS);
  interrupt_flag_value |= IF_JOYPAD;
  bus->write(IF_ADDRESS, interrupt_flag_value);
}

/** key_is_pressed
    Check whether a key is pressed.

    @param ks uint8_t identifier of a key according to SDL_scancode
    @return bool whether the key is pressed or not
*/
bool Joypad::key_is_pressed(uint8_t ks) {
  const Uint8* state = SDL_GetKeyboardState(nullptr);
  SDL_Event e;

  // TODO: Avoid this loop and add counter
  while ((SDL_PollEvent(&e)) != 0){
    if(e.type == SDL_KEYDOWN) break;
  }

  return state[ks];

}

/** Joypad::update_JOYP
    Modify the content of JOYP depending on the current pressed keys.

    @return bool true if a key is pressed, so an interrupt is to be set

*/
bool Joypad::update_JOYP(){

  uint8_t activate_interrupt = 0;
  static int volume_debouncing = 0;

  if(key_is_pressed(JOYPAD_QUIT_BUTTON)){
    gb_global.exit_request = 1;
    return 0;
  }

  if(!(JOYP & JOYPAD_SB_MASK)){
    if(key_is_pressed(JOYPAD_START_BUTTON)) JOYP &= (~JOYPAD_START_MASK), activate_interrupt = 1;
    else                                    JOYP |= ( JOYPAD_START_MASK);

    if(key_is_pressed(JOYPAD_SELECT_BUTTON))  JOYP &= (~JOYPAD_SELECT_MASK), activate_interrupt = 1;
    else                                      JOYP |= ( JOYPAD_SELECT_MASK);

    if(key_is_pressed(JOYPAD_B_BUTTON)) JOYP &= (~JOYPAD_B_MASK), activate_interrupt = 1;
    else                                JOYP |= ( JOYPAD_B_MASK);

    if(key_is_pressed(JOYPAD_A_BUTTON)) JOYP &= (~JOYPAD_A_MASK), activate_interrupt = 1;
    else                                JOYP |= ( JOYPAD_A_MASK);
  }

  if(!(JOYP & (JOYPAD_DP_MASK))){
    if(key_is_pressed(JOYPAD_UP_BUTTON))  JOYP &= (~JOYPAD_UP_MASK), activate_interrupt = 1;
    else                                  JOYP |= ( JOYPAD_UP_MASK);

    if(key_is_pressed(JOYPAD_DOWN_BUTTON))  JOYP &= (~JOYPAD_DOWN_MASK), activate_interrupt = 1;
    else                                    JOYP |= ( JOYPAD_DOWN_MASK);

    if(key_is_pressed(JOYPAD_LEFT_BUTTON)) JOYP &= (~JOYPAD_LEFT_MASK), activate_interrupt = 1;
    else                                   JOYP |= ( JOYPAD_LEFT_MASK);

    if(key_is_pressed(JOYPAD_RIGHT_BUTTON)) JOYP &= (~JOYPAD_RIGHT_MASK), activate_interrupt = 1;
    else                                    JOYP |= ( JOYPAD_RIGHT_MASK);
  }

  // If the buttons for the volume are checked at each step of the joypad, the volume is decreased/increased too fast
  // volume_debouncing avoids this behavior
  if(volume_debouncing == 0){
    if(key_is_pressed(JOYPAD_VOLUME_UP_BUTTON)){
      if(gb_global.volume_amplification != JOYPAD_MAX_VOLUME) gb_global.volume_amplification++;
      volume_debouncing = JOYPAD_VOLUME_DEBOUNCING_DELAY;
      #ifdef __DEBUG
      printf("Current volume: %d%% \n", gb_global.volume_amplification*10);
      #endif
    }
    if(key_is_pressed(JOYPAD_VOLUME_DOWN_BUTTON)){
      if(gb_global.volume_amplification != JOYPAD_MIN_VOLUME) gb_global.volume_amplification--;
      volume_debouncing = JOYPAD_VOLUME_DEBOUNCING_DELAY;
      #ifdef __DEBUG
      printf("Current volume: %d%% \n", gb_global.volume_amplification*10);
      #endif
    }
  }
  else volume_debouncing--;

  return activate_interrupt;
}
