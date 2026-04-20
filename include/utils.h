#ifndef AXKYRO_UTILS_H
#define AXKYRO_UTILS_H

#include "types.h"

// Collection of utils and helper functions for the project
void command_dump(const Command *cmd);
void report_dump(const Report *report);

void bits_to_symbol(int permissions);
void build_path(const char *district_id, const char *file, char *path);
#endif
