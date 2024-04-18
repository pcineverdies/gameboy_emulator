#ifndef __GB_GLOBAL_T_H
#define __GB_GLOBAL_T_H

#include <stdint.h>

/*
 * This object is used to share information across the different
 * objects of the gameboy each time memory itself is not enough
 * */
struct gb_global_t{

  // Whether we are in GBC mode ore not, depending on what happened
  // in the boot of the gameboy
  uint8_t gbc_mode;

  // Amplification factor of the volume. This variables is in the
  // range [0, 10]
  uint8_t volume_amplification;

  // After pressing a button, this terminates the execution of the GB,
  // allowing all the objects to be deallocated correctly
  uint8_t exit_request;

  // Double_speed mode of the system
  uint8_t double_speed;
};

#endif // __GB_GLOBAL_T_H

