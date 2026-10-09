#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_N (1ULL << 28)

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGS = -2,
    STATUS_OVERFLOW = -4
} Status;

typedef Status (*Function)(double, double *);
typedef Status (*Rule)(Function, double, double, unsigned long long, double *);

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

static Status function_a(double x, double *result)
{
    if (!result)
        return STATUS_INVALID_ARGS;

    if (x == 0.0)
        *result = 1.0;
    else
        *result = log1p(x) / x;

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status function_b(double x, double *result)
{
    if (!result)
        return STATUS_INVALID_ARGS;

    *result = exp(-x * x / 2.0);

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status function_c(double x, double *result)
{
    if (!result)
        return STATUS_INVALID_ARGS;

    if (x >= 1.0)
        return STATUS_OVERFLOW;

    *result = -log1p(-x);

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status function_d(double x, double *result)
{
    if (!result)
        return STATUS_INVALID_ARGS;

    if (x == 0.0)
        *result = 1.0;
    else
        *result = exp(x * log(x));

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status trapezoid(
    Function function,
    double left,
    double right,
    unsigned long long n,
    double *result)
{
    if (!function || !result || n == 0 || left >= right)
        return STATUS_INVALID_ARGS;

    double h = (right - left) / (double)n;
    double value_left;
    double value_right;
    double value;
    double sum;

    Status status;

    status = function(left, &value_left);
    if (status != STATUS_OK)
        return status;

    status = function(right, &value_right);
    if (status != STATUS_OK)
        return status;

    sum = (value_left + value_right) / 2.0;

    for (unsigned long long i = 1; i < n; i++) {
        double x = left + (double)i * h;

        status = function(x, &value);
        if (status != STATUS_OK)
            return status;

        sum += value;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;
    }

    *result = sum * h;

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status midpoint(
    Function function,
    double left,
    double right,
    unsigned long long n,
    double *result)
{
    if (!function || !result || n == 0 || left >= right)
        return STATUS_INVALID_ARGS;

    double h = (right - left) / (double)n;
    double value;
    double sum = 0.0;

    Status status;

    for (unsigned long long i = 0; i < n; i++) {
        double x = left + ((double)i + 0.5) * h;

        status = function(x, &value);
        if (status != STATUS_OK)
            return status;

        sum += value;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;
    }

    *result = sum * h;

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status integrate(
    Function function,
    Rule rule,
    double left,
    double right,
    double epsilon,
    double *result)
{
    if (!function || !rule || !result || epsilon <= 0.0 || left >= right)
        return STATUS_INVALID_ARGS;

    unsigned long long n = 2;
    double previous;
    double current;
    Status status;

    status = rule(function, left, right, n, &previous);
    if (status != STATUS_OK)
        return status;

    while (n < MAX_N) {
        n *= 2;

        status = rule(function, left, right, n, &current);
        if (status != STATUS_OK)
            return status;

        if (fabs(current - previous) <= epsilon) {
            *result = current;
            return STATUS_OK;
        }

        previous = current;
    }

    return STATUS_OVERFLOW;
}

int main(int argc, char *argv[])
{
    Status status = STATUS_OK;

    double epsilon;

    double result_a;
    double result_b;
    double result_c;
    double result_d;

    if (argc != 2) {
        status = STATUS_INVALID_ARGS;
        goto EXIT;
    }

    status = parse_epsilon(argv[1], &epsilon);
    if (status != STATUS_OK)
        goto EXIT;

    status = integrate(function_a, trapezoid, 0.0, 1.0, epsilon, &result_a);
    if (status != STATUS_OK)
        goto EXIT;

    status = integrate(function_b, trapezoid, 0.0, 1.0, epsilon, &result_b);
    if (status != STATUS_OK)
        goto EXIT;

    status = integrate(function_c, midpoint, 0.0, 1.0, epsilon, &result_c);
    if (status != STATUS_OK)
        goto EXIT;

    status = integrate(function_d, trapezoid, 0.0, 1.0, epsilon, &result_d);
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