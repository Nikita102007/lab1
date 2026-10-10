#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_COUNT_ARGS_ERR = -1,
    STATUS_INVALID_ARGS = -2,
    STATUS_WRONG_FLAG = -3,
    STATUS_OVERFLOW = -4,
    STATUS_ERR_OPEN_FILE = -5,
    STATUS_ERR_CLOSE_FILE = -6,
    STATUS_ERR_MALLOC = -7,
    STATUS_ERR_PUTC = -8,
} Status;

static Status filename_validator(const char *str)
{
    if (!str) return STATUS_INVALID_ARGS;

    size_t len = strlen(str);

    if (len <= 4) return STATUS_INVALID_ARGS;

    if (strcmp(".txt", &str[len - 4]) != 0) return STATUS_INVALID_ARGS;

    return STATUS_OK;
}

static Status argv_validator(const int argc, char *argv[], char *isout, char *flag)
{
    if (argc < 3) return STATUS_COUNT_ARGS_ERR;

    if (strlen(argv[1]) == 2) {
        if (argv[1][0] == '-' || argv[1][0] == '/')
            *flag = argv[1][1];
        else
            return STATUS_WRONG_FLAG;
    }
    else if (strlen(argv[1]) == 3) {
        if ((argv[1][0] == '-' || argv[1][0] == '/') &&
            argv[1][1] == 'n') {
            (*isout)++;
            *flag = argv[1][2];
        }
        else {
            return STATUS_WRONG_FLAG;
        }
    }
    else {
        return STATUS_WRONG_FLAG;
    }

    if (*isout == 1 && argc != 4) return STATUS_COUNT_ARGS_ERR;
    if (*isout == 0 && argc != 3) return STATUS_COUNT_ARGS_ERR;

    if (*flag != 'd' && *flag != 'i' &&
        *flag != 's' && *flag != 'a')
        return STATUS_WRONG_FLAG;

    Status status_input = filename_validator(argv[2]);
    Status status_output = STATUS_OK;

    if (*isout == 1)
        status_output = filename_validator(argv[3]);

    if (status_input != STATUS_OK || status_output != STATUS_OK)
        return STATUS_INVALID_ARGS;

    struct stat s1, s2;

    if (stat(argv[2], &s1) != 0)
        return STATUS_INVALID_ARGS;

    if (*isout == 1) {
        if (stat(argv[3], &s2) == 0) {
            if (s1.st_dev == s2.st_dev &&
                s1.st_ino == s2.st_ino)
                return STATUS_INVALID_ARGS;
        }
    }
    else {
        size_t len = strlen(argv[2]);
        char *result = malloc(strlen("out_") + len + 1);

        if (!result) return STATUS_ERR_MALLOC;

        strcpy(result, "out_");
        strcat(result, argv[2]);

        if (stat(result, &s2) == 0) {
            if (s1.st_dev == s2.st_dev &&
                s1.st_ino == s2.st_ino) {
                free(result);
                return STATUS_INVALID_ARGS;
            }
        }

        free(result);
    }

    return STATUS_OK;
}

static Status delete_numbers(FILE *input, FILE *output)
{
    if (!input || !output) return STATUS_INVALID_ARGS;

    int c;

    while ((c = fgetc(input)) != EOF) {
        if (!(c >= '0' && c <= '9')) {
            if (fputc(c, output) == EOF) return STATUS_ERR_PUTC;
        }
    }

    return STATUS_OK;
}

static Status count_alpha_in_rows(FILE *input, FILE *output)
{
    if (!input || !output) return STATUS_INVALID_ARGS;

    int c;
    long long row = 1;
    long long count_alpha = 0;
    int has_chars = 0;

    while ((c = fgetc(input)) != EOF) {
        if (c == '\n') {
            if (fprintf(output, "Row %lld: %lld\n", row, count_alpha) < 0) return STATUS_ERR_PUTC;
            if (row == LLONG_MAX) return STATUS_OVERFLOW;
            row++;
            count_alpha = 0;
            has_chars = 0;
            continue;
        }

        has_chars = 1;

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            if (count_alpha == LLONG_MAX) return STATUS_OVERFLOW;
            count_alpha++;
        }
    }

    if (has_chars)
        if (fprintf(output, "Row %lld: %lld\n", row, count_alpha) < 0) return STATUS_ERR_PUTC;

    return STATUS_OK;
}

static Status count_other_symbols_in_rows(FILE *input, FILE *output)
{
    if (!input || !output) return STATUS_INVALID_ARGS;

    int c;
    long long row = 1;
    long long count_other = 0;
    int has_chars = 0;

    while ((c = fgetc(input)) != EOF) {
        if (c == '\n') {
            if (fprintf(output, "Row %lld: %lld\n", row, count_other) < 0) return STATUS_ERR_PUTC;
            if (row == LLONG_MAX) return STATUS_OVERFLOW;
            row++;
            count_other = 0;
            has_chars = 0;
            continue;
        }

        has_chars = 1;

        if ((c & 0xC0) == 0x80) continue;

        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == ' ')) {
            if (count_other == LLONG_MAX) return STATUS_OVERFLOW;
            count_other++;
        }
    }

    if (has_chars)
        if (fprintf(output, "Row %lld: %lld\n", row, count_other) < 0) return STATUS_ERR_PUTC;

    return STATUS_OK;
}

static Status hex_symbols(FILE *input, FILE *output)
{
    if (!input || !output) return STATUS_INVALID_ARGS;

    int c;

    while ((c = fgetc(input)) != EOF) {
        if (c >= '0' && c <= '9') {
            if (fputc(c, output) == EOF) return STATUS_ERR_PUTC;
        }
        else {
            if (fprintf(output, "%02X", (unsigned)c) < 0) return STATUS_ERR_PUTC;
        }
    }

    return STATUS_OK;
}

int main(int argc, char *argv[])
{
    Status status;
    char isout = 0;
    char flag = 0;

    status = argv_validator(argc, argv, &isout, &flag);

    if (status != STATUS_OK) {
        printf("Status: %d\n", status);
        return status;
    }

    FILE *input = fopen(argv[2], "r");

    if (!input) {
        printf("Status: %d\n", STATUS_ERR_OPEN_FILE);
        return STATUS_ERR_OPEN_FILE;
    }

    FILE *output;

    if (isout == 1) output = fopen(argv[3], "w");
    else {
        char *result = malloc(strlen("out_") + strlen(argv[2]) + 1);

        if (!result) {
            if (fclose(input) != 0) {
                printf("Status: %d\n", STATUS_ERR_CLOSE_FILE);
                return STATUS_ERR_CLOSE_FILE;
            }
            printf("Status: %d\n", STATUS_ERR_MALLOC);
            return STATUS_ERR_MALLOC;
        }
        strcpy(result, "out_");
        strcat(result, argv[2]);
        output = fopen(result, "w");
        free(result);
    }

    if (!output) {
        if (fclose(input) != 0) {
            printf("Status: %d\n", STATUS_ERR_CLOSE_FILE);
            return STATUS_ERR_CLOSE_FILE;
        }
        printf("Status: %d\n", STATUS_ERR_OPEN_FILE);
        return STATUS_ERR_OPEN_FILE;
    }

    switch (flag) {
        case 'd':
            status = delete_numbers(input, output);
            break;
        case 'i':
            status = count_alpha_in_rows(input, output);
            break;
        case 's':
            status = count_other_symbols_in_rows(input, output);
            break;
        case 'a':
            status = hex_symbols(input, output);
            break;
        default:
            status = STATUS_WRONG_FLAG;
            break;
    }

    char err_flag = 0;

    if (fclose(input) != 0) err_flag = 1;
    if (fclose(output) != 0) err_flag = 1;

    if (err_flag == 1) {
        printf("Status: %d\n", STATUS_ERR_CLOSE_FILE);
        return STATUS_ERR_CLOSE_FILE;
    }

    if (status != STATUS_OK) {
        printf("Status: %d\n", status);
        return status;
    }

    printf("Status: %d\n", STATUS_OK);
    return STATUS_OK;
}
