#include "gameboy.h"

/** Gameboy::Gameboy
    Constructor of the class. It creates and initialize all the objects which will
    be connected to the main bus; it sets the addresses with the respect to the mmu
    configuration; it uses the input file to initialize the cartridge

    @param name std::string Path to the rom file to use
    @param fixed_fps uint8_t Decides whether the FPS should be set to 60 or not

*/
Gameboy::Gameboy(std::string rom_file, uint8_t fixed_fps){

  // Create bus
  this->bus = new Bus("BUS", 0, 0xFFFF, BUS_FREQUENCY, fixed_fps);

  // Create all the components to be attached to the bus
  this->cart = new Cartridge(   "CART",    MMU_CART_INIT_ADDR,   MMU_CART_SIZE        );
  this->wram = new Memory(      "WRAM",    MMU_WRAM_INIT_ADDR,   MMU_WRAM_SIZE        );
  this->oam = new Memory(       "OAM",     MMU_OAM_INIT_ADDR,    MMU_OAM_SIZE         );
  this->joypad = new Joypad(    "JOYPAD",  MMU_JOYPAD_INIT_ADDR                       );
  this->serial = new Serial(    "SERIAL",  MMU_SERIAL_INIT_ADDR                       );
  this->timer = new Timer(      "TIMER",   MMU_TIMER_INIT_ADDR                        );
  this->if_reg = new Register(  "IF_REG",  MMU_IF_REG_INIT_ADDR,  MMU_IF_REG_INIT_VAL );
  this->ppu = new PPU(          "PPU",     MMU_PPU_INIT_ADDR                          );
  this->brom_en = new Register( "BROM_EN", MMU_BROM_EN_INIT_ADDR                      );
  this->hram = new Memory(      "HRAM",    MMU_HRAM_INIT_ADDR,    MMU_HRAM_SIZE       );
  this->ie_ref = new Register(  "IE_REG",  MMU_IE_REG_INIT_ADDR,  MMU_IE_REG_INIT_VAL );

  this->cpu = new Cpu("CPU", CPU_FREQUENCY);

  // Initialize cartridge
  this->cart->init_from_file(rom_file);

  // Set frequency of the active components
  this->timer->set_frequency(BUS_FREQUENCY);
  this->serial->set_frequency(SERIAL_FREQUENCY);
  this->ppu->set_frequency(BUS_FREQUENCY);
  this->joypad->set_frequency(JOYPAD_FREQUENCY);

  // Add components to the bus
  this->bus->add_to_bus(this->cart);
  this->bus->add_to_bus(this->wram);
  this->bus->add_to_bus(this->oam);
  this->bus->add_to_bus(this->ppu);
  this->bus->add_to_bus(this->timer);
  this->bus->add_to_bus(this->joypad);
  this->bus->add_to_bus(this->serial);
  this->bus->add_to_bus(this->if_reg);
  this->bus->add_to_bus(this->brom_en);
  this->bus->add_to_bus(this->hram);
  this->bus->add_to_bus(this->ie_ref);
  this->bus->add_to_bus(this->cpu);
}

/** Gameboy::run
    Runs the gameboy by stepping the bus

*/
void Gameboy::run(){
  while(1)
    this->bus->step(bus);
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
  delete this->ie_ref;
  delete this->cpu;
}
