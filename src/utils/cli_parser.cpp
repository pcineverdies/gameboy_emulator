#include "cli_parser.h"


/** parse_gb_args
    Given the argv of the program, parses the input and provide the corrent arguments
    to the main function, in order to properly create the gameboy class. The available
    arguments are:

     --rom path   -> path to the rom to run
    [--fixed_fps] -> Fixes the fps to 60 while running the gameboy
    [--help]      -> Prints the help message

    @param argc int Number of arguments in the cli command
    @param argv char*[] Array with the cli strings
    @return gb_cli_args_t structure contianing the expected elements

*/
gb_cli_args_t parse_gb_args(int argc, char* argv[]){

  gb_cli_args_t args;
  args.fixed_fps = 0;
  args.rom_file_name = "";
  const std::string helper_string = "Usage: ./gameboy --rom path/to/rom [--fixed_fps]";

  // Skip ./gameboy command
  for(int i = 1; i < argc; i++){

    // Take current string
    std::string current_argv = argv[i];

    // If help message, show the message and exit
    if(current_argv == "--help"){
      std::cout << helper_string << std::endl;
      exit(1);
    }

    // if "--rom", consider next token if available
    if(current_argv == "--rom"){
      if(++i == argc) break;
      args.rom_file_name = argv[i];
      continue;
    }

    // if "--fixed_fps", set the value to true
    if(current_argv == "--fixed_fps"){
      args.fixed_fps = true;
    }
  }

  if(args.rom_file_name == ""){
    std::cerr << helper_string << std::endl;
    exit(1);
  }

  return args;
}
