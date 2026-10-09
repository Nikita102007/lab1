#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>

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

static Status calculate_e_series(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double sum = 1.0;
    double term = 1.0;
    unsigned long long n = 1;

    while (fabs(term) > epsilon) {
        term /= (double)n;

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

static Status calculate_e_equation(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double x = 2.0;

    while (1) {
        double next = x * (2.0 - log(x));

        if (!isfinite(next))
            return STATUS_OVERFLOW;

        if (fabs(next - x) <= epsilon) {
            *result = next;
            return STATUS_OK;
        }

        if (next == x)
            return STATUS_OVERFLOW;

        x = next;
    }
}

static Status calculate_e_limit(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double previous = 1.0;

    for (unsigned long long n = 1; n < ULLONG_MAX; n++) {
        double current = pow(
            1.0 + 1.0 / (double)n,
            (double)n);

        if (!isfinite(current))
            return STATUS_OVERFLOW;

        if (fabs(current - previous) * (double)n <= epsilon) {
            *result = current;
            return STATUS_OK;
        }

        previous = current;
    }

    return STATUS_OVERFLOW;
}

static Status calculate_pi_series(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double sum = 0.0;
    double term;
    unsigned long long n = 1;

    while (1) {
        term = 4.0 / (double)(2 * n - 1);

        if (n % 2 == 0)
            sum -= term;
        else
            sum += term;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;

        if (fabs(term) <= epsilon) {
            *result = sum;
            return STATUS_OK;
        }

        if (n > (ULLONG_MAX - 1) / 2)
            return STATUS_OVERFLOW;

        n++;
    }
}

static Status calculate_pi_equation(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double x = 3.0;

    while (1) {
        double sin_x = sin(x);
        double cos_x = cos(x);

        if (!isfinite(sin_x) || !isfinite(cos_x))
            return STATUS_OVERFLOW;

        if (fabs(sin_x) < DBL_MIN)
            return STATUS_OVERFLOW;

        double next = x + (cos_x + 1.0) / sin_x;

        if (!isfinite(next))
            return STATUS_OVERFLOW;

        if (fabs(next - x) <= epsilon) {
            *result = next;
            return STATUS_OK;
        }

        if (next == x)
            return STATUS_OVERFLOW;

        x = next;
    }
}

static Status calculate_pi_limit(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double previous = 4.0;

    for (unsigned long long n = 2; n < ULLONG_MAX; n++) {
        double dn = (double)n;
        double current = previous *
            4.0 * dn * (dn - 1.0) /
            ((2.0 * dn - 1.0) * (2.0 * dn - 1.0));

        if (!isfinite(current))
            return STATUS_OVERFLOW;

        if (fabs(current - previous) * dn <= epsilon) {
            *result = current;
            return STATUS_OK;
        }

        previous = current;
    }

    return STATUS_OVERFLOW;
}

static Status calculate_ln2_series(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double sum = 0.0;
    double term;
    unsigned long long n = 1;

    while (1) {
        term = 1.0 / (double)n;

        if (n % 2 == 0)
            sum -= term;
        else
            sum += term;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;

        if (fabs(term) <= epsilon) {
            *result = sum;
            return STATUS_OK;
        }

        if (n == ULLONG_MAX)
            return STATUS_OVERFLOW;

        n++;
    }
}

static Status calculate_ln2_equation(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double x = 0.5;

    while (1) {
        double exp_x = exp(x);

        if (!isfinite(exp_x) || exp_x == 0.0)
            return STATUS_OVERFLOW;

        double next = x - (exp_x - 2.0) / exp_x;

        if (!isfinite(next))
            return STATUS_OVERFLOW;

        if (fabs(next - x) <= epsilon) {
            *result = next;
            return STATUS_OK;
        }

        if (next == x)
            return STATUS_OVERFLOW;

        x = next;
    }
}

static Status calculate_ln2_limit(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double previous = 0.0;

    for (unsigned long long n = 1; n < ULLONG_MAX; n++) {
        double dn = (double)n;
        double current = dn * expm1(log(2.0) / dn);

        if (!isfinite(current))
            return STATUS_OVERFLOW;

        if (n > 1 && fabs(current - previous) * dn <= epsilon) {
            *result = current;
            return STATUS_OK;
        }

        previous = current;
    }

    return STATUS_OVERFLOW;
}

static Status calculate_sqrt2_series(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double product = 1.0;
    double previous = 1.0;

    for (unsigned long long k = 2; k < ULLONG_MAX; k++) {
        double exponent = pow(2.0, -(double)k);
        double factor = pow(2.0, exponent);

        if (!isfinite(factor))
            return STATUS_OVERFLOW;

        product *= factor;

        if (!isfinite(product))
            return STATUS_OVERFLOW;

        if (fabs(product - previous) <= epsilon) {
            *result = product;
            return STATUS_OK;
        }

        previous = product;
    }

    return STATUS_OVERFLOW;
}

static Status calculate_sqrt2_equation(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double x = 1.0;

    while (1) {
        if (fabs(x) < DBL_MIN)
            return STATUS_OVERFLOW;

        double next = (x + 2.0 / x) / 2.0;

        if (!isfinite(next))
            return STATUS_OVERFLOW;

        if (fabs(next - x) <= epsilon) {
            *result = next;
            return STATUS_OK;
        }

        if (next == x)
            return STATUS_OVERFLOW;

        x = next;
    }
}

static Status calculate_sqrt2_limit(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double x = -0.5;

    while (1) {
        double next = x - x * x / 2.0 + 1.0;

        if (!isfinite(next))
            return STATUS_OVERFLOW;

        if (fabs(next - x) <= epsilon) {
            *result = next;
            return STATUS_OK;
        }

        if (next == x)
            return STATUS_OVERFLOW;

        x = next;
    }
}

static Status is_prime(unsigned long long n, int *result)
{
    if (!result)
        return STATUS_INVALID_ARGS;

    if (n < 2) {
        *result = 0;
        return STATUS_OK;
    }

    if (n == 2) {
        *result = 1;
        return STATUS_OK;
    }

    if (n % 2 == 0) {
        *result = 0;
        return STATUS_OK;
    }

    for (unsigned long long i = 3; i <= n / i; i += 2) {
        if (n % i == 0) {
            *result = 0;
            return STATUS_OK;
        }
    }

    *result = 1;

    return STATUS_OK;
}

static Status calculate_gamma_series(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double pi;
    Status status = calculate_pi_series(epsilon / 2.0, &pi);

    if (status != STATUS_OK)
        return status;

    double sum = -pi * pi / 6.0;

    for (unsigned long long m = 1; m < 4294967295ULL; m++) {
        double inv = 1.0 / ((double)m * (double)m);
        unsigned long long start = m * m;
        unsigned long long end = start + 2 * m;
        double block = 0.0;

        for (unsigned long long k = start; k <= end; k++)
            block += inv - 1.0 / (double)k;

        sum += block;

        if (!isfinite(sum))
            return STATUS_OVERFLOW;

        if (fabs(block) <= epsilon * epsilon / 2.0) {
            *result = sum;
            return STATUS_OK;
        }
    }

    return STATUS_OVERFLOW;
}

static Status calculate_gamma_equation(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    long double previous = 0.0L;

    for (unsigned long long m = 1; m < ULLONG_MAX; m++) {
        long double term = 0.0L;
        long double binomial = 1.0L;
        long double log_factorial = 0.0L;

        for (unsigned long long k = 1; k <= m; k++) {
            binomial *= (long double)(m - k + 1) / (long double)k;
            log_factorial += logl((long double)k);

            long double part = binomial / (long double)k * log_factorial;

            if (k % 2 == 1)
                part = -part;

            term += part;

            if (!isfinite(term))
                return STATUS_OVERFLOW;
        }

        if (m > 1 && fabsl(term - previous) <= (long double)epsilon) {
            *result = (double)term;
            return STATUS_OK;
        }

        previous = term;
    }

    return STATUS_OVERFLOW;
}

static Status calculate_gamma_limit(double epsilon, double *result)
{
    if (!result || epsilon <= 0.0)
        return STATUS_INVALID_ARGS;

    double log_product = 0.0;
    double previous = 0.0;

    for (unsigned long long t = 2; t < ULLONG_MAX; t++) {
        int prime;

        Status status = is_prime(t, &prime);

        if (status != STATUS_OK)
            return status;

        if (prime) {
            double factor = log1p(-1.0 / (double)t);

            if (!isfinite(factor))
                return STATUS_OVERFLOW;

            log_product += factor;

            if (!isfinite(log_product))
                return STATUS_OVERFLOW;
        }

        double current = exp(log(log((double)t)) + log_product);

        if (!isfinite(current))
            return STATUS_OVERFLOW;

        if (t > 2 && fabs(current - previous) <= epsilon) {
            *result = -log(current);
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

    double e_series;
    double e_equation;
    double e_limit;

    double pi_series;
    double pi_equation;
    double pi_limit;

    double ln2_series;
    double ln2_equation;
    double ln2_limit;

    double sqrt2_series;
    double sqrt2_equation;
    double sqrt2_limit;

    double gamma_series;
    double gamma_equation;
    double gamma_limit;

    if (argc != 2) {
        status = STATUS_INVALID_ARGS;
        goto EXIT;
    }

    status = parse_epsilon(argv[1], &epsilon);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_e_series(epsilon, &e_series);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_e_equation(epsilon, &e_equation);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_e_limit(epsilon, &e_limit);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_pi_series(epsilon, &pi_series);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_pi_equation(epsilon, &pi_equation);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_pi_limit(epsilon, &pi_limit);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_ln2_series(epsilon, &ln2_series);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_ln2_equation(epsilon, &ln2_equation);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_ln2_limit(epsilon, &ln2_limit);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_sqrt2_series(epsilon, &sqrt2_series);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_sqrt2_equation(epsilon, &sqrt2_equation);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_sqrt2_limit(epsilon, &sqrt2_limit);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_gamma_series(epsilon, &gamma_series);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_gamma_equation(epsilon, &gamma_equation);
    if (status != STATUS_OK)
        goto EXIT;

    status = calculate_gamma_limit(epsilon, &gamma_limit);
    if (status != STATUS_OK)
        goto EXIT;

    printf("e (series):       %.15f\n", e_series);
    printf("e (equation):     %.15f\n", e_equation);
    printf("e (limit):        %.15f\n", e_limit);

    printf("pi (series):      %.15f\n", pi_series);
    printf("pi (equation):    %.15f\n", pi_equation);
    printf("pi (limit):       %.15f\n", pi_limit);

    printf("ln2 (series):     %.15f\n", ln2_series);
    printf("ln2 (equation):   %.15f\n", ln2_equation);
    printf("ln2 (limit):      %.15f\n", ln2_limit);

    printf("sqrt2 (series):   %.15f\n", sqrt2_series);
    printf("sqrt2 (equation): %.15f\n", sqrt2_equation);
    printf("sqrt2 (limit):    %.15f\n", sqrt2_limit);

    printf("gamma (series):   %.15f\n", gamma_series);
    printf("gamma (equation): %.15f\n", gamma_equation);
    printf("gamma (limit):    %.15f\n", gamma_limit);

EXIT:
    printf("Status: %d\n", status);

    return status;
}