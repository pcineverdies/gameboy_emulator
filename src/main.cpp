#include "gameboy.h"

int main(int argc, char* argv[]){

  Gameboy* gb;

  if(argc < 2 or argc > 3)
    throw std::invalid_argument("Usage: ./gameboy ./path/to/ROM [fix_FPS]");

  gb = new Gameboy(argv[1], (argc == 3) ? 1 : 0);

  gb->run();

  delete gb;

}
