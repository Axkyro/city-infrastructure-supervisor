#include "commands.h"
#include "types.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#define DIR_FLAGS 0750

int exists_district(const char *str) {
   return 0; 
}

int add_district(Command *cmd) {
    if(strlen(cmd->district_id) == 0) {
        fprintf(stderr, "Empty district received!\n"); 
        return -1; 
    }
    int check = mkdir(cmd->district_id, DIR_FLAGS);
    if (check == -1) {
        fprintf(stderr, "District [%s] directory already exists!\n", cmd->district_id);
        return -1;
    }
    
    return 0;
}

