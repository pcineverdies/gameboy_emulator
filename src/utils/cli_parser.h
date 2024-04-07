#ifndef __CLI_PARSER_H
#define __CLI_PARSER_H

#include <string>
#include <iostream>

struct gb_cli_args_t {
  std::string rom_file_name;
  bool        fixed_fps;
};

gb_cli_args_t parse_gb_args(int, char*[]);

#endif // __CLI_PARSER_H
