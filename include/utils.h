#ifndef AXKYRO_UTILS_H
#define AXKYRO_UTILS_H

#include "types.h"

// Collection of utils and helper functions for the project
void command_dump(const Command *cmd);
void report_dump(const Report *report);

void bits_to_symbol(int permissions);
void build_path(const char *district_id, const char *file, char *path);


// AI GENERATED AS REQUIRED
int compare_int(int a, const char *op, int b);
int compare_str(const char *a, const char *op, const char *b);
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);
void free_command(Command *cmd);

#endif
