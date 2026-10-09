#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGS = -2,
    STATUS_OVERFLOW = -4
} Status;

static Status parse_epsilon(const char *str, double *epsilon)
{
    if (!str || !epsilon)
        return STATUS_INVALID_ARGS;

    char *end;
    double value = strtod(str, &end);

    if (*str == '\0' || *end != '\0' || value <= 0.0)
        return STATUS_INVALID_ARGS;

    if (!isfinite(value))
        return STATUS_OVERFLOW;

    *epsilon = value;

    return STATUS_OK;
}

static Status parse_x(const char *str, double *x)
{
    if (!str || !x)
        return STATUS_INVALID_ARGS;

    char *end;
    double value = strtod(str, &end);

    if (*str == '\0' || *end != '\0')
        return STATUS_INVALID_ARGS;

    if (!isfinite(value))
        return STATUS_OVERFLOW;

    *x = value;

    return STATUS_OK;
}

static Status calculate_a(double x, double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double sum = 1.0;
    double term = 1.0;
    unsigned long long n = 1;

    while (fabs(term) > epsilon) {
        term *= x / (double)n;

        if (!isfinite(term))
            return STATUS_OVERFLOW;

        sum += term;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;

        if (n == ULLONG_MAX)
            return STATUS_OVERFLOW;

        n++;
    }

    *result = sum;

    return STATUS_OK;
}

static Status calculate_b(double x, double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double sum = 1.0;
    double term = 1.0;
    unsigned long long n = 1;

    while (fabs(term) > epsilon) {
        term *= -x * x;
        term /= (double)(2 * n - 1);
        term /= (double)(2 * n);

        if (!isfinite(term))
            return STATUS_OVERFLOW;

        sum += term;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;

        if (n > (ULLONG_MAX - 2) / 2)
            return STATUS_OVERFLOW;

        n++;
    }

    *result = sum;

    return STATUS_OK;
}

static Status calculate_c(double x, double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    if (fabs(x) >= 1.0)
        return STATUS_INVALID_ARGS;

    double sum = 1.0;
    double term = 1.0;
    unsigned long long n = 1;

    while (fabs(term) > epsilon) {
        double dn = (double)n;

        term *= 27.0 * dn * dn * dn * x * x;
        term /= (3.0 * dn) * (3.0 * dn - 1.0) * (3.0 * dn - 2.0);

        if (!isfinite(term))
            return STATUS_OVERFLOW;

        sum += term;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;

        if (n == ULLONG_MAX)
            return STATUS_OVERFLOW;

        n++;
    }

    *result = sum;

    return STATUS_OK;
}

static Status calculate_d(double x, double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    if (fabs(x) > 1.0)
        return STATUS_INVALID_ARGS;

    double sum = 0.0;
    double term = 1.0;
    unsigned long long n = 1;

    while (1) {
        term *= -x * x;
        term *= (double)(2 * n - 1) / (double)(2 * n);

        if (!isfinite(term))
            return STATUS_OVERFLOW;

        if (fabs(term) <= epsilon)
            break;

        sum += term;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;

        if (n > (ULLONG_MAX - 2) / 2)
            return STATUS_OVERFLOW;

        n++;
    }

    *result = sum;

    return STATUS_OK;
}

int main(int argc, char *argv[])
{
    Status status = STATUS_OK;

    double epsilon;
    double x;

    double result_a;
    double result_b;
    double result_c;
    double result_d;

    if (argc != 3) {
        status = STATUS_INVALID_ARGS;
        goto EXIT;
    }

    status = parse_epsilon(argv[1], &epsilon);
    if (status != STATUS_OK)
        goto EXIT;

    status = parse_x(argv[2], &x);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_a(x, epsilon, &result_a);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_b(x, epsilon, &result_b);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_c(x, epsilon, &result_c);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_d(x, epsilon, &result_d);
    if (status != STATUS_OK)
        goto EXIT;

    printf("a: %.15f\n", result_a);
    printf("b: %.15f\n", result_b);
    printf("c: %.15f\n", result_c);
    printf("d: %.15f\n", result_d);

EXIT:
    printf("Status: %d\n", status);

    return status;
}