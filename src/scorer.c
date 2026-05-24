#include "commands.h"
#include "types.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GROWTH_FACTOR 2
#define INIT_SCORES_SIZE 3
#define MALLOC_FAIL 1

typedef struct {
    char inspector[MAX_USERNAME_LEN];
    int workload;
} Score;

Score *resize_scores(Score *scores, size_t *size);

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Received invalid arguments!\n");
        return 1;
    }

    char path[MAX_PATH_LEN];
    struct stat info;

    build_dir_path(argv[1], path);

    if (!directory_exists(path, &info)) {
        printf("District not found: %s!\n", argv[1]);
        return 1;
    }

    if (check_district_sanity(argv[1]) != 0) {
        printf("Corrupted district found: %s!\n", path);
        return 1;
    }
    size_t scores_size = 0;
    size_t scores_capacity;
    Score *scores = resize_scores(NULL, &scores_capacity);

    if (!scores) {
        printf("Failed initial scores malloc!\n");
        return MALLOC_FAIL;
    }
    build_file_path(argv[1], "reports.dat", path);
    file_exists(path, &info);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Failed to open: %s!\n", path);
        return 1;
    }
    size_t reports_count = info.st_size / sizeof(Report);

    Report report;
    int found = 0;
    for (size_t i = 0; i < reports_count; i++) {
        found = 0;
        if (read(fd, &report, sizeof(Report)) == -1) {
            close(fd);
            printf("Read Error!\n");
            return 1;
        }
        for (size_t k = 0; k < scores_size; k++) {
            if (strcmp(scores[k].inspector, report.inspector) == 0) {
                scores[k].workload += (int)report.severity_level;
                found = 1;
                break;
            }
        }
        if (found)
            continue;
        if (scores_size >= scores_capacity)
            scores = resize_scores(scores, &scores_capacity);

        strcpy(scores[scores_size].inspector, report.inspector);
        scores[scores_size].workload = report.severity_level;
        scores_size++;
    }

    for (size_t i = 0; i < scores_size; i++) {
        printf("Inspector: [%s] | Workload: [%d]\n", scores[i].inspector,
               scores[i].workload);
    }
    close(fd);
    free(scores);
    return 0;
}

Score *resize_scores(Score *scores, size_t *size) {

    if (!scores) {
        *size = INIT_SCORES_SIZE;
        return malloc(sizeof(Score) * INIT_SCORES_SIZE);
    }

    if (*size < *size * GROWTH_FACTOR) {
        *size *= GROWTH_FACTOR;
    } else {
        exit(MALLOC_FAIL); // overflow
    }

    void *temp = realloc(scores, sizeof(Score) * *size);

    if (!temp) {
        free(scores);
        printf("Failed realloc with size: %lu\n", *size);
        exit(MALLOC_FAIL);
    }

    return temp;
}
