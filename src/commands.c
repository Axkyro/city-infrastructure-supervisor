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
#define LOGGED_DISTRICT_FLAGS 0644
#define REPORTS_DAT_FLAGS 0664
#define DISTRICT_CFG_FLAGS 0640
#define MAX_PATH_LENGTH MAX_DISTRICT_ID_LEN + 20


int exists_district(const char *str) {
   return 0; 
}

int extract_permissions(const char *path) {
    struct stat info;
    if (stat(path, &info) != 0 ) {
        fprintf(stderr, "Couldn't read [%s] permissions!\n", path);
    }

    return 0x1FF & info.st_mode;
}


int add_district(Command *cmd) {
    
    if(strlen(cmd->district_id) == 0) {
        fprintf(stderr, "Empty district received!\n"); 
        return -1; 
    }

    int check = mkdir(cmd->district_id, 0); // we could also do mkdir(..., DIR_FLAGS);
    
    chmod(cmd->district_id, DIR_FLAGS);  
    if (check == -1) {
        fprintf(stderr, "District [%s] directory already exists!\n", cmd->district_id);
        return -1;
    }

    int fd;
    char path[MAX_PATH_LENGTH] = "";

    strcat(path, cmd->district_id);
    strcat(path, "/");

    size_t cat_point = strlen(path);
    
    // creating reports.dat
    strcpy(path + cat_point, "reports.dat");
    fd = open(path, O_CREAT | O_RDONLY, 0);
    chmod(path, REPORTS_DAT_FLAGS);  
    close(fd);
    
    // creating district.cfg
    strcpy(path + cat_point, "district.cfg");
    fd = open(path, O_CREAT | O_RDONLY, 0);
    chmod(path, DISTRICT_CFG_FLAGS);  
    close(fd);

    // creating logged_district
    strcpy(path + cat_point, "logged_district");
    fd = open(path, O_CREAT | O_RDONLY, 0);
    chmod(path, LOGGED_DISTRICT_FLAGS);  
    close(fd);
    
    


    return 0;
}

