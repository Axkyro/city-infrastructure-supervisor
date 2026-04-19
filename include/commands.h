#ifndef AXKYRO_COMMANDS_H
#define AXKYRO_COMMANDS_H

#include "types.h"

int create_district(Command *cmd);
int exists_district(const char *path);
int extract_permissions(const char *path);
int add(Command *cmd);
int list(Command *cmd);
int view(Command *cmd);
void log_op(Command *cmd, time_t timestamp);
int is_empty_file(const char *path);
#endif
