#ifndef AXKYRO_PARSER_H
#define AXKYRO_PARSER_H

#include "types.h"

// Functions for parsing arguments and input data

int parse_arguments(int argc, char **argv, Command *cmd);
Operation str_to_op(const char *str);
const char *op_to_str(Operation op);
const char *role_to_str(Role role);


#endif
