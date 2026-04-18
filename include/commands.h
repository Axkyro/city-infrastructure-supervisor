#ifndef AXKYRO_COMMANDS_H
#define AXKYRO_COMMANDS_H

#include "types.h"

int add_district(Command *cmd);
int exists_district(const char *str);


int extract_permissions(const char *str);

#endif
