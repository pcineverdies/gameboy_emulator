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

  _wave_duty_table.push_back({0, 0, 0, 0, 0, 0, 0, 1}); // 12.5 %
  _wave_duty_table.push_back({0, 0, 0, 0, 0, 0, 1, 1}); // 25.0 %
  _wave_duty_table.push_back({0, 0, 0, 0, 1, 1, 1, 1}); // 50.0 %
  _wave_duty_table.push_back({1, 1, 1, 1, 1, 1, 0, 0}); // 75.0 %

  _previous_DIV_value = 0;
  _current_DIV_value = 0;

  _channel_2_is_enabled = 0;
  _channel_2_timer = 0;
  _channel_2_wave_duty_position = 0;
  _channel_2_frame_sequencer = 0;
  _channel_2_length_timer = 0;

  _audio_buffer_counter = 0;
  _audio_buffer_downsampling_counter = 0;
  memset((void*) _audio_buffer, 0, APU_AUDIO_BUFFER_SIZE * sizeof(uint16_t));
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
    res = NR10;
  }
  else if(addr == APU_NR11_ADDR){
    res = NR11;
  }
  else if(addr == APU_NR12_ADDR){
    res = NR12;
  }
  else if(addr == APU_NR13_ADDR){
    res = NR13;
  }
  else if(addr == APU_NR14_ADDR){
    res = NR14;
  }
  else if(addr == APU_NR21_ADDR){
    res = NR21;
  }
  else if(addr == APU_NR22_ADDR){
    res = NR22;
  }
  else if(addr == APU_NR23_ADDR){
    res = NR23;
  }
  else if(addr == APU_NR24_ADDR){
    res = NR24;
  }
  else if(addr == APU_NR30_ADDR){
    res = NR30;
  }
  else if(addr == APU_NR31_ADDR){
    res = NR31;
  }
  else if(addr == APU_NR32_ADDR){
    res = NR32;
  }
  else if(addr == APU_NR33_ADDR){
    res = NR33;
  }
  else if(addr == APU_NR34_ADDR){
    res = NR34;
  }
  else if(addr == APU_NR41_ADDR){
    res = NR41;
  }
  else if(addr == APU_NR42_ADDR){
    res = NR42;
  }
  else if(addr == APU_NR43_ADDR){
    res = NR43;
  }
  else if(addr == APU_NR44_ADDR){
    res = NR44;
  }
  else if(addr == APU_NR50_ADDR){
    res = NR50;
  }
  else if(addr == APU_NR51_ADDR){
    res = NR51;
  }
  else if(addr == APU_NR52_ADDR){
    res = NR52;
  }
  else{
    std::invalid_argument("Incorrect address for APU\n");
  }

  return res;
}

/** write::write
    Write a byte in APU at a given address

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void APU::write(uint16_t addr, uint8_t data){

  // NR52 is always writable
  // Only bit 7 can be modified, while the msbs are read-only
  if(addr == APU_NR52_ADDR){
    NR52 = (data & 0x80) | 0b01110000;
    return;
  }

  // In non-CGB consoles, the length registers
  // are always writable, also when the APU is off.
  if(addr == APU_NR11_ADDR){
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

  // If the APU is off, all the registers are read-only
  if((NR52 & 0x80) == 0) return;

  if(addr >= APU_WPRAM_INIT_ADDR and addr <= APU_WPRAM_END_ADDR){
    WPRAM[addr - APU_WPRAM_INIT_ADDR] = data;
  }
  else if(addr == APU_NR10_ADDR){
    NR10 = data | 0b10000000;
  }
  else if(addr == APU_NR12_ADDR){
    NR12 = data;
  }
  else if(addr == APU_NR13_ADDR){
    NR13 = data;
  }
  else if(addr == APU_NR14_ADDR){
    NR14 = data | 0b00111000;
  }
  else if(addr == APU_NR22_ADDR){
    NR22 = data;
    if((data & 0xF8) == 0){
      _channel_2_is_enabled = 0;
    }
  }
  else if(addr == APU_NR23_ADDR){
    NR23 = data;
  }
  else if(addr == APU_NR24_ADDR){
    NR24 = data| 0b00111000;
    if(NR24 & 0x80){
      _channel_2_is_enabled = 1;
      _channel_2_length_timer = 64 - (NR21 & 0x3F);
    }
  }
  else if(addr == APU_NR30_ADDR){
    NR30 = data | 0b01111111;
  }
  else if(addr == APU_NR32_ADDR){
    NR32 = data | 0b10011111;
  }
  else if(addr == APU_NR33_ADDR){
    NR33 = data;
  }
  else if(addr == APU_NR34_ADDR){
    NR34 = data | 0b00111000;
  }
  else if(addr == APU_NR42_ADDR){
    NR42 = data;
  }
  else if(addr == APU_NR43_ADDR){
    NR43 = data;
  }
  else if(addr == APU_NR44_ADDR){
    NR44 = data | 0b00111111;
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

  // Disable APU
  if((NR52 & 0x80) == 0) return;

  _current_DIV_value = bus->read(MMU_TIMER_INIT_ADDR);

  uint16_t channel_1_output = channel_1_handler();
  uint16_t channel_2_output = channel_2_handler();
  uint16_t channel_3_output = channel_3_handler();
  uint16_t channel_4_output = channel_4_handler();

  uint16_t apu_sample = (channel_1_output + channel_2_output +
                         channel_3_output + channel_4_output   ) / 4;

  _audio_buffer_downsampling_counter += APU_DSP_FREQUENCY;
  if(_audio_buffer_downsampling_counter > APU_BUS_FREQUENCY){
    _audio_buffer_downsampling_counter -= APU_BUS_FREQUENCY;

    _audio_buffer[_audio_buffer_counter++] = apu_sample;
    _audio_buffer[_audio_buffer_counter++] = apu_sample;

    if(_audio_buffer_counter == APU_AUDIO_BUFFER_SIZE){
      _audio_buffer_counter = 0;
      while ((SDL_GetQueuedAudioSize(audio_device)) > APU_AUDIO_BUFFER_SIZE * 2) SDL_Delay(1);
      SDL_QueueAudio(audio_device, _audio_buffer, APU_AUDIO_BUFFER_SIZE * 2);
    }
  }

  _previous_DIV_value = _current_DIV_value;
}

APU::~APU(){
  SDL_CloseAudioDevice(audio_device);
  SDL_Quit();
}

uint16_t APU::channel_1_handler(){
  return 0;
}

uint16_t APU::channel_2_handler(){
  uint8_t  duty_cycle             = (NR21 >> 6) & 0x03;
  uint16_t frequency              = NR23 | ((NR24 & 0x07) << 8);
  uint16_t frequency_timer_init   = (2048 - frequency) * 4;
  uint16_t amplitude              = 0;
  uint8_t  length_step            = 0;
  uint8_t  envelope_step          = 0;

  if(!_channel_2_is_enabled) return 0;

  _channel_2_timer--;

  if(_channel_2_timer == 0){
    _channel_2_timer = frequency_timer_init;
    _channel_2_wave_duty_position = (_channel_2_wave_duty_position + 1) % 8;
  }

  amplitude = _wave_duty_table[duty_cycle][_channel_2_wave_duty_position];


  if((_previous_DIV_value & (1 << 5)) and !(_current_DIV_value & (1 << 5))){
    _channel_2_frame_sequencer++;
    if((_channel_2_frame_sequencer % 2) == 0) length_step = 1;
    if((_channel_2_frame_sequencer % 8) == 0) envelope_step = 1;
  }

  if(length_step and (NR24 & 0x40)){
    _channel_2_length_timer--;
    if(_channel_2_length_timer == 0){
      _channel_2_is_enabled = 0;
    }
  }

  if(envelope_step){}

  return amplitude * 5000;

  // ????
  // pushing 3 seconds of samples to the audio buffer:
  // uint64_t x = 0;
  // for (int i = 0; i < audio_spec.freq * 3; i++) {
  //   x += 1;
  //   // SDL_QueueAudio expects a signed 16-bit value
  //   // note: "5000" here is just gain so that we will hear something
  //   int16_t sample = 0;
  //   if(x == 4000) x = 0;
  //   if(x < 2000) sample = 15000;
  //   else         sample = -15000;
  //   const int sample_size = sizeof(int16_t) * 1;
  //   SDL_QueueAudio(audio_device, &sample, sample_size);
  // }
  // ????
}

uint16_t APU::channel_3_handler(){
  return 0;
}
uint16_t APU::channel_4_handler(){
  return 0;
}

