#include "APU.h"

/** APU::APU
    APU constructor

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus

*/
APU::APU(std::string name, uint16_t init_addr) : Bus_obj(name, init_addr, APU_REG_N){

  // Init SDL APU
  if(SDL_Init(SDL_INIT_AUDIO) != 0){
    std::runtime_error("APU: SDL_Init failed");
  }

  SDL_zero(audio_spec);
  audio_spec.freq = APU_DSP_FREQUENCY;        // DSP frequency (sample per seconds)
  audio_spec.format = AUDIO_S16SYS;           // Native 16-bits data in native order
  audio_spec.channels = 2;                    // Left and right channel
  audio_spec.samples = APU_AUDIO_BUFFER_SIZE; // Audio buffer size in sample
  audio_spec.callback = nullptr;              // No callback function

  audio_device = SDL_OpenAudioDevice(
    NULL,         // Use most-reasonable default device
    0,            // Use for playback, not recording
    &audio_spec,  // Desired output format as specified above
    NULL,         // Actual output format (?)
    0             // Allowed changes
  );

  // unpausing the audio device (starts playing):
  SDL_PauseAudioDevice(audio_device, 0);

  // Create waveforms for channels 1 and 2
  _wave_duty_table.push_back({0, 0, 0, 0, 0, 0, 0, 1}); // 12.5 %
  _wave_duty_table.push_back({0, 0, 0, 0, 0, 0, 1, 1}); // 25.0 %
  _wave_duty_table.push_back({0, 0, 0, 0, 1, 1, 1, 1}); // 50.0 %
  _wave_duty_table.push_back({1, 1, 1, 1, 1, 1, 0, 0}); // 75.0 %

  // Reset registers
  reset_registers();
}

/** APU::read
    Read by from APU at a given address

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t APU::read(uint16_t addr){
  uint8_t res = 0;

  if(addr >= APU_WPRAM_INIT_ADDR and addr <= APU_WPRAM_END_ADDR){
    res = WPRAM[addr - APU_WPRAM_INIT_ADDR];
  }
  else if(addr == APU_NR10_ADDR){
    res = NR10 | 0x80;
  }
  else if(addr == APU_NR11_ADDR){
    res = NR11 | 0x3F;
  }
  else if(addr == APU_NR12_ADDR){
    res = NR12 | 0x00;
  }
  else if(addr == APU_NR13_ADDR){
    res = NR13 | 0xFF;
  }
  else if(addr == APU_NR14_ADDR){
    res = NR14 | 0xBF;
  }
  else if(addr == APU_NR21_ADDR){
    res = NR21 | 0x3F;
  }
  else if(addr == APU_NR22_ADDR){
    res = NR22 | 0x00;
  }
  else if(addr == APU_NR23_ADDR){
    res = NR23 | 0xFF;
  }
  else if(addr == APU_NR24_ADDR){
    res = NR24 | 0xBF;
  }
  else if(addr == APU_NR30_ADDR){
    res = NR30 | 0x7F;
  }
  else if(addr == APU_NR31_ADDR){
    res = NR31 | 0xFF;
  }
  else if(addr == APU_NR32_ADDR){
    res = NR32 | 0x9F;
  }
  else if(addr == APU_NR33_ADDR){
    res = NR33 | 0xFF;
  }
  else if(addr == APU_NR34_ADDR){
    res = NR34 | 0xBF;
  }
  else if(addr == APU_NR41_ADDR){
    res = NR41 | 0xFF;
  }
  else if(addr == APU_NR42_ADDR){
    res = NR42 | 0x00;
  }
  else if(addr == APU_NR43_ADDR){
    res = NR43 | 0x00;
  }
  else if(addr == APU_NR44_ADDR){
    res = NR44 | 0xBF;
  }
  else if(addr == APU_NR50_ADDR){
    res = NR50;
  }
  else if(addr == APU_NR51_ADDR){
    res = NR51;
  }
  else if(addr == APU_NR52_ADDR){
    res = NR52 | 0x70;
  }
  else{
    res = 0xFF;
  }

  return res;
}

/** write::write
    Write a byte in APU at a given address

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void APU::write(uint16_t addr, uint8_t data){

  uint16_t frequency;
  uint8_t  sweep_period;

  // NR52 is always writable
  // Only bit 7 can be modified, while the msbs are read-only
  if(addr == APU_NR52_ADDR){
    if(data & 0x80){
      NR52 = (NR52 | 0x80);
    }
    else{
      reset_registers();
      NR52 = 0;
      NR10 = 0;
      NR11 = 0;
      NR12 = 0;
      NR13 = 0;
      NR14 = 0;
      NR21 = 0;
      NR22 = 0;
      NR23 = 0;
      NR24 = 0;
      NR30 = 0;
      NR31 = 0;
      NR32 = 0;
      NR33 = 0;
      NR34 = 0;
      NR41 = 0;
      NR42 = 0;
      NR43 = 0;
      NR44 = 0;
      NR50 = 0;
      NR51 = 0;
      NR52 = 0;
    }
    return;
  }

  // If the APU is off, all the registers are read-only
  else if((NR52 & 0x80) == 0) return;

  // In non-CGB consoles, the length registers
  // are always writable, also when the APU is off.
  else if(addr == APU_NR11_ADDR){
    NR11 = data;
  }
  else if(addr == APU_NR21_ADDR){
    NR21 = data;
  }
  else if(addr == APU_NR31_ADDR){
    NR31 = data;
  }
  else if(addr == APU_NR41_ADDR){
    NR41 = data | 0b11000000;
  }

  else if(addr >= APU_WPRAM_INIT_ADDR and addr <= APU_WPRAM_END_ADDR){
    WPRAM[addr - APU_WPRAM_INIT_ADDR] = data;
  }
  else if(addr == APU_NR10_ADDR){
    NR10 = data | 0b10000000;
  }
  else if(addr == APU_NR12_ADDR){
    NR12 = data;
    if((data & 0xF8) == 0) _channel_1_is_enabled = 0;
  }
  else if(addr == APU_NR13_ADDR){
    NR13 = data;
  }
  else if(addr == APU_NR14_ADDR){
    NR14 = data | 0b00111000;

    // During a trigger event, reset registers
    if(NR14 & 0x80){
      _channel_1_is_enabled       = 1;
      _channel_1_envelope_timer   = NR12 & 0x07;
      _channel_1_length_timer     = 64 - (NR11 & 0x3F);
      _channel_1_volume           = (NR12 >> 4) & 0x0f;
      frequency                   = NR13 | ((NR14 & 0x07) << 8);
      _channel_1_frequency        = frequency;
      _channel_1_shadow_frequency = frequency;
      sweep_period                = (NR10 >> 4) & 0x07;
      _channel_1_sweep_timer      = (sweep_period == 0) ? 8 : sweep_period;
      _channel_1_sweep_en         = (sweep_period != 0 or (NR10 & 0x07) != 0) ? 1 : 0;
    }
  }
  else if(addr == APU_NR22_ADDR){
    NR22 = data;
    if((data & 0xF8) == 0) _channel_2_is_enabled = 0;
  }
  else if(addr == APU_NR23_ADDR){
    NR23 = data;
  }
  else if(addr == APU_NR24_ADDR){
    NR24 = data| 0b00111000;

    // During a trigger event, reset registers
    if(NR24 & 0x80){
      _channel_2_is_enabled     = 1;
      _channel_2_envelope_timer = NR22 & 0x07;
      _channel_2_length_timer   = 64 - (NR21 & 0x3F);
      _channel_2_volume         = (NR22 >> 4) & 0x0f;
      frequency                 = NR23 | ((NR24 & 0x07) << 8);
      _channel_2_timer          = (2048 - frequency) * 4;
    }
  }
  else if(addr == APU_NR30_ADDR){
    NR30 = data | 0b01111111;

    // ADC off
    if(NR30 & 0x80) _channel_3_dac_enabled = 1;
    else            _channel_3_dac_enabled = 0;
  }
  else if(addr == APU_NR32_ADDR){
    NR32 = data | 0b10011111;
  }
  else if(addr == APU_NR33_ADDR){
    NR33 = data;
  }
  else if(addr == APU_NR34_ADDR){
    NR34 = data | 0b00111000;

    // During a trigger event, reset registers
    if(NR34 & 0x80 and _channel_3_dac_enabled){
      _channel_3_is_enabled     = 1;
      frequency                 = NR33 | ((NR34 & 0x07) << 8);
      _channel_3_timer          = (2048 - frequency) * 2;
      _channel_3_current_sample = 1;
      _channel_3_length_timer   = 256 - NR31;
    }
  }
  else if(addr == APU_NR42_ADDR){
    NR42 = data;
    if((data & 0xF8) == 0) _channel_4_is_enabled = 0;
  }
  else if(addr == APU_NR43_ADDR){
    NR43 = data;
  }
  else if(addr == APU_NR44_ADDR){
    NR44 = data | 0b00111111;

    // During a trigger event, reset registers
    if(data & 0x80){
      _channel_4_is_enabled       = 1;
      _channel_4_timer            = channel_4_get_period();
      _channel_4_length_timer     = 64 - (NR41 & 0x3f);
      _channel_4_volume           = (NR42 >> 4) & 0x0f;
      _channel_4_envelope_timer   = NR42 & 0x07;
      _channel_4_LSFR             = 0;
    }
  }
  else if(addr == APU_NR50_ADDR){
    NR50 = data;
  }
  else if(addr == APU_NR51_ADDR){
    NR51 = data;
  }
  else{
    std::invalid_argument("Incorrect address for APU\n");
  }
}

/** APU::step
    Perform the step of the APU at each T-cycle.

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void APU::step(Bus_obj* bus){

  uint16_t channel_1_output = 0;
  uint16_t channel_2_output = 0;
  uint16_t channel_3_output = 0;
  uint16_t channel_4_output = 0;
  uint16_t apu_sample_left  = 0;
  uint16_t apu_sample_right = 0;
  uint8_t  apu_left_volume  = (NR50 >> 4) & 0x07;
  uint8_t  apu_right_volume = (NR50     ) & 0x07;

  // If APU is disabled, skip the computation of current samples and provide
  // 0 as values. The audio cannot be skipped in order to mantain stable the framerate
  if((NR52 & 0x80) != 0){

    // Frame sequencer handler using timer DIV register
    _current_DIV_value = bus->read(MMU_TIMER_INIT_ADDR);

    // Compute steps of sweep, envelope and length functions, with resepct to a
    // falling edge of bit 5 of DIV.
    // -> Envlope is updated with 1/8 of frequency
    // -> Length  is updated with 1/2 of frequency
    // -> Sweep   is updated with 1/4 of frequency
    _envelope_step = 0;
    _sweep_step = 0;
    _length_step = 0;

    if((_previous_DIV_value & (1 << 5)) and !(_current_DIV_value & (1 << 5))){
      _frame_sequencer++;
      _length_step   = ((_frame_sequencer % 2) == 0) ? 1 : 0;
      _sweep_step    = ((_frame_sequencer % 4) == 0) ? 1 : 0;
      _envelope_step = ((_frame_sequencer % 8) == 0) ? 1 : 0;
    }

    // Store previous DIV value for next iteration
    _previous_DIV_value = _current_DIV_value;

    // Get channels outputs
    channel_1_output = channel_1_handler();
    channel_2_output = channel_2_handler();
    channel_3_output = channel_3_handler();
    channel_4_output = channel_4_handler();

    // Sound panning left with volume
    apu_sample_left =  ((NR51 & 0x80) ? channel_4_output / 4 : 0) +
                       ((NR51 & 0x40) ? channel_3_output / 4 : 0) +
                       ((NR51 & 0x20) ? channel_2_output / 4 : 0) +
                       ((NR51 & 0x10) ? channel_1_output / 4 : 0) ;

    apu_sample_left = apu_sample_left * (apu_left_volume + 1) / 8;

    // Sound panning right with volume
    apu_sample_right =  ((NR51 & 0x08) ? channel_4_output / 4 : 0) +
                        ((NR51 & 0x04) ? channel_3_output / 4 : 0) +
                        ((NR51 & 0x02) ? channel_2_output / 4 : 0) +
                        ((NR51 & 0x01) ? channel_1_output / 4 : 0) ;

    apu_sample_right = apu_sample_right * (apu_right_volume + 1) / 8;
  }

  // Downsampling: use one sample each APU_BUS_FREQUENCY / APU_DSP_FREQUENCY
  _audio_buffer_downsampling_counter += APU_DSP_FREQUENCY;
  if(_audio_buffer_downsampling_counter > APU_BUS_FREQUENCY){
    _audio_buffer_downsampling_counter -= APU_BUS_FREQUENCY;

    // Store samples in the buffer using LR order
    _audio_buffer[_audio_buffer_counter++] = apu_sample_left;
    _audio_buffer[_audio_buffer_counter++] = apu_sample_right;

    // When the buffer is full, send the samples to the speaker functino
    if(_audio_buffer_counter == APU_AUDIO_BUFFER_SIZE){
      _audio_buffer_counter = 0;

      // Audio sync: This allows the audio to be synchronized with the screen, by
      // running at almost 60 FPS
      while ((SDL_GetQueuedAudioSize(audio_device)) > APU_AUDIO_BUFFER_SIZE * 2) SDL_Delay(1);
      SDL_QueueAudio(audio_device, _audio_buffer, APU_AUDIO_BUFFER_SIZE * 2);
    }
  }

  // Adjust NR52 according to which channels are working
  if(_channel_1_is_enabled) NR52 |= 0b00000001;
  else                      NR52 &= 0b11111110;
  if(_channel_2_is_enabled) NR52 |= 0b00000010;
  else                      NR52 &= 0b11111101;
  if(_channel_3_is_enabled) NR52 |= 0b00000100;
  else                      NR52 &= 0b11111011;
  if(_channel_4_is_enabled) NR52 |= 0b00001000;
  else                      NR52 &= 0b11110111;

}

/** APU::~APU
    Destroy the APU

*/
APU::~APU(){
  SDL_CloseAudioDevice(audio_device);
  SDL_Quit();
}

/** APU::channel_1_handler
    Handle the step for channel 1, generating the corresponding sample in the range
    [0, 15 * APU_AMPLITUDE_SCALING]

    @return uint16_t sample from channel 1

*/
uint16_t APU::channel_1_handler(){

  // Duty cycle to use for the wave function
  uint8_t  duty_cycle     = (NR11 >> 6) & 0x03;
  // Timer to use for the envelope function to use
  uint8_t  envelope_timer = (NR12 >> 0) & 0x07;
  // Envelope direction
  uint8_t  envelope_dir   = (NR12 >> 3) & 0x01;
  // Sweep period
  uint8_t  sweep_period   = (NR10 >> 4) & 0x07;
  // Sweep direction
  uint8_t  sweep_dir      = (NR10 >> 3) & 0x01;
  // Step for sweep
  uint8_t  sweep_step     = (NR10 >> 0) & 0x07;
  // Amplitude to use
  uint16_t amplitude      = 0;
  // Next frequency to use due to sweep
  uint16_t new_frequency  = 0;

  if(!_channel_1_is_enabled) return 0;

  // Reset period and step the element to use in the waveform
  if(--_channel_1_timer == 0){
    _channel_1_timer = (2048 - _channel_1_frequency) * 4;
    _channel_1_wave_duty_position = (_channel_1_wave_duty_position + 1) % 8;
  }

  // Compute current amplitude for the channel
  amplitude = _wave_duty_table[duty_cycle][_channel_1_wave_duty_position] * APU_AMPLITUDE_SCALING;

  // Stop timer when length timer expires
  if(_length_step and (NR14 & 0x40)){
    if(--_channel_1_length_timer == 0){
      _channel_1_is_enabled = 0;
    }
  }

  // Perform envelope step
  if(_envelope_step and envelope_timer != 0){
    if(--_channel_1_envelope_timer == 0){
      _channel_1_envelope_timer = envelope_timer;

      // Either increment or decrement volume depending on the current volume and direction
      if(_channel_1_volume != 0x0f and  envelope_dir) _channel_1_volume++;
      if(_channel_1_volume != 0x00 and !envelope_dir) _channel_1_volume--;
    }
  }

  // Perform sweep step
  if(_sweep_step){
    if(--_channel_1_sweep_timer == 0){

      // Reset sweep timer
      _channel_1_sweep_timer = (sweep_period == 0) ? 8 : sweep_period;

      if(_channel_1_sweep_en and sweep_period > 0){
        new_frequency = _channel_1_shadow_frequency >> sweep_step;
        if(sweep_dir) new_frequency = _channel_1_shadow_frequency - new_frequency;
        else          new_frequency = _channel_1_shadow_frequency + new_frequency;

        if(new_frequency > 2048 or new_frequency == 0) _channel_1_is_enabled = 0;

        // Update frequency
        if(_channel_1_is_enabled and _sweep_step > 0){
          _channel_1_frequency        = new_frequency;
          _channel_1_shadow_frequency = new_frequency;
        }
      }
    }
  }

  return amplitude * _channel_1_volume;
}

/** APU::channel_2_handler
    Handle the step for channel 2, generating the corresponding sample in the range
    [0, 15 * APU_AMPLITUDE_SCALING]

    @return uint16_t sample from channel 2

*/
uint16_t APU::channel_2_handler(){

  // Duty cycle to use for the wave function
  uint8_t  duty_cycle           = (NR21 >> 6) & 0x03;
  // Initial frequency to use (no sweep function in channel 2)
  uint16_t frequency            = NR23 | ((NR24 & 0x07) << 8);
  // Timer to use
  uint16_t frequency_timer_init = (2048 - frequency) * 4;
  // Amplitude to use
  uint16_t amplitude            = 0;
  // Timer to use for the envelope function
  uint8_t  envelope_timer       = NR22 & 0x07;
  // Envelope direction
  uint8_t  envelope_dir         = (NR22 >> 3) & 0x01;

  if(!_channel_2_is_enabled) return 0;

  // Reset period and step the element to use in the waveform
  if(--_channel_2_timer == 0){
    _channel_2_timer = frequency_timer_init;
    _channel_2_wave_duty_position = (_channel_2_wave_duty_position + 1) % 8;
  }

  // Compute current amplitude for the channel
  amplitude = _wave_duty_table[duty_cycle][_channel_2_wave_duty_position] * APU_AMPLITUDE_SCALING;

  // Stop timer when length timer expires
  if(_length_step and (NR24 & 0x40)){
    if(--_channel_2_length_timer == 0){
      _channel_2_is_enabled = 0;
    }
  }

  // Perform envelope step
  if(_envelope_step and envelope_timer != 0){
    if(--_channel_2_envelope_timer == 0){
      _channel_2_envelope_timer = envelope_timer;

      // Either increment or decrement volume depending on the current volume and direction
      if(_channel_2_volume != 0x0f and  envelope_dir) _channel_2_volume++;
      if(_channel_2_volume != 0x00 and !envelope_dir) _channel_2_volume--;
    }
  }

  return amplitude * _channel_2_volume;
}

/** APU::channel_3_handler
    Handle the step for channel 3, generating the corresponding sample in the range
    [0, 15 * APU_AMPLITUDE_SCALING]

    @return uint16_t sample from channel 3

*/
uint16_t APU::channel_3_handler(){

  // Current volume
  uint8_t  output_level           = (NR32 >> 5) & 0x03;
  // Initial frequency to use (no sweep function in channel 3)
  uint16_t frequency              = NR33 | ((NR34 & 0x07) << 8);
  // Timer to use
  uint16_t frequency_timer_init   = (2048 - frequency) * 2;
  // Amplitude to use
  uint16_t amplitude              = 0;

  if(!_channel_3_is_enabled) return 0;

  // Reset period and step the element to use in the waveform
  if(--_channel_3_timer == 0){
    _channel_3_timer          = frequency_timer_init;
    _channel_3_current_sample = (_channel_3_current_sample + 1) % 32;
  }

  // Compute current amplitude for the channel (for each sample, first
  // use the upper nibble, then the lower one)
  amplitude = WPRAM[_channel_3_current_sample / 2];
  if(_channel_3_current_sample % 2 == 0) amplitude = (amplitude >> 4) & 0x0f;
  else                                   amplitude =  amplitude       & 0x0f;

  // Modify the amplitude according to the output volume
  amplitude = (output_level == 0b00) ? 0              :
              (output_level == 0b01) ? amplitude      :
              (output_level == 0b10) ? amplitude >> 1 :
                                       amplitude >> 2 ;

  // Stop timer when length timer expires
  if(_length_step and (NR34 & 0x40)){
    if(--_channel_3_length_timer == 0){
      _channel_3_is_enabled = 0;
    }
  }

  // Return 0 if DAC is disabled
  return amplitude * APU_AMPLITUDE_SCALING * _channel_3_dac_enabled;

}
/** APU::channel_4_handler
    Handle the step for channel 4, generating the corresponding sample in the range
    [0, 15 * APU_AMPLITUDE_SCALING]

    @return uint16_t sample from channel 4

*/
uint16_t APU::channel_4_handler(){

  // Timer to use for the envelope function
  uint8_t  envelope_timer = NR42 & 0x07;
  // Envelope direction
  uint8_t  envelope_dir   = (NR42 >> 3) & 0x01;
  // Stores the xor result for the LSFR step
  uint8_t  xor_result     = 0;
  // Amplitude to use
  uint8_t  amplitude      = 0;

  if(!_channel_4_is_enabled) return 0;

  // Reset period and step the LSFR
  if(--_channel_4_timer == 0){
    _channel_4_timer = channel_4_get_period();

    // Compute LSFR new bit and shift
    xor_result = (~((_channel_4_LSFR & 1) ^ ((_channel_4_LSFR >> 1) & 1))) & 1;
    _channel_4_LSFR = (_channel_4_LSFR >> 1) | (xor_result << 14);

    // Update bit 6 if small mode is enabled
    if(NR43 & 0x04){
      _channel_4_LSFR &= ~(1 << 6);
      _channel_4_LSFR |= xor_result << 6;
    }
  }

  // LSB of the LSFR corresponds to the amplitude to use
  amplitude = (_channel_4_LSFR & 1) * APU_AMPLITUDE_SCALING;

  // Stop timer when length timer expires
  if(_length_step and (NR44 & 0x40)){
    _channel_4_length_timer--;
    if(_channel_4_length_timer == 0){
      _channel_4_is_enabled = 0;
    }
  }

  // Perform envelope step
  if(_envelope_step and envelope_timer != 0){
    if(--_channel_4_envelope_timer == 0){
      _channel_4_envelope_timer = envelope_timer;

      if(_channel_4_volume != 0x0f and  envelope_dir) _channel_4_volume++;
      if(_channel_4_volume != 0x00 and !envelope_dir) _channel_4_volume--;
    }
  }

  return amplitude * _channel_4_volume;
}

/** APU::reset_registers
    Reset the content of the registers of the APU. This is required
    each time the APU turns on, either at startup or after the APU
    was manually turned off.

*/
void APU::reset_registers(){

  NR10 = 0x80;
  NR11 = 0xBF;
  NR12 = 0xF3;
  NR13 = 0xFF;
  NR14 = 0xBF;

  NR21 = 0x3F;
  NR22 = 0x00;
  NR23 = 0xFF;
  NR24 = 0xBF;

  NR30 = 0x7F;
  NR31 = 0xFF;
  NR32 = 0x9F;
  NR33 = 0xFF;
  NR34 = 0xBF;

  NR41 = 0xFF;
  NR42 = 0x00;
  NR43 = 0x00;
  NR44 = 0xFB;

  NR50 = 0x77;
  NR51 = 0xF3;
  NR52 = 0xF1;

  _previous_DIV_value = 0;
  _current_DIV_value = 0;
  _frame_sequencer = 0;

  _channel_1_is_enabled = 0;
  _channel_1_envelope_timer = 0;
  _channel_1_volume = 0;
  _channel_1_sweep_en = 0;
  _channel_1_sweep_timer = 0;
  _channel_1_shadow_frequency = 0;
  _channel_1_frequency = 0;
  _channel_1_timer = 0;
  _channel_1_wave_duty_position = 0;
  _channel_1_length_timer = 0;

  _channel_2_is_enabled = 0;
  _channel_2_envelope_timer = 0;
  _channel_2_volume = 0;
  _channel_2_timer = 0;
  _channel_2_wave_duty_position = 0;
  _channel_2_length_timer = 0;

  _channel_3_is_enabled = 0;
  _channel_3_current_sample = 0;
  _channel_3_timer = 0;
  _channel_3_length_timer = 0;
  _channel_3_dac_enabled = 0;

  _channel_4_is_enabled = 0;
  _channel_4_envelope_timer = 0;
  _channel_4_timer = 0;
  _channel_4_length_timer = 0;
  _channel_4_volume = 0;
  _channel_4_LSFR = 0;

  _audio_buffer_counter = 0;
  _audio_buffer_downsampling_counter = 0;

  memset((void*) _audio_buffer, 0, APU_AUDIO_BUFFER_SIZE * sizeof(uint16_t));
}

/** APU::channel_4_get_period
    Compute the period to use for channel 4 according to the content
    of register NR43

    @return uint16_t period to use
*/
uint16_t APU::channel_4_get_period(){
  uint16_t res          = 0;
  uint8_t clock_divider = NR43 & 0x07;
  uint8_t clock_shift   = (NR43 >> 4) & 0x0f;

  res = (clock_divider == 0b000) ? 8   :
        (clock_divider == 0b001) ? 16  :
        (clock_divider == 0b010) ? 32  :
        (clock_divider == 0b011) ? 48  :
        (clock_divider == 0b100) ? 64  :
        (clock_divider == 0b101) ? 80  :
        (clock_divider == 0b110) ? 96  :
                                   112 ;

  return res << clock_shift;
}

