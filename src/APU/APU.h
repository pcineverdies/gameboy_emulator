#ifndef __APU_H
#define __APU_H

#include "../bus/bus_obj.h"
#include "APU_def.h"
#include <cstdint>
#include <vector>
#include <stdexcept>
#include "../memory/memory_map.h"
#include "../utils/gb_global_t.h"
#include <SDL.h>

#define APU_AUDIO_BUFFER_SIZE 2400
#define APU_DSP_FREQUENCY     48000
#define APU_BUS_FREQUENCY     4800000
#define APU_AMPLITUDE_SCALING 100

class APU : public Bus_obj {

  // APU Registers

  // Channel 1
  uint8_t NR10;
  uint8_t NR11;
  uint8_t NR12;
  uint8_t NR13;
  uint8_t NR14;

  // Channel 2
  uint8_t NR21;
  uint8_t NR22;
  uint8_t NR23;
  uint8_t NR24;

  // Channel 3
  uint8_t NR30;
  uint8_t NR31;
  uint8_t NR32;
  uint8_t NR33;
  uint8_t NR34;

  // Channel 4
  uint8_t NR41;
  uint8_t NR42;
  uint8_t NR43;
  uint8_t NR44;

  // Global control registers
  uint8_t NR52;
  uint8_t NR51;
  uint8_t NR50;

  // Wave pattern RAM
  uint8_t WPRAM[16];

  // APU Internal variables
  uint16_t _audio_buffer[APU_AUDIO_BUFFER_SIZE];
  uint16_t _audio_buffer_counter;
  uint32_t _audio_buffer_downsampling_counter;
  std::vector<std::vector<uint8_t>> _wave_duty_table;
  uint8_t _previous_DIV_value;
  uint8_t _current_DIV_value;

  uint8_t _length_step;
  uint8_t _envelope_step;
  uint8_t _sweep_step;
  uint8_t _frame_sequencer;

  // Channel 1 internal variables
  uint8_t  _channel_1_is_enabled;
  uint8_t  _channel_1_envelope_timer;
  uint8_t  _channel_1_volume;
  uint8_t  _channel_1_sweep_en;
  uint8_t  _channel_1_sweep_timer;
  uint16_t _channel_1_shadow_frequency;
  uint16_t _channel_1_frequency;
  uint16_t _channel_1_timer;
  uint16_t _channel_1_wave_duty_position;
  uint16_t _channel_1_length_timer;

  // Channel 2 internal variables
  uint8_t  _channel_2_is_enabled;
  uint8_t  _channel_2_envelope_timer;
  uint8_t  _channel_2_volume;
  uint16_t _channel_2_timer;
  uint16_t _channel_2_wave_duty_position;
  uint16_t _channel_2_length_timer;

  // Channel 3 internal variables
  uint8_t  _channel_3_is_enabled;
  uint8_t  _channel_3_current_sample;
  uint16_t _channel_3_timer;
  uint16_t _channel_3_length_timer;
  uint8_t  _channel_3_dac_enabled;

  // Channel 4 internal variables
  uint8_t  _channel_4_is_enabled;
  uint8_t  _channel_4_envelope_timer;
  uint16_t _channel_4_timer;
  uint16_t _channel_4_length_timer;
  uint8_t  _channel_4_volume;
  uint16_t _channel_4_LSFR;

  SDL_AudioDeviceID audio_device;
  SDL_AudioSpec audio_spec;

  // APU Internal functions
  uint16_t channel_1_handler();
  uint16_t channel_2_handler();
  uint16_t channel_3_handler();
  uint16_t channel_4_handler();
  void     reset_registers();
  uint16_t channel_4_get_period();

public:

  APU(std::string, uint16_t);
  uint8_t read(uint16_t);
  void    write(uint16_t, uint8_t);
  void    step(Bus_obj*);
  ~APU();

};

#endif // !__APU_H
