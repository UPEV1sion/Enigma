#pragma once

#include <stdint.h>

/*----------GENERAL----------*/
#define HELP        "--help"
#define HELP_SHORT  "-h"
#define GUI         "--gui"
#define GUI_SHORT   "-g"

void query_help(void);
void query_input(int32_t argc, char *argv[]);
