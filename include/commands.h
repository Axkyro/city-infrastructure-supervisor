#ifndef AXKYRO_COMMANDS_H
#define AXKYRO_COMMANDS_H

#include "types.h"
#include <sys/stat.h>

int directory_exists(const char *path, struct stat *info);
int file_exists(const char *path, struct stat *info);
int symlink_exists(const char *path, struct stat *info);

int create_file(const char *path, int permissions);
int create_directory(const char *path, int permissions);

int create_symlink(const char *path, const char *link);
int remove_symlink(const char *link);

int file_empty(struct stat *info);

int extract_permissions(struct stat *info);
int enforce_permissions(const char *path, int permissions);

int check_obj_sanity(struct stat *info, int permissions);
int sanitize();

int check_read_perm(int permissions, Role role);
int check_write_perm(int permissions, Role role);

void log_operation(Command *cmd, time_t timestamp);

int create_district(Command *cmd);

int input_report(Report *report, Command *cmd);


int add(Command *cmd);
int list(Command *cmd);
int view(Command *cmd);
int update_threshold(Command *cmd);


#endif
