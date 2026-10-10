#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_COUNT_ARGS_ERR = -1,
    STATUS_INVALID_ARGS = -2,
    STATUS_WRONG_FLAG = -3,
    STATUS_OVERFLOW = -4,
    STATUS_ERR_OPEN_FILE = -5,
    STATUS_ERR_CLOSE_FILE = -6,
    STATUS_ERR_PUTC = -7,
} Status;

static Status filename_validator(const char *str)
{
    if (!str) return STATUS_INVALID_ARGS;

    size_t len = strlen(str);

    if (len <= 4) return STATUS_INVALID_ARGS;

    if (strcmp(".txt", &str[len - 4]) != 0) return STATUS_INVALID_ARGS;

    return STATUS_OK;
}

static Status write_word(FILE *file, FILE *output, int *is_first, int *c)
{
    if (!file || !output || !is_first || !c) return STATUS_INVALID_ARGS;

    while (*c == ' ' || *c == '\t' || *c == '\n')
        *c = fgetc(file);

    if (*c == EOF) return STATUS_OK;

    if (*is_first != 1) {
        if (fputc(' ', output) == EOF) return STATUS_ERR_PUTC;
    } else {
        *is_first = 0;
    }

    while (*c != ' ' && *c != '\t' && *c != '\n' && *c != EOF) {
        if (fputc(*c, output) == EOF) return STATUS_ERR_PUTC;
        *c = fgetc(file);
    }

    return STATUS_OK;
}

static Status r_func(FILE *file1, FILE *file2, FILE *output)
{
    if (!file1 || !file2 || !output) return STATUS_INVALID_ARGS;

    char current_file = 1;
    int is_first = 1;
    int c1 = fgetc(file1);
    int c2 = fgetc(file2);
    Status status = STATUS_OK;

    while (c1 != EOF && c2 != EOF) {
        if (current_file == 1) {
            status = write_word(file1, output, &is_first, &c1);
            if (status != STATUS_OK) return status;
            current_file = 2;
            continue;
        }

        status = write_word(file2, output, &is_first, &c2);
        if (status != STATUS_OK) return status;
        current_file = 1;
    }

    while (c1 != EOF) {
        status = write_word(file1, output, &is_first, &c1);
        if (status != STATUS_OK) return status;
    }

    while (c2 != EOF) {
        status = write_word(file2, output, &is_first, &c2);
        if (status != STATUS_OK) return status;
    }

    return STATUS_OK;
}

static Status a_func(FILE *file, FILE *output)
{
    if (!file || !output) return STATUS_INVALID_ARGS;

    int symbol;
    int flag = 0;
    int is_first = 1;
    int c = fgetc(file);

    while (c != EOF) {
        while (c == ' ' || c == '\t' || c == '\n')
            c = fgetc(file);

        if (c == EOF) break;

        if (is_first != 1) {
            if (fputc(' ', output) == EOF) return STATUS_ERR_PUTC;
        } else {
            is_first = 0;
        }

        if (flag + 1 > 10) flag = 1;
        else flag++;

        if (flag % 10 == 0) {
            char arr[4];
            int count;

            while (c != ' ' && c != '\t' && c != '\n' && c != EOF) {
                count = 0;
                symbol = tolower((unsigned char)c);

                while (symbol > 0) {
                    if (count == 4) return STATUS_OVERFLOW;
                    arr[count++] = '0' + (symbol % 4);
                    symbol /= 4;
                }

                for (int i = count - 1; i >= 0; i--)
                    if (fputc(arr[i], output) == EOF) return STATUS_ERR_PUTC;

                c = fgetc(file);
            }
        } else if (flag % 2 == 0) {
            while (c != ' ' && c != '\t' && c != '\n' && c != EOF) {
                if (fputc(tolower((unsigned char)c), output) == EOF)
                    return STATUS_ERR_PUTC;
                c = fgetc(file);
            }
        } else if (flag % 5 == 0) {
            char arr[3];
            int count;

            while (c != ' ' && c != '\t' && c != '\n' && c != EOF) {
                count = 0;
                symbol = c;

                while (symbol > 0) {
                    if (count == 3) return STATUS_OVERFLOW;
                    arr[count++] = '0' + (symbol % 8);
                    symbol /= 8;
                }

                for (int i = count - 1; i >= 0; i--)
                    if (fputc(arr[i], output) == EOF) return STATUS_ERR_PUTC;

                c = fgetc(file);
            }
        } else {
            while (c != ' ' && c != '\t' && c != '\n' && c != EOF) {
                if (fputc(c, output) == EOF) return STATUS_ERR_PUTC;
                c = fgetc(file);
            }
        }
    }

    return STATUS_OK;
}

int main(int argc, char *argv[])
{
    Status status = STATUS_OK;
    char flag;
    FILE *file1 = NULL;
    FILE *file2 = NULL;
    FILE *output = NULL;

    if (argc != 4 && argc != 5) {
        printf("Status: %d\n", STATUS_COUNT_ARGS_ERR);
        return STATUS_COUNT_ARGS_ERR;
    }

    if (strlen(argv[1]) != 2) {
        printf("Status: %d\n", STATUS_WRONG_FLAG);
        return STATUS_WRONG_FLAG;
    }

    if (!(argv[1][0] == '-' || argv[1][0] == '/') ||
        !(argv[1][1] == 'r' || argv[1][1] == 'a')) {
        printf("Status: %d\n", STATUS_WRONG_FLAG);
        return STATUS_WRONG_FLAG;
    }

    flag = argv[1][1];

    if (flag == 'r') {
        if (argc != 5) {
            printf("Status: %d\n", STATUS_COUNT_ARGS_ERR);
            return STATUS_COUNT_ARGS_ERR;
        }

        if (filename_validator(argv[2]) != STATUS_OK ||
            filename_validator(argv[3]) != STATUS_OK ||
            filename_validator(argv[4]) != STATUS_OK) {
            printf("Status: %d\n", STATUS_INVALID_ARGS);
            return STATUS_INVALID_ARGS;
        }
    }

    if (flag == 'a') {
        if (argc != 4) {
            printf("Status: %d\n", STATUS_COUNT_ARGS_ERR);
            return STATUS_COUNT_ARGS_ERR;
        }

        if (filename_validator(argv[2]) != STATUS_OK ||
            filename_validator(argv[3]) != STATUS_OK) {
            printf("Status: %d\n", STATUS_INVALID_ARGS);
            return STATUS_INVALID_ARGS;
        }
    }

    switch (flag) {
        case 'r': {
            struct stat s1, s2, s3;

            if (stat(argv[2], &s1) != 0 ||
                stat(argv[3], &s2) != 0) {
                status = STATUS_INVALID_ARGS;
                break;
            }

            if (s1.st_dev == s2.st_dev &&
                s1.st_ino == s2.st_ino) {
                status = STATUS_INVALID_ARGS;
                break;
            }

            if (stat(argv[4], &s3) == 0) {
                if ((s1.st_dev == s3.st_dev &&
                     s1.st_ino == s3.st_ino) ||
                    (s2.st_dev == s3.st_dev &&
                     s2.st_ino == s3.st_ino)) {
                    status = STATUS_INVALID_ARGS;
                    break;
                }
            }

            file1 = fopen(argv[2], "r");
            if (!file1) {
                status = STATUS_ERR_OPEN_FILE;
                break;
            }

            file2 = fopen(argv[3], "r");
            if (!file2) {
                status = STATUS_ERR_OPEN_FILE;
                break;
            }

            output = fopen(argv[4], "w");
            if (!output) {
                status = STATUS_ERR_OPEN_FILE;
                break;
            }

            status = r_func(file1, file2, output);
            break;
        }

        case 'a': {
            struct stat s1, s2;

            if (stat(argv[2], &s1) != 0) {
                status = STATUS_INVALID_ARGS;
                break;
            }

            if (stat(argv[3], &s2) == 0) {
                if (s1.st_dev == s2.st_dev &&
                    s1.st_ino == s2.st_ino) {
                    status = STATUS_INVALID_ARGS;
                    break;
                }
            }

            file1 = fopen(argv[2], "r");
            if (!file1) {
                status = STATUS_ERR_OPEN_FILE;
                break;
            }

            output = fopen(argv[3], "w");
            if (!output) {
                status = STATUS_ERR_OPEN_FILE;
                break;
            }

            status = a_func(file1, output);
            break;
        }
    }

    if (file1 && fclose(file1) != 0 && status == STATUS_OK)
        status = STATUS_ERR_CLOSE_FILE;

    if (file2 && fclose(file2) != 0 && status == STATUS_OK)
        status = STATUS_ERR_CLOSE_FILE;

    if (output && fclose(output) != 0 && status == STATUS_OK)
        status = STATUS_ERR_CLOSE_FILE;

    if (status != STATUS_OK) {
        printf("Status: %d\n", status);
        return status;
    }

    printf("Status: %d\n", STATUS_OK);
    return STATUS_OK;
}
