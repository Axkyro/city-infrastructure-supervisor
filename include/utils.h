#ifndef AXKYRO_UTILS_H
#define AXKYRO_UTILS_H

#include "types.h"

// Collection of utils and helper functions for the project
void command_dump(Command *cmd);
void report_dump(Report *report);
void bits_to_symbol(int permissions);
#endif
