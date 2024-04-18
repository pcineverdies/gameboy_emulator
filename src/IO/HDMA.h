#ifndef __HDMA_H
#define __HDMA_H

#include "../bus/bus_obj.h"
#include "../memory/memory_map.h"
#include "../utils/gb_global_t.h"
#include <cstdint>
#include <string>
#include <stdexcept>

#define HDMA_GP_DMA 0
#define HDMA_HBLANK_DMA 1

class HDMA : public Bus_obj {

  uint8_t HDMA1;
  uint8_t HDMA2;
  uint8_t HDMA3;
  uint8_t HDMA4;
  uint8_t HDMA5;

  uint8_t  _is_transfering;
  uint8_t  _transfering_mode;
  uint16_t _transfer_length;
  uint16_t _current_transfer;
  uint16_t _cycles_to_wait;
  uint16_t _destination_address;
  uint16_t _source_address;
  uint8_t  _hblank_to_do;

public:

  HDMA(std::string, uint16_t);
  uint8_t read(uint16_t);
  void    write(uint16_t, uint8_t);
  void    step(Bus_obj*);

};


#endif // __HDMA_H
