#include "gameboy.h"
#include "utils/cli_parser.h"

int main(int argc, char* argv[]){

  Gameboy* gb;
  gb_cli_args_t args = parse_gb_args(argc, argv);

  gb = new Gameboy(args.rom_file_name, args.fixed_fps);

  gb->run();

  delete gb;

}
