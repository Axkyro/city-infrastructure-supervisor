#ifndef AXKYRO_COMMANDS_H
#define AXKYRO_COMMANDS_H

#include "types.h"
#include <sys/stat.h>

// check if a file exists and extract info if it does
int directory_exists(const char *path, struct stat *info);
int file_exists(const char *path, struct stat *info);
int symlink_exists(const char *path, struct stat *info);

// create a specific file/dir with permissions
int create_file(const char *path, int permissions);
int create_directory(const char *path, int permissions);

void remove_directory_recursive_force(const char *path);

int create_symlink(const char *path, const char *link);
int remove_symlink(const char *link);

int file_empty(struct stat *info);

int extract_permissions(struct stat *info);
int enforce_permissions(const char *path, int permissions);

int check_obj_sanity(struct stat *info, int permissions);
int check_district_sanity(const char *path);

int check_read_perm(int permissions, Role role);
int check_write_perm(int permissions, Role role);

void log_operation(Command *cmd, time_t timestamp);
void log_operation_message(Command *cmd, time_t timestamp, const char *message);

int create_district(Command *cmd);
int input_report(Report *report, Command *cmd);

int add(Command *cmd);
int list(Command *cmd);
int view(Command *cmd);
int remove_report(Command *cmd);
int update_threshold(Command *cmd);
int filter(Command *cmd);
int remove_district(Command *cmd);

#endif
