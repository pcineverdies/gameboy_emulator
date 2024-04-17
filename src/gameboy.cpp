#include "gameboy.h"

/*
 * This variable will be used by all the components to know
 * whether we are in CGB mode or in compatibility mode ("Non CGB")
 * */
struct gb_global_t gb_global;

/** Gameboy::Gameboy
    Constructor of the class. It creates and initialize all the objects which will
    be connected to the main bus; it sets the addresses with the respect to the mmu
    configuration; it uses the input file to initialize the cartridge

    @param name std::string Path to the rom file to use
    @param fixed_fps uint8_t Decides whether the FPS should be set to 60 or not

*/
Gameboy::Gameboy(std::string rom_file, uint8_t fixed_fps){

  // Create bus
  this->bus = new Bus("BUS", 0, 0xFFFF, BUS_FREQUENCY);

  // Initialize cartridge and set cgb mode. This is important for the initialization of
  // registers in the different components
  this->cart = new Cartridge(     "CART",       MMU_CART_INIT_ADDR,       MMU_CART_SIZE                             );
  this->cart->init_from_file(rom_file);

  // Create all the components to be attached to the bus
  this->wram = new WRAM(          "WRAM",       MMU_WRAM_INIT_ADDR,       MMU_WRAM_SIZE                             );
  this->cram = new CRAM(          "CRAM",       MMU_CRAM_INIT_ADDR,       MMU_CRAM_SIZE                             );
  this->oam = new Memory(         "OAM",        MMU_OAM_INIT_ADDR,        MMU_OAM_SIZE                              );
  this->joypad = new Joypad(      "JOYPAD",     MMU_JOYPAD_INIT_ADDR                                                );
  this->serial = new Serial(      "SERIAL",     MMU_SERIAL_INIT_ADDR                                                );
  this->timer = new Timer(        "TIMER",      MMU_TIMER_INIT_ADDR                                                 );
  this->ppu = new PPU(            "PPU",        MMU_PPU_INIT_ADDR                                                   );
  this->apu = new APU(            "APU",        MMU_APU_INIT_ADDR                                                   );
  this->brom_en = new Register(   "BROM_EN",    MMU_BROM_EN_INIT_ADDR,    MMU_BROM_EN_SIZE                          );
  this->hram = new Memory(        "HRAM",       MMU_HRAM_INIT_ADDR,       MMU_HRAM_SIZE                             );
  this->ie_reg = new Register(    "IE_REG",     MMU_IE_REG_INIT_ADDR,     MMU_IE_REG_SIZE,    MMU_IE_REG_INIT_VAL   );
  this->if_reg = new Register(    "IF_REG",     MMU_IF_REG_INIT_ADDR,     MMU_IF_REG_SIZE,    MMU_IF_REG_INIT_VAL   );
  this->svbk_reg = new Register(  "SVBK_REG",   MMU_SVBK_REG_INIT_ADDR,   MMU_SVBK_REG_SIZE,  MMU_SVBK_REG_INIT_VAL );
  this->vbk_reg = new Register(   "VBK_REG",    MMU_VBK_REG_INIT_ADDR,    MMU_VBK_REG_SIZE,   MMU_VBK_REG_INIT_VAL  );

  this->cpu = new Cpu("CPU", CPU_FREQUENCY);

  // Set frequency of the active components
  this->timer->set_frequency(BUS_FREQUENCY);
  this->serial->set_frequency(SERIAL_FREQUENCY);
  this->ppu->set_frequency(BUS_FREQUENCY);
  this->joypad->set_frequency(JOYPAD_FREQUENCY);
  this->apu->set_frequency(BUS_FREQUENCY);

  // Add components to the bus
  this->bus->add_to_bus(this->cart);
  this->bus->add_to_bus(this->wram);
  this->bus->add_to_bus(this->cram);
  this->bus->add_to_bus(this->oam);
  this->bus->add_to_bus(this->ppu);
  if(fixed_fps)
    this->bus->add_to_bus(this->apu);
  this->bus->add_to_bus(this->timer);
  this->bus->add_to_bus(this->joypad);
  this->bus->add_to_bus(this->serial);
  this->bus->add_to_bus(this->if_reg);
  this->bus->add_to_bus(this->brom_en);
  this->bus->add_to_bus(this->hram);
  this->bus->add_to_bus(this->ie_reg);
  this->bus->add_to_bus(this->svbk_reg);
  this->bus->add_to_bus(this->vbk_reg);
  this->bus->add_to_bus(this->cpu);

  // Add reference to the bus for specific components which
  // require out-of-step reading/writing
  this->cart->_bus_to_read = bus;
  this->wram->_bus_to_read = bus;

  // Add reference to the cartridge
  this->ppu->cart = this->cart;

  // Add reference to the cram
  this->ppu->cram = this->cram;

  gb_global.volume_amplification = 10;
  gb_global.exit_request = 0;
}

/** Gameboy::run
    Runs the gameboy by stepping the bus

*/
void Gameboy::run(){
  while(1){
    this->bus->step(bus);
    if(gb_global.exit_request) break;
  }
}

/** Gameboy::~Gameboy
    Deallocate all the objects of the Gameboy

*/
Gameboy::~Gameboy(){
  delete this->bus;
  delete this->cart;
  delete this->wram;
  delete this->oam;
  delete this->joypad;
  delete this->serial;
  delete this->timer;
  delete this->if_reg;
  delete this->ppu;
  delete this->brom_en;
  delete this->hram;
  delete this->ie_reg;
  delete this->cpu;
  delete this->svbk_reg;
  delete this->vbk_reg;
  delete this->cram;
}
