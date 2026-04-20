#include "utils.h"
#include "parser.h"
#include "types.h"
#include <stdio.h>

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
