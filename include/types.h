#ifndef AXKYRO_TYPES_H
#define AXKYRO_TYPES_H

#define MAX_USERNAME_LEN        50
#define MAX_DISTRICT_ID_LEN     20
#define MAX_FILTER_LEN          30
#define MAX_CATEGORY_LEN        20
#define MAX_DESCRIPTION_LEN     200
#define MAX_PATH_LEN            70
#define MAX_LOG_LEN             120

#define DISTRICT_PERMS          0750
#define LOGGED_DISTRICT_PERMS   0644
#define REPORTS_DAT_PERMS       0664
#define DISTRICT_CFG_PERMS      0640

#define MIN_ESCALATION_LEVEL 2
#define MAX_ESCALATION_DIGITS_LEN 2 // must always be >= 2

#define INITIAL_REPORT_ID 0

#include <stdint.h>
#include <time.h>

typedef enum {
    Manager,
    Inspector,
    Missing 
} Role;

typedef enum {
    Add,
    List,
    View,
    RemoveReport,
    UpdateThreshold,
    Filter,
    Invalid,
} Operation;

typedef struct {
    char user[MAX_USERNAME_LEN];
    Role role;
    Operation operation;
    
    char district_id[MAX_DISTRICT_ID_LEN];

    union { // used for additional possible fields  
        uint32_t report_id;
        uint8_t new_threshold;
        char **filter_conditions;
    } extra;
} Command;

typedef struct {
    time_t timestamp; 
    int32_t report_id;
    struct {
        float lat;
        float lng;
    } coordinates;
    char inspector[MAX_USERNAME_LEN];
    char issue_category[MAX_CATEGORY_LEN];
    char description[MAX_DESCRIPTION_LEN];
    uint8_t severity_level;
} __attribute__((packed)) Report;

#endif
