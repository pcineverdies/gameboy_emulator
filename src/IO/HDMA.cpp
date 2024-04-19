#include "HDMA.h"
#include <cstdio>

extern struct gb_global_t gb_global;

/** HDMA::read
    Read by from the hdma at a given address

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t HDMA::read(uint16_t addr){

  uint8_t res = 0;

  // If gbc_mode is not active, the peripheral should do nothing
  if(gb_global.gbc_mode == 0) return 0xff;

  if     (addr == 0) res = HDMA1;
  else if(addr == 1) res = HDMA2;
  else if(addr == 2) res = HDMA3;
  else if(addr == 3) res = HDMA4;
  else if(addr == 4) res = HDMA5;
  else{
    throw std::invalid_argument("Not valid address for HRAM reading");
  }

  return res;
}

/** HDMA::write
    Write of the registers

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void HDMA::write(uint16_t addr, uint8_t data){

  // If gbc_mode is not active, the peripheral should do nothing
  if(gb_global.gbc_mode == 0) return;

  if (addr == 0){
    HDMA1 = data;
  }
  else if(addr == 1){
    // 4 MSBs are ignored
    HDMA2 = data & 0xF0;
  }
  else if(addr == 2){
    // The destination is always VRAM, thus the MSBs of the 16-bit address
    // are always `100`. This is automatically handled in hardware, and games
    // don't always handle this requirement.
    HDMA3 = (data | 0x80) & 0x9F;
  }
  else if(addr == 3){
    // 4 MSBs are ignored
    HDMA4 = data & 0xF0;
  }
  else if(addr == 4){

    // If a value is written while not transfering, then a transfer starts
    if(_is_transfering == 0){
      _is_transfering = 1;

      /*
       * HBLANK mode: The HDMA transfers 16 bytes for each HBLANK stage. The CPU can run
       * outside of these periods.
       *
       * Genral Purpose DMA mode: The HDMA transfers all the bytes at once (2 T-cycles per byte),
       * and the CPU is stucked during this period
       * */
      _transfering_mode = (data & HDMA_HDMA5_CONTROL_BIT_MASK) ? HDMA_HBLANK_DMA : HDMA_GP_DMA;

      // 7 MSBs of the data are about the number of chunks of 16 bytes to be moved, minus one
      _transfer_length = data & 0x7F;

      // Stores which is the current transferred chunk
      _current_transfer = 0;

      // Compute source address
      _source_address = (HDMA1 << 8) | HDMA2;

      // Compute destination address
      _destination_address = (HDMA3 << 8) | HDMA4;

      // Different timings are required for the different modes
      _cycles_to_wait = (data & HDMA_HDMA5_CONTROL_BIT_MASK) ? 0 : 32;

      // This variables tells the HDMA to start a chunk transfer as soon as the
      // the next HBLANK phase starts
      _hblank_to_do = 1;

      // HDMA5 must be always updated with the number of chunks still to be transferred (minus 1),
      // and the MSB tells whether the CPU can run or not.
      HDMA5 = _transfer_length | ((data & HDMA_HDMA5_CONTROL_BIT_MASK) ? HDMA_HDMA5_CONTROL_BIT_MASK : 0x00);
    }
    else{
      // In case the register is written during a transfer with MSB being 0, the transfer is stopped
      if(!(data & HDMA_HDMA5_CONTROL_BIT_MASK)){
        _is_transfering = 0;
        HDMA5 |= HDMA_HDMA5_CONTROL_BIT_MASK;
      }
    }

  }
  else{
    throw std::invalid_argument("Not valid address for HRAM reading");
  }
}

/** HDMA::HDMA
    HDMA constructor, setting init_addr of the hdma in the memory space
    The size of the object is fixed to 5. All the variables are set to zero,
    and they will be initialized when a transfer starts

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus

*/
HDMA::HDMA(std::string name, uint16_t init_addr) : Bus_obj(name, init_addr, 5){
  HDMA1 = 0;
  HDMA2 = 0;
  HDMA3 = 0;
  HDMA4 = 0;
  HDMA5 = 0x80; // Transfer not active

  _is_transfering = 0;
  _transfering_mode = 0;
  _transfer_length = 0;
  _current_transfer = 0;
  _cycles_to_wait = 0;
  _destination_address = 0;
  _source_address = 0;
  _hblank_to_do = 0;
}

/** HDMA::step
    Perform the step of the timer at each T-cycle

    @param bus Bus_obj* pointer to a bus to use for reading

*/
void HDMA::step(Bus_obj* bus){
  uint8_t current_STAT = 0;
  uint8_t to_move;

  // In non-gbc mode no hdma is allowed. Also,
  // no step is required if transfering is being done
  if(gb_global.gbc_mode == 0 or !_is_transfering) return;

  /*
   * Case of general purpose DMA: cpu is stuck until the transfer
   * is completed. Each chungs of 16 bytes takes 32 cycles. To simplify
   * timing, 16 bytes are instantly transferred, and then 32 cycles are waited
   * */
  if(_transfering_mode == HDMA_GP_DMA){

    // Wait for some cycles
    if(_cycles_to_wait-- != 0) return;

    // In 32 cycles, HDMA is able to move 0x10 bytes
    for(int i = 0; i < 0x10; i++){
      to_move = bus->read(_source_address + _current_transfer * 0x10 + i);
      bus->write(_destination_address + _current_transfer * 0x10 + i, to_move);
    }

    // Move to next transfer and proper cycles to wait
    _current_transfer++, _cycles_to_wait = 32;

    // In case all the bytes are transferred, then the HDMA stops. This is
    // indicated by HDMA5 being 0xFF.
    if(_current_transfer > _transfer_length){
      _is_transfering = 0;
      HDMA5 = 0xFF;
    }
  }

  /*
   * In HBLANK mode, for each HBLANK phase of the PPU, 0x10 bytes are transferred.
   * During these transfers, the CPU cannot work.
   * */
  else{

    // You need to know if you are in HBLANK
    current_STAT  = bus->read(PPU_STAT);

    // If we entered mode 2 of PPU, then we can perform a transfer
    // once that we find a new HBLANK mode
    if((current_STAT & 0x03) == 0x02) _hblank_to_do = 1;

    // If the current mode is not HBLANK, there is nothing to do.
    if((current_STAT & 0x03) != 0x00) return;

    // After a transfer is done, 32 cycles must be waited. Once
    // this time is passed, the MSB of HDMA5 is set.
    if(_cycles_to_wait){
      if(--_cycles_to_wait == 0){
        _hblank_to_do = 0;
        HDMA5 = 0x80 | HDMA5;

        // If the number of transfers to be done is reched, the transfer is finished
        if(_current_transfer > _transfer_length){
          _is_transfering = 0;
          HDMA5 = 0xFF;
        }
      }
      return;
    }

    // In case _hblank_to_do is set, we still have to perform a transfer of a chunk
    // of 16 bytes of the current HBLANK phase. We do it, and we set the `_cycles_to_wait`
    // to get the prover value.
    if(_hblank_to_do){

      // In 32 cycles, HDMA is able to move 0x10 bytes
      for(int i = 0; i < 16; i++){
        to_move = bus->read(_source_address + _current_transfer * 0x10 + i);
        bus->write(_destination_address + _current_transfer * 0x10 + i, to_move);
      }

      _current_transfer++;
      _cycles_to_wait = 32;

      // Set new value of HDMA5. This expression has MSB cleared, which is fine as
      // the CPU needs to wait until a transfer is done.
      HDMA5 = (_transfer_length - _current_transfer);

    }
  }
}

