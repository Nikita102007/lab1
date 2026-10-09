#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#define MIN_SIZE    8

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGS = -2,
    STATUS_OVERFLOW = -4,
    STATUS_ERR_MALLOC = -8,
    STATUS_ERR_REALLOC = -9,
    STATUS_WRONG_BASE = -10
} Status;

static Status symbol_base_validator(const int c, const int base)
{
    if (2 > base || base > 36) return STATUS_WRONG_BASE;

    if (c >= 'A' && c <= 'Z') {
        if (c - 'A' + 10 >= base) return STATUS_INVALID_ARGS;
    }
    else if (c >= '0' && c <= '9') {
        if (c - '0' >= base) return STATUS_INVALID_ARGS;
    }
    else {
        return STATUS_INVALID_ARGS;
    }

    return STATUS_OK;
}

static Status get_base(int *c, int *base)
{
    if (!c || !base) return STATUS_INVALID_ARGS;

    *base = 0;

    while (*c != '\n' && *c != EOF) {
        if (*c >= '0' && *c <= '9') {
            int digit = *c - '0';

            if (*base > (36 - digit) / 10) return STATUS_WRONG_BASE;

            *base = *base * 10 + digit;

            *c = getchar();
            continue;
        }
        return STATUS_INVALID_ARGS;
    }

    if (*c == EOF) return STATUS_INVALID_ARGS;

    if (*base < 2) return STATUS_WRONG_BASE;

    return STATUS_OK;
}

static Status get_word(int *c, char **num, size_t *count_nums, int *is_negative, int *is_stop)
{
    if (!c || !num || !count_nums || !is_negative || !is_stop) return STATUS_INVALID_ARGS;

    *is_negative = 0;
    *is_stop = 0;
    char is_leading_zero = 0;

    if (*c == 'S') *is_stop = 1;

    if (*c == '-') {
        *is_negative = 1;
        *c = getchar();
        if (*c == EOF) return STATUS_INVALID_ARGS;
    }

    if (*c == '0') is_leading_zero = 1;

    size_t size = MIN_SIZE;
    *num = malloc(MIN_SIZE * sizeof(char));

    if (*num == NULL) return STATUS_ERR_MALLOC;

    *count_nums = 0;

    while (*c != '\n' && *c != EOF) {
        if (*count_nums == SIZE_MAX) return STATUS_OVERFLOW;

        if (*count_nums > size - 1) {
            if (size > SIZE_MAX / 2) return STATUS_OVERFLOW;

            char *tmp = realloc(*num, 2 * size * sizeof(char));

            if (!tmp) return STATUS_ERR_REALLOC;

            *num = tmp;
            size *= 2;
        }

        if (is_leading_zero == 1 && *c != '0') is_leading_zero = 0;

        if (is_leading_zero == 0){
            (*num)[(*count_nums)++] = *c;
        }
        *c = getchar();
    }

    if (is_leading_zero == 1) (*num)[(*count_nums)++] = '0';
    
    if (*count_nums == 0) return STATUS_INVALID_ARGS;

    if (*count_nums == size) {
        if (size == SIZE_MAX) return STATUS_OVERFLOW;

            char *tmp = realloc(*num, (1 + size) * sizeof(char));

            if (!tmp) return STATUS_ERR_REALLOC;

            *num = tmp;
        }

    (*num)[*count_nums] = '\0';

    if (strcmp("Stop", *num) != 0) *is_stop = 0;

    return STATUS_OK;
}

static Status num_to_dec(const char *num, const int base, long long *dec_num, const int is_negative)
{
    if (!num || !dec_num) return STATUS_INVALID_ARGS;

    if (base < 2 || base > 36) return STATUS_WRONG_BASE;

    if (is_negative != 0 && is_negative != 1) return STATUS_INVALID_ARGS;

    unsigned long long value = 0;
    unsigned long long limit;

    if (is_negative) limit = (unsigned long long)LLONG_MAX + 1ULL;
    else limit = (unsigned long long)LLONG_MAX;

    for (size_t i = 0; num[i] != '\0'; i++) {
        int digit;

        Status status = symbol_base_validator(num[i], base);

        if (status != STATUS_OK) return status;

        if (num[i] >= 'A' && num[i] <= 'Z')
            digit = num[i] - 'A' + 10;
        else
            digit = num[i] - '0';

        if (value > (limit - (unsigned long long)digit) / (unsigned)base)
            return STATUS_OVERFLOW;

        value = value * (unsigned)base + (unsigned long long)digit;
    }

    if (is_negative) {
        if (value == (unsigned long long)LLONG_MAX + 1ULL)
            *dec_num = LLONG_MIN;
        else
            *dec_num = -(long long)value;
    } else {
        *dec_num = (long long)value;
    }

    return STATUS_OK;
}

static Status dec_to_base(long long num, char *result, const int base, int *size) 
{
    if (!result || !size) return STATUS_INVALID_ARGS;

    if (!(base >= 2 && base <= 36)) return STATUS_WRONG_BASE;

    char is_neg = 0;
    unsigned long long dec;
    *size = 0;

    if (num == LLONG_MIN)
        dec = (unsigned long long)LLONG_MAX + 1ULL;
    else
        dec = (unsigned long long)llabs(num);

    if (num < 0) is_neg = 1;

    if (dec == 0) {
        result[(*size)++] = '0';
        return STATUS_OK;
    }

    while (dec > 0) {
        if (dec % base > 9) {
            result[(*size)++] = 'A' + (dec % base - 10);
        }
        else {
            result[(*size)++] = '0' + (dec % base);
        }

        dec /= base;
    }

    if (is_neg == 1) result[(*size)++] = '-';

    return STATUS_OK;
}

int main(void)
{
    Status status = STATUS_OK;
    int base;
    char *num = NULL;
    long long dec_num;
    int is_negative;
    long long max_num = 0;
    long long sum = 0;
    size_t count_nums;
    int size;
    int has_num = 0;
    char arr[66];
    int is_stop;
    int c = getchar();

    status = get_base(&c, &base);

    if (status != STATUS_OK) {
        printf("Status: %d\n", status);
        return status;
    }

    if (c == '\n') c = getchar();

    while (c != EOF) {

        status = get_word(&c, &num, &count_nums, &is_negative, &is_stop);

        if (status != STATUS_OK) {
            free(num);
            num = NULL;
            break;
        }

        if (is_stop) {
            free(num);
            num = NULL;
            break;
        }

        status = num_to_dec(num, base, &dec_num, is_negative);

        if (status != STATUS_OK) {
            free(num);
            num = NULL;
            break;
        }

        if (dec_num == LLONG_MIN) max_num = dec_num;
        else {
            if (max_num != LLONG_MIN)
                if (llabs(max_num) < llabs(dec_num)) max_num = dec_num;
        }

        if (dec_num > 0) {
            if (sum > LLONG_MAX - dec_num) {
                free(num);
                num =NULL;
                status = STATUS_OVERFLOW;
                break;
            }
            sum += dec_num;
        }
        else {
            if (sum < LLONG_MIN - dec_num) {
                free(num);
                num = NULL;
                status = STATUS_OVERFLOW;
                break;
            }
            sum += dec_num;
        }
        has_num = 1;
        free(num);
        num = NULL;
        c = getchar();
    }

    if (status != STATUS_OK) goto EXIT;

    if (has_num == 0) {
        status = STATUS_INVALID_ARGS;
        goto EXIT;
    }

    printf("MAX_NUM: ");

    status = dec_to_base(max_num, arr, base, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf(" ");

    status = dec_to_base(max_num, arr, 9, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf(" ");

    status = dec_to_base(max_num, arr, 18, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf(" ");

    status = dec_to_base(max_num, arr, 27, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf(" ");

    status = dec_to_base(max_num, arr, 36, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf("\n");
    printf("SUM ");

    status = dec_to_base(sum, arr, base, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf(" ");

    status = dec_to_base(sum, arr, 9, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf(" ");

    status = dec_to_base(sum, arr, 18, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf(" ");

    status = dec_to_base(sum, arr, 27, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf(" ");

    status = dec_to_base(sum, arr, 36, &size);
    if (status != STATUS_OK) goto EXIT;
    for (int i = size - 1; i >= 0; i--) printf("%c", arr[i]);

    printf("\n");
    goto EXIT;

    EXIT:
        printf("Status: %d\n", status);
        return status;
}