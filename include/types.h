#ifndef AXKYRO_TYPES_H
#define AXKYRO_TYPES_H

#define MAX_USERNAME_LEN 50
#define MAX_DISTRICT_ID_LEN 20
#define MAX_FILTER_LEN 30
#define MAX_CATEGORY_LEN 20
#define MAX_DESCRIPTION_LEN 200
#define REPORT_SIZE 296
#include <stdint.h>
#include <time.h>

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
        uint8_t new_threshold;
        char filter_condition[MAX_FILTER_LEN];
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
} Report; // 296 bytes including padding

#endif
