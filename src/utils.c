#include <stdio.h>
#include "utils.h"
#include "types.h"
#include "parser.h"

void command_dump(Command *cmd) {
    printf("User: [%s] | Role: [%s]\n", cmd->user, role_to_str(cmd->role));
    printf("Op: [%s] | District: [%s]\n", op_to_str(cmd->operation), cmd->district_id);

}

void report_dump(Report *report) {
    printf("Report ID: %d\n", report->report_id);
    printf("User: [%s] | Issue: [%s]\n", report->inspector, report->issue_category);
    printf("Coordinates: Y: %.2f, X: %.2f | Severity: [%hhu]\n", report->coordinates.lat, report->coordinates.lng, report->severity_level);
    printf("Time: %ld\n", report->timestamp); 
    printf("Description: %s\n", report->description);
}


void bits_to_symbol(int permissions) {
    char *symbols="xwrxwrxwr";
    for(int i = 8; i >= 0; i--) {
        printf("%c", 0x1 & (permissions >> i) ? symbols[i] : '-');
    }
}
