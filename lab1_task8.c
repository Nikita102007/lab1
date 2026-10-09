#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#define MIN_SIZE    8

typedef enum {
    STATUS_OK = 0,
    STATUS_COUNT_ARGS_ERR = -1,
    STATUS_INVALID_ARGS = -2,
    STATUS_OVERFLOW = -4,
    STATUS_ERR_OPEN_FILE = -5,
    STATUS_ERR_CLOSE_FILE = -6,
    STATUS_ERR_PUTC = -7,
    STATUS_ERR_MALLOC = -8,
    STATUS_ERR_REALLOC = -9,
    STATUS_WRONG_BASE = -10,
} Status;

static Status filename_validator(const char *str)
{
    if (!str) return STATUS_INVALID_ARGS;

    size_t len = strlen(str);

    if (len <= 4) return STATUS_INVALID_ARGS;

    if (strcmp(".txt", &str[len - 4]) != 0) return STATUS_INVALID_ARGS;

    return STATUS_OK;
}

static Status min_base_func(FILE *input, char **num, int *c, int *min_base, size_t *count_nums, int *is_negative)
{
    if (!input || !c || !min_base || !count_nums || !is_negative || !num) return STATUS_INVALID_ARGS;

    *num = malloc(MIN_SIZE * sizeof(char));

    if (!*num) return STATUS_ERR_MALLOC;

    char is_leading_zero = 1;
    char is_first = 1;
    size_t size_arr = MIN_SIZE;
    *min_base = 2;
    *count_nums = 0;
    *is_negative = 0;


    while (*c != ' ' && *c != '\t' && *c != '\n' && *c != EOF) {
        int symbol = (int)toupper((unsigned char)*c);
        if (symbol == '-' && is_first == 1) {
            is_first = 0;
            *is_negative = 1;
            *c = fgetc(input);
            continue;
        }

        if (!(symbol >= 'A' && symbol <= 'Z') && !(symbol >= '0' && symbol <= '9'))
            return STATUS_INVALID_ARGS;

        if (symbol == '0' && is_leading_zero == 1) {
            is_first = 0;
            *c = fgetc(input);
            if (*c == EOF || *c == ' ' || *c == '\t' || *c == '\n') {
                *min_base = 2;
                (*num)[*(count_nums)++] = '0';
            }
            continue;
        }

        if (*count_nums == SIZE_MAX) return STATUS_OVERFLOW;

        if (*count_nums + 1 > size_arr) {
            if (size_arr > SIZE_MAX / 2) return STATUS_OVERFLOW;

            char *tmp = realloc(*num, 2 * size_arr * sizeof(char));

            if (!tmp) return STATUS_ERR_REALLOC;

            *num = tmp;
            size_arr *= 2;
        }

        if (symbol > *min_base) *min_base = symbol;

        (*num)[(*count_nums)++] = symbol;

        *c = fgetc(input);
        is_leading_zero = 0;
        is_first = 0;
    }

    if (*min_base >= 'A' && *min_base <= 'Z') *min_base -= 54;
    else {
        if (*min_base - 47 > 2) *min_base -= 47;
        else *min_base = 2; 
    }

    if (!(*min_base >= 2 && *min_base <= 36)) return STATUS_WRONG_BASE;

    if (*count_nums == 0) return STATUS_INVALID_ARGS;

    return STATUS_OK;
}

static Status num_to_dec(const char *num, const size_t count_nums, const int *base, long long *dec_num, const int is_negative)
{
    if (!num || !base || !dec_num) return STATUS_INVALID_ARGS;

    if (*base < 2 || *base > 36) return STATUS_WRONG_BASE;

    if (is_negative != 0 && is_negative != 1) return STATUS_INVALID_ARGS;

    long long pow_base;
    long long digit;
    *dec_num = 0;

    for (size_t i = 0; i < count_nums; i++) {
        if (num[i] >= 'A' && num[i] <= 'Z')
            digit = num[i] - 55;
        else if (num[i] >= '0' && num[i] <= '9')
            digit = num[i] - 48;
        else
            return STATUS_INVALID_ARGS;
        pow_base = 1;
        for (size_t j = count_nums - 1 - i; j > 0; j--) {
            if (pow_base > LLONG_MAX / *base) return STATUS_OVERFLOW;
            pow_base *= *base;
        }
        if (digit > LLONG_MAX / pow_base) return STATUS_OVERFLOW;

        digit *= pow_base;

        if (*dec_num > LLONG_MAX - digit) return STATUS_OVERFLOW;

        *dec_num += digit;
    }

    if (is_negative == 1) *dec_num *= -1;

    return STATUS_OK;
}

int main(int argc, char *argv[])
{
    Status status = STATUS_OK;
    FILE *input = NULL;
    FILE *output = NULL;
    int c;
    char *num = NULL;
    int min_base;
    size_t count_nums;
    long long dec_num;
    int is_negative = 0;

    if (argc != 3) {
        printf("Status: %d\n", STATUS_COUNT_ARGS_ERR);
        return STATUS_COUNT_ARGS_ERR;
    }

    if (filename_validator(argv[1]) != STATUS_OK || filename_validator(argv[2]) != STATUS_OK) {
        printf("Status: %d\n", STATUS_INVALID_ARGS);
        return STATUS_INVALID_ARGS;
    }

    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");

    if (!input) {
        if (output) if (fclose(output) != 0) return STATUS_ERR_CLOSE_FILE;
        return STATUS_ERR_OPEN_FILE;
    }

    if (!output) {
        if (input) if (fclose(input) != 0) return STATUS_ERR_CLOSE_FILE;
        return STATUS_ERR_OPEN_FILE;
    }

    c = fgetc(input);

    while (c != EOF) {
        while (c == ' ' || c == '\t' || c == '\n') c = fgetc(input);
        if (c == EOF) {
            status = STATUS_OK;
            break;
        }

        status = min_base_func(input, &num, &c, &min_base, &count_nums, &is_negative);

        if (status != STATUS_OK) {
            free(num);
            num = NULL;
            break;
        }

        status = num_to_dec(num, count_nums, &min_base, &dec_num, is_negative);

        if (status != STATUS_OK) {
            free(num);
            num = NULL;
            break;
        }

        if (count_nums > INT_MAX) {
            status = STATUS_OVERFLOW;
            free(num);
            num = NULL;
            break;
        }

        if (dec_num < 0)
            if (fputc('-', output) == EOF) status = STATUS_ERR_PUTC;

        if (fprintf(output, "%.*s: %d %lld\n", (int)count_nums, num, min_base, dec_num) < 0) 
            status = STATUS_ERR_PUTC;
        free(num);
        num = NULL;

        if (status != STATUS_OK) break;
    }

    if (fclose(input) != 0 && status == STATUS_OK) status = STATUS_ERR_CLOSE_FILE;
    if (fclose(output) != 0 && status == STATUS_OK) status = STATUS_ERR_CLOSE_FILE;

    if (status != STATUS_OK) {
        printf("Status: %d\n", status);
        return status;
    }

    printf("Status: %d\n", status);
    return status;
}