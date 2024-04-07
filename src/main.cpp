#include "gameboy.h"
#include "utils/cli_parser.h"

int main(int argc, char* argv[]){

  gb_cli_args_t args = parse_gb_args(argc, argv);
  Gameboy gb(args.rom_file_name, args.fixed_fps);

  gb.run();
}
