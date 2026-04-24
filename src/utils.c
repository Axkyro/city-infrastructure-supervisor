#include "utils.h"
#include "parser.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void command_dump(const Command *cmd) {
    printf("+-----------------------------------------------------------------+"
           "\n");
    printf("| User: [%-20s] | Role    : [%-20s] |\n", cmd->user,
           role_to_str(cmd->role));
    printf("| Op  : [%-20s] | District: [%-20s] |\n", op_to_str(cmd->operation),
           cmd->district_id);
    printf("+-----------------------------------------------------------------+"
           "\n");
}
void report_dump(const Report *report) {
    const char *border =
        "+-------------------------------------------------------------------+";
    printf("%s\n", border);
    printf("| ID    : [%-20d] | Category: [%-20s] |\n", report->report_id,
           report->issue_category);
    printf("| User  : [%-20s] | Severity: [%-20hhu] |\n", report->inspector,
           report->severity_level);
    printf("| Y(Lat): [%-20.2f] | X(Lng)  : [%-20.2f] |\n",
           report->coordinates.lat, report->coordinates.lng);
    printf("| Time  : [%-20ld] |         : [%-20s] |\n", report->timestamp, "");
    printf("%s\n", border);

    printf("| Description:                                                     "
           " |\n");
    printf("| %-65.65s |\n", report->description);
    printf("%s\n", border);
}
void bits_to_symbol(int permissions) {
    const char *symbols = "xwrxwrxwr";
    for (int i = 8; i >= 0; i--) {
        printf("%c", 0x1 & (permissions >> i) ? symbols[i] : '-');
    }
}

void build_path(const char *district, const char *file, char *path) {
    snprintf(path, MAX_PATH_LEN, "%s/%s", district, file);
}

// THE FUNCTIONS BELOW ARE AI GENERATED AS PER REQUIREMENTS

int compare_int(int a, const char *op, int b) {
    if (strcmp(op, "==") == 0) return a == b;
    if (strcmp(op, "!=") == 0) return a != b;
    if (strcmp(op, "<")  == 0) return a < b;
    if (strcmp(op, "<=") == 0) return a <= b;
    if (strcmp(op, ">")  == 0) return a > b;
    if (strcmp(op, ">=") == 0) return a >= b;
    return 0;
}

int compare_str(const char *a, const char *op, const char *b) {
    if (strcmp(op, "==") == 0) return strcmp(a, b) == 0;
    if (strcmp(op, "!=") == 0) return strcmp(a, b) != 0;
    return 0; // invalid for strings
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    if (!r || !field || !op || !value)
        return 0;

    // severity (uint8_t)
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);
        return compare_int(r->severity_level, op, val);
    }

    // category (string)
    if (strcmp(field, "category") == 0) {
        return compare_str(r->issue_category, op, value);
    }

    // inspector (string)
    if (strcmp(field, "inspector") == 0) {
        return compare_str(r->inspector, op, value);
    }

    // timestamp (time_t)
    if (strcmp(field, "timestamp") == 0) {
        long val = atol(value);
        return compare_int((long)r->timestamp, op, val);
    }

    return 0; // unknown field
}

int parse_condition(const char *input, char *field, char *op, char *value) {
    if (!input || !field || !op || !value)
        return -1;

    // Find first ':' (field separator)
    const char *first_colon = strchr(input, ':');
    if (!first_colon)
        return -1;

    // Find second ':' (operator separator)
    const char *second_colon = strchr(first_colon + 1, ':');
    if (!second_colon)
        return -1;

    // Extract field
    size_t field_len = first_colon - input;
    strncpy(field, input, field_len);
    field[field_len] = '\0';

    // Extract operator
    size_t op_len = second_colon - (first_colon + 1);
    strncpy(op, first_colon + 1, op_len);
    op[op_len] = '\0';

    // Extract value
    strcpy(value, second_colon + 1);

    return 0;
}

void free_command(Command *cmd) {
    if (!cmd)
        return;

    // Only free if it's a Filter operation and the array was allocated
    if (cmd->operation == Filter && cmd->extra.filter_conditions != NULL) {
        // Free every duplicated string
        for (int k = 0; cmd->extra.filter_conditions[k] != NULL; k++) {
            free(cmd->extra.filter_conditions[k]);
        }
        // Free the array itself
        free(cmd->extra.filter_conditions);
        cmd->extra.filter_conditions = NULL; // Prevent double-free
    }
}
