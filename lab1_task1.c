#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define MAX_SIZE    100

typedef enum {
    STATUS_OK = 0,
    STATUS_COUNT_ARGS_ERR = -1,
    STATUS_INVALID_ARGS = -2,
    STATUS_WRONG_FLAG = -3,
    STATUS_OVERFLOW = -4,
    STATUS_WRONG_NUMBER = -5,
    STATUS_ERR_DIV_BY_ZERO = -6,
    STATUS_NON_PC = -7,
} Status;

static Status parse_number(const char *str, long long *result)
{
    if (!str || !result || str[0] == '\0') return STATUS_INVALID_ARGS;

    *result = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') { 
            if (*result <= (LLONG_MAX - (str[i] - '0')) / 10) *result = *result * 10 + (str[i] - '0');
            
            else return STATUS_OVERFLOW;
        }
        else {
            return STATUS_WRONG_NUMBER;
        }
    }
    return STATUS_OK;
}

static Status find_multiples(int *arr, int *count, const long long *x)
{
    if (!arr || !count || !x) return STATUS_INVALID_ARGS;
    if (*x == 0) return STATUS_ERR_DIV_BY_ZERO;

    *count = 0;
    for (long long i = 1; i <= MAX_SIZE; i++) {
        if (i % *x == 0) {
            arr[*count] = i;
            (*count)++;
        }
    }

    return STATUS_OK;
}

static Status prime_or_composite(const long long *x, char *prime)
{
    if (!x || !prime) return STATUS_INVALID_ARGS;
    if (*x == 0 || *x == 1) return STATUS_NON_PC;

    *prime = 1;

    for (long long i = 2; i <= (long long)sqrt(*x); i++) {
        if (*x % i == 0) {
            *prime = 0;
            break;
        }
    }

    return STATUS_OK;
}

static Status hex_digits(char *arr, int *count, const long long *x)
{
    if (!arr || !count || !x) return STATUS_INVALID_ARGS;

    long long n = *x;
    *count = 0;

    while (n > 0) {
        char a = n % 16;
        if (a < 10) arr[(*count)++] = '0' + a;
        else arr[(*count)++] = 'A' + (a - 10);
        n /= 16;
    }

    return STATUS_OK;
}

static Status pow_table(long long *total, const long long *x)
{
    if (!total || !x || *x > 10 || *x < 1) return STATUS_INVALID_ARGS;

    int cur_pos = 0;
    int n = 0;
    while (cur_pos < 10 * *x) {
        n++;
        long long sup_n = n;
        for (int i = 0; i < *x; i++) {
            total[cur_pos++] = sup_n;
            sup_n *= n;
        }

    }

    return STATUS_OK;
}

static Status one_to_x_sum(long long *sum, const long long *x)
{
    if (!sum || !x || *x == 0) return STATUS_INVALID_ARGS;
    if (*x == LLONG_MAX) return STATUS_OVERFLOW;
    
    long long a, b;


    if (*x % 2 == 0) {
        a = *x / 2;
        b = *x + 1;
    }
    else {
        a = *x;
        b = (*x + 1) / 2;
    }

    if (a <= LLONG_MAX / b) *sum = a * b;
    else return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status factorial(long long *fact,  const long long *x)
{
    if (!fact || !x) return STATUS_INVALID_ARGS;

    *fact = 1;
    for (long long i = *x; i > 1; i--) {
        if (*fact <= LLONG_MAX / i) *fact *= i;
        else return STATUS_OVERFLOW;
    }

    return STATUS_OK;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Status: %d\n", STATUS_COUNT_ARGS_ERR);
        return STATUS_COUNT_ARGS_ERR;
    }

    if (strlen(argv[2]) != 2) {
        printf("Status: %d\n", STATUS_WRONG_FLAG);
        return STATUS_WRONG_FLAG;
    }

    if (argv[2][0] != '-' && argv[2][0] != '/') {
        printf("Status: %d\n", STATUS_WRONG_FLAG);
        return STATUS_WRONG_FLAG;
    }

    long long x;
    char flag = argv[2][1];

    Status status = parse_number(argv[1], &x);

    if (status != STATUS_OK) {
        printf("Status: %d\n", status);
        return status;
    }

    switch (flag) {
        case 'h':
            int arr[MAX_SIZE];
            int count;
            Status status_h = find_multiples(arr, &count, &x);

            if (status_h != STATUS_OK) {
                printf("Status: %d\n", status_h);
                return status_h;
            }

            for (int i = 0; i < count; i++) printf("%d ", arr[i]);

            if (count == 0) printf("Numbers is not found\n");
            break;
        case 'p':
            char prime;
            Status status_p = prime_or_composite(&x, &prime);

            if (status_p != STATUS_OK) {
                printf("Status: %d\n", status_p);
                return status_p;
            }

            if (prime == 1) printf("%lld is prime\n", x);
            if (prime == 0) printf("%lld is composite\n", x);
            break;
        case 's':
            char digs[sizeof(long long) * CHAR_BIT];
            int size;
            Status status_s = hex_digits(digs, &size, &x);

            if (status_s != STATUS_OK) {
                printf("Status: %d\n", status_s);
                return status_s;
            }

            for (int i = size - 1; i >= 0; i--) printf("%c ", digs[i]);

            if (size == 0) printf("0\n");
            break;
        case 'e':
            long long table[MAX_SIZE];
            Status status_e = pow_table(table, &x);

            if (status_e != STATUS_OK) {
                printf("Status: %d\n", status_e);
                return status_e;
            }

            for (int i = 0; i < 10 * x; i++) {
                printf("%lld", table[i]);
                if ((i + 1) % x == 0) printf("\n");
                else printf(" ");
            }
            break;
        case 'a':
            long long sum;
            Status status_a = one_to_x_sum(&sum, &x);

            if (status_a != STATUS_OK) {
                printf("Status: %d\n", status_a);
                return status_a;
            }

            printf("Sum 1 to %lld is %lld\n", x, sum);
            break;
        case 'f':
            long long fact;
            Status status_f = factorial(&fact, &x);

            if (status_f != STATUS_OK) {
                printf("Status: %d\n", status_f);
                return status_f;
            }

            printf("%lld factorial is %lld\n", x, fact);
            break;
        default:
            printf("Status: %d\n", STATUS_WRONG_FLAG);
            return STATUS_WRONG_FLAG;
    }

    printf("Status %d\n", STATUS_OK);
    return STATUS_OK;
}