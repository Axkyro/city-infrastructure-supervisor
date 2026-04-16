#ifndef AXKYRO_TYPES_H
#define AXKYRO_TYPES_H

#define MAX_USERNAME_LEN 50
#define MAX_DISTRICT_ID_LEN 20


typedef enum {
    Manager,
    Inspector,
    Missing // no role
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
        int report_id;
        int new_threshold;
    } extra;
} Command;

#endif
