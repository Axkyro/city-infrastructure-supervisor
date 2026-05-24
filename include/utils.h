#ifndef AXKYRO_UTILS_H
#define AXKYRO_UTILS_H

#include "types.h"

// utils to check a command or report
void command_dump(const Command *cmd);
void report_dump(const Report *report);

// printing the symbolic equivalent of permissions to screen - rwxrwx--
void bits_to_symbol(int permissions);

// building file paths
void build_file_path(const char *district_id, const char *file, char *path);
void build_symlink_path(const char *district_id, char *path);
void build_dir_path(const char *district_id, char *path);

// ai generated functions as required by the spec
int compare_int(int a, const char *op, int b);
int compare_str(const char *a, const char *op, const char *b);

int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op,
                    const char *value);

void free_command(Command *cmd);

#endif
