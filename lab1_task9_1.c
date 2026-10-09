#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define SIZE    100

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGS = -2,
    STATUS_OVERFLOW = -4,
} Status;

static Status parse_number(const char *str, long long *num)
{
    if (!str || !num) return STATUS_INVALID_ARGS;

    *num = 0;
    char sign = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '-' && i == 0) {
            sign = 1;
            continue;
        }

        if (str[i] < '0' || str[i] > '9')
            return STATUS_INVALID_ARGS;

        int digit = str[i] - '0';

        if (*num > (LLONG_MAX - digit) / 10)
            return STATUS_OVERFLOW;

        *num = *num * 10 + digit;  
    }

    if (sign == 1) *num *= -1;

    return STATUS_OK;
}

static Status get_and_swap_max_min(long long *arr, long long *min, long long *max)
{
    if (!arr || !min || !max) return STATUS_INVALID_ARGS;

    int min_pose = 0;
    int max_pose = 0;

    for (int i = 0; i < SIZE; i++) {
        if (*max <= arr[i]) {
            max_pose = i;
            *max = arr[i];
        }

        if (*min >= arr[i]) {
            min_pose = i;
            *min = arr[i];
        }
    }

    long long t = arr[min_pose];
    arr[min_pose] = arr[max_pose];
    arr[max_pose] = t;

    return STATUS_OK;
}

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL));
    Status status = STATUS_OK;
    long long a;
    long long b;
    long long max;
    long long min;
    long long arr[SIZE];

    if (argc != 3) {
        status = STATUS_INVALID_ARGS;
        goto EXIT;
    }

    status = parse_number(argv[1], &a);

    if (status != STATUS_OK) goto EXIT;

    status = parse_number(argv[2], &b);

    if (status != STATUS_OK) goto EXIT;

    if (a >= b) {
        status = STATUS_INVALID_ARGS;
        goto EXIT;
    }

    for (int i = 0; i < SIZE; i++) {
        if (a < 0) {
            if (b > LLONG_MAX + a) {
                status = STATUS_OVERFLOW;
                goto EXIT;
            }
        }
        arr[i] = a + rand() % (b - a + 1);
    }

    for (int i = 0; i < SIZE; i++) {
        if (i + 1 == SIZE) printf("%lld\n", arr[i]);
        else printf("%lld ", arr[i]);
    }

    min = arr[0];
    max = arr[0];

    status = get_and_swap_max_min(arr, &min, &max);

    printf("MAX: %lld\n", max);
    printf("MIN: %lld\n", min);

    for (int i = 0; i < SIZE; i++) {
        if (i + 1 == SIZE) printf("%lld\n", arr[i]);
        else printf("%lld ", arr[i]);
    }

    goto EXIT;

    EXIT:
        printf("Status: %d\n", status);
        return status;
}