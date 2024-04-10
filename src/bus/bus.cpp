#include "bus.h"
#include <unistd.h>
#include <SDL.h>

/** Bus::Bus
    Constructor of the class. It just calls the parent constructor.
    By having a bus which is Bus_obj, we can have a hierarchy of buses.

    @param name std::string Name of the object to create
    @param init_addr uint16_t Initial address of the object once connected to the bus
    @param size uint16_t Size of the addressable space of the object
    @param frequency uint32_t Working frequency of the clock connected to the bus

*/
Bus::Bus(std::string name, uint16_t init_addr, uint16_t size, uint32_t frequency) :
  Bus_obj(name, init_addr, size){
  this->set_frequency(frequency);
  current_cycles_counting = 0;
}

/** Bus::add_to_bus
    Add the pointer of an object to the list of handled objects. It
    checks that the requested addresses were not used by a previous object

    There are 4 overlap situations (this is the usual probelm of ranges of dates):

    1.  Prev:            |------------------|
        New:        |---------------|

    2.  Prev:        |---------------|
        New:            |------------------|

    3.  Prev:                |--------|
        New:            |------------------|

    4.  Prev:            |------------------|
        New:                |--------|

    Each of these sitations have to lead to an error.

    @param new_object Bus_obj* object to add to the bus

*/
void Bus::add_to_bus(Bus_obj* new_object){

  // Always add objects whose size is 0. These are non addressable objects
  if(new_object->get_size() != 0){

    for(auto& obj : bus_objects){
      if(obj->get_size() == 0) continue;
      uint16_t N_i = new_object->get_init_addr();
      uint16_t N_l = new_object->get_last_addr();
      uint16_t P_i = obj->get_init_addr();
      uint16_t P_l = obj->get_last_addr();

      if((N_i <= P_i and N_l >= P_i) or (N_i >= P_i and N_i <= P_l) or (N_i >= P_i and N_l <= P_l) or (N_i <= P_i and P_l <= N_l))
        throw std::invalid_argument("Overlap in address space between " + obj->name + " and " + new_object->name);
    }
  }

  if(new_object->get_frequency() != 0 and
      ( this->get_frequency() % new_object->get_frequency() != 0 or new_object->get_frequency() > this->get_frequency())
  ) throw std::invalid_argument("Objects connected to bus must have frequency divisible for the bus frequency");

  bus_objects.push_back(new_object);
  frequency_cache.push_back(new_object->get_frequency());
  init_addr_cache.push_back(new_object->get_init_addr());
  size_cache.push_back(new_object->get_size());
}


/** Bus::read
    Read by from memory at a given address.
    Try all the objects connected to the bus.

    @param addr uint16_t address to read
    @return uint8_t read byte

*/
uint8_t Bus::read(uint16_t addr){

  uint16_t size;
  uint16_t init_addr;

  for(uint32_t i = 0; i < bus_objects.size(); i++){

    size = size_cache[i];
    init_addr = init_addr_cache[i];

    // Cannot read on non-addressable objects
    if(size == 0) continue;

    if(addr >= init_addr && addr < init_addr + size)
      return bus_objects[i]->read(addr - init_addr);
  }

  return 0xff;
}

/** Bus::write
    Write a byte in memory at a given address.
    Try with all the object connected to the bus.

    @param addr uint16_t address to use
    @param data uint8_t  byte to write

*/
void Bus::write(uint16_t addr, uint8_t data){

  uint16_t size;
  uint16_t init_addr;

  for(uint32_t i = 0; i < bus_objects.size(); i++){

    size = size_cache[i];
    init_addr = init_addr_cache[i];

    // Cannot write on non-addressable objects
    if(size == 0) continue;

    if(addr >= init_addr && addr < init_addr + size){
      bus_objects[i]->write(addr - init_addr, data);
      return;
    }
  }

}

/** Bus::step

    @param bus Bus_obj* pointer to the bus to use to perform reading from the elements side

*/
void Bus::step(Bus_obj* bus){

  uint32_t bus_frequency = this->frequency;
  uint32_t obj_frequency;

  // Reset current tick
  if(current_cycles_counting == 0)
    initial_tick_frame = SDL_GetTicks();

  for(uint32_t i = 0; i < bus_objects.size(); i++){

    obj_frequency = frequency_cache[i];

    // Cannot step asynchronous objects, such as memories
    if(obj_frequency == 0) continue;

    // Try the next 4 T-cycles and possibly perform all the steps.
    // This is a way to lose a bit of timing accuracy, while gaining
    // performances in the emulator
    for(uint32_t j = 0; j < BUS_STEP_SIZE; j++){
      if((current_cc + j) % (bus_frequency / obj_frequency) == 0)
        bus_objects[i]->step(bus);
    }

  }

  current_cc += BUS_STEP_SIZE;

}
