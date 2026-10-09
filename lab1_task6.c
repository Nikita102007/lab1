#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <limits.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGS = -2,
    STATUS_NO_MEMORY = -3,
    STATUS_OVERFLOW = -4
} Status;

typedef struct {
    double x;
    double y;
} Point;

static Status calculate_convex(int count, int *result, ...)
{
    if (!result || count < 3)
        return STATUS_INVALID_ARGS;

    Point *points = malloc((size_t)count * sizeof(Point));

    if (!points)
        return STATUS_NO_MEMORY;

    va_list args;
    va_start(args, result);

    for (int i = 0; i < count; i++) {
        points[i].x = va_arg(args, double);
        points[i].y = va_arg(args, double);

        if (!isfinite(points[i].x) || !isfinite(points[i].y)) {
            va_end(args);
            free(points);
            return STATUS_INVALID_ARGS;
        }
    }

    va_end(args);

    const double pi = acos(-1.0);
    int sign = 0;
    double total = 0.0;

    for (int i = 0; i < count; i++) {
        Point a = points[i];
        Point b = points[(i + 1) % count];
        Point c = points[(i + 2) % count];

        double abx = b.x - a.x;
        double aby = b.y - a.y;
        double bcx = c.x - b.x;
        double bcy = c.y - b.y;

        double cross = abx * bcy - aby * bcx;
        double dot = abx * bcx + aby * bcy;

        if (!isfinite(cross) || !isfinite(dot)) {
            free(points);
            return STATUS_OVERFLOW;
        }

        double tolerance = 1e-12 * hypot(abx, aby) * hypot(bcx, bcy);

        if (fabs(cross) <= tolerance) {
            if (dot < 0.0) {
                *result = 0;
                free(points);
                return STATUS_OK;
            }

            continue;
        }

        int current_sign = cross > 0.0 ? 1 : -1;

        if (sign == 0)
            sign = current_sign;
        else if (sign != current_sign) {
            *result = 0;
            free(points);
            return STATUS_OK;
        }

        total += atan2(cross, dot);
    }

    free(points);

    if (sign == 0 || fabs(fabs(total) - 2.0 * pi) > 1e-9)
        *result = 0;
    else
        *result = 1;

    return STATUS_OK;
}

static Status calculate_polynomial(double x, int n, double *result, ...)
{
    if (!result || n < 0 || !isfinite(x))
        return STATUS_INVALID_ARGS;

    va_list args;
    va_start(args, result);

    double coefficient = va_arg(args, double);

    if (!isfinite(coefficient)) {
        va_end(args);
        return STATUS_INVALID_ARGS;
    }

    double value = coefficient;

    for (int i = 1; i <= n; i++) {
        coefficient = va_arg(args, double);

        if (!isfinite(coefficient)) {
            va_end(args);
            return STATUS_INVALID_ARGS;
        }

        value = value * x + coefficient;

        if (!isfinite(value)) {
            va_end(args);
            return STATUS_OVERFLOW;
        }
    }

    va_end(args);

    *result = value;

    return STATUS_OK;
}

static Status digit_value(char c, int *result)
{
    if (!result)
        return STATUS_INVALID_ARGS;

    if (c >= '0' && c <= '9')
        *result = c - '0';
    else if (c >= 'A' && c <= 'Z')
        *result = c - 'A' + 10;
    else if (c >= 'a' && c <= 'z')
        *result = c - 'a' + 10;
    else
        return STATUS_INVALID_ARGS;

    return STATUS_OK;
}

static Status string_to_number(const char *str, int base, unsigned long long *result)
{
    if (!str || !result || base < 2 || base > 36 || *str == '\0')
        return STATUS_INVALID_ARGS;

    unsigned long long value = 0;

    for (size_t i = 0; str[i] != '\0'; i++) {
        int digit;
        Status status = digit_value(str[i], &digit);

        if (status != STATUS_OK || digit >= base)
            return STATUS_INVALID_ARGS;

        if (value > (ULLONG_MAX - (unsigned long long)digit) /
                    (unsigned long long)base)
            return STATUS_OVERFLOW;

        value = value * (unsigned long long)base +
                (unsigned long long)digit;
    }

    *result = value;

    return STATUS_OK;
}

static Status number_to_base(
    unsigned long long value,
    int base,
    char *buffer,
    size_t size)
{
    if (!buffer || size < 2 || base < 2 || base > 36)
        return STATUS_INVALID_ARGS;

    char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char temp[128];
    size_t length = 0;

    if (value == 0)
        temp[length++] = '0';

    while (value > 0) {
        if (length >= sizeof(temp))
            return STATUS_OVERFLOW;

        temp[length++] = digits[value % (unsigned long long)base];
        value /= (unsigned long long)base;
    }

    if (length + 1 > size)
        return STATUS_OVERFLOW;

    for (size_t i = 0; i < length; i++)
        buffer[i] = temp[length - i - 1];

    buffer[length] = '\0';

    return STATUS_OK;
}

static Status is_kaprekar(
    const char *str,
    int base,
    int *result)
{
    if (!str || !result)
        return STATUS_INVALID_ARGS;

    unsigned long long number;

    Status status = string_to_number(str, base, &number);

    if (status != STATUS_OK)
        return status;

    if (number == 0 || number == 1) {
        *result = 1;
        return STATUS_OK;
    }

    if (number > ULLONG_MAX / number)
        return STATUS_OVERFLOW;

    unsigned long long square = number * number;
    char square_str[128];

    status = number_to_base(square, base, square_str, sizeof(square_str));

    if (status != STATUS_OK)
        return status;

    size_t length = strlen(square_str);

    for (size_t split = 1; split < length; split++) {
        char left_str[128];
        char right_str[128];

        memcpy(left_str, square_str, split);
        left_str[split] = '\0';

        memcpy(right_str, square_str + split, length - split);
        right_str[length - split] = '\0';

        unsigned long long left;
        unsigned long long right;

        status = string_to_number(left_str, base, &left);
        if (status != STATUS_OK)
            return status;

        status = string_to_number(right_str, base, &right);
        if (status != STATUS_OK)
            return status;

        if (right == 0)
            continue;

        if (left + right == number) {
            *result = 1;
            return STATUS_OK;
        }
    }

    *result = 0;

    return STATUS_OK;
}

static Status find_kaprekar(int base, int count, int *found, ...)
{
    if (!found || count <= 0 || base < 2 || base > 36)
        return STATUS_INVALID_ARGS;

    *found = 0;

    va_list args;
    va_start(args, found);

    for (int i = 0; i < count; i++) {
        const char *str = va_arg(args, const char *);

        if (!str) {
            va_end(args);
            return STATUS_INVALID_ARGS;
        }

        int kaprekar;

        Status status = is_kaprekar(str, base, &kaprekar);

        if (status != STATUS_OK) {
            va_end(args);
            return status;
        }

        if (kaprekar) {
            *found = 1;
            printf("Kaprekar: %s\n", str);
        }
    }

    va_end(args);

    return STATUS_OK;
}

static Status calculate_geometric_mean(double *result, int count, ...)
{
    if (!result || count <= 0)
        return STATUS_INVALID_ARGS;

    va_list args;
    va_start(args, count);

    double log_sum = 0.0;

    for (int i = 0; i < count; i++) {
        double value = va_arg(args, double);

        if (!isfinite(value) || value <= 0.0) {
            va_end(args);
            return STATUS_INVALID_ARGS;
        }

        log_sum += log(value);

        if (!isfinite(log_sum)) {
            va_end(args);
            return STATUS_OVERFLOW;
        }
    }

    va_end(args);

    double mean = exp(log_sum / (double)count);

    if (!isfinite(mean))
        return STATUS_OVERFLOW;

    *result = mean;

    return STATUS_OK;
}

static Status fast_power(double x, long long n, double *result)
{
    if (!result || !isfinite(x))
        return STATUS_INVALID_ARGS;

    if (n == 0) {
        *result = 1.0;
        return STATUS_OK;
    }

    if (x == 0.0 && n < 0)
        return STATUS_INVALID_ARGS;

    unsigned long long exponent;

    if (n < 0)
        exponent = (unsigned long long)(-(n + 1)) + 1ULL;
    else
        exponent = (unsigned long long)n;

    double half;
    Status status;

    if (exponent % 2 == 0) {
        status = fast_power(x, (long long)(exponent / 2), &half);

        if (status != STATUS_OK)
            return status;

        *result = half * half;
    } else {
        status = fast_power(x, (long long)(exponent - 1), &half);

        if (status != STATUS_OK)
            return status;

        *result = x * half;
    }

    if (n < 0)
        *result = 1.0 / *result;

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status equation_one(double x, double *result)
{
    if (!result || !isfinite(x))
        return STATUS_INVALID_ARGS;

    *result = x * x - 2.0;

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status equation_two(double x, double *result)
{
    if (!result || !isfinite(x))
        return STATUS_INVALID_ARGS;

    *result = x * x * x - x - 2.0;

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status equation_three(double x, double *result)
{
    if (!result || !isfinite(x))
        return STATUS_INVALID_ARGS;

    *result = cos(x) - x;

    if (!isfinite(*result))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status bisection(
    double left,
    double right,
    double epsilon,
    Status (*function)(double, double *),
    double *result)
{
    if (!result || !function || epsilon <= 0.0 ||
        !isfinite(left) || !isfinite(right) ||
        left >= right)
        return STATUS_INVALID_ARGS;

    double f_left;
    double f_right;

    Status status = function(left, &f_left);
    if (status != STATUS_OK)
        return status;

    status = function(right, &f_right);
    if (status != STATUS_OK)
        return status;

    if (f_left == 0.0) {
        *result = left;
        return STATUS_OK;
    }

    if (f_right == 0.0) {
        *result = right;
        return STATUS_OK;
    }

    if ((f_left > 0.0) == (f_right > 0.0))
        return STATUS_INVALID_ARGS;

    while (right - left > epsilon) {
        double middle = left + (right - left) / 2.0;
        double f_middle;

        if (middle <= left || middle >= right)
            break;

        status = function(middle, &f_middle);
        if (status != STATUS_OK)
            return status;

        if (f_middle == 0.0) {
            *result = middle;
            return STATUS_OK;
        }

        if ((f_left > 0.0) != (f_middle > 0.0))
            right = middle;
        else {
            left = middle;
            f_left = f_middle;
        }
    }

    *result = left + (right - left) / 2.0;

    return STATUS_OK;
}

static void demo_bisection(
    const char *name,
    double left,
    double right,
    double epsilon,
    Status (*function)(double, double *))
{
    double root;
    Status status = bisection(left, right, epsilon, function, &root);

    if (status == STATUS_OK)
        printf("   %s, [%g, %g], eps = %g: %.15f\n",
               name, left, right, epsilon, root);
    else
        printf("   %s, [%g, %g], eps = %g: error %d\n",
               name, left, right, epsilon, status);
}

int main(int argc, char *argv[])
{
    Status status = STATUS_OK;

    double epsilon = 0.000001;

    int convex;
    double polynomial;
    int kaprekar_found;
    double geometric_mean;
    double power;

    if (argc == 2) {
        char *end;
        epsilon = strtod(argv[1], &end);

        if (*argv[1] == '\0' || *end != '\0' || epsilon <= 0.0) {
            status = STATUS_INVALID_ARGS;
            goto EXIT;
        }

        if (!isfinite(epsilon)) {
            status = STATUS_OVERFLOW;
            goto EXIT;
        }
    } else if (argc != 1) {
        status = STATUS_INVALID_ARGS;
        goto EXIT;
    }

    status = calculate_convex(
        4,
        &convex,
        0.0, 0.0,
        4.0, 0.0,
        4.0, 3.0,
        0.0, 3.0
    );

    if (status != STATUS_OK)
        goto EXIT;

    printf("1. Rectangle convex: %s\n", convex ? "yes" : "no");

    status = calculate_convex(
        5,
        &convex,
        0.0, 0.0,
        4.0, 0.0,
        4.0, 4.0,
        2.0, 1.0,
        0.0, 4.0
    );

    if (status != STATUS_OK)
        goto EXIT;

    printf("1. Concave pentagon convex: %s\n", convex ? "yes" : "no");

    status = calculate_convex(
        5,
        &convex,
        0.0, 3.0,
        2.0, -2.0,
        -1.5, 1.0,
        1.5, 1.0,
        -2.0, -2.0
    );

    if (status != STATUS_OK)
        goto EXIT;

    printf("1. Star convex: %s\n", convex ? "yes" : "no");

    status = calculate_polynomial(
        2.0,
        3,
        &polynomial,
        2.0,
        3.0,
        4.0,
        5.0
    );

    if (status != STATUS_OK)
        goto EXIT;

    printf("2. P(2) = %.15f\n", polynomial);

    status = find_kaprekar(
        10,
        9,
        &kaprekar_found,
        "1",
        "9",
        "45",
        "55",
        "99",
        "100",
        "297",
        "703",
        "999"
    );

    if (status != STATUS_OK)
        goto EXIT;

    printf("3. Kaprekar found: %s\n",
           kaprekar_found ? "yes" : "no");

    status = calculate_geometric_mean(
        &geometric_mean,
        4,
        1.0,
        2.0,
        4.0,
        8.0
    );

    if (status != STATUS_OK)
        goto EXIT;

    printf("4. Geometric mean: %.15f\n", geometric_mean);

    status = fast_power(2.0, 10, &power);

    if (status != STATUS_OK)
        goto EXIT;

    printf("5. 2^10 = %.15f\n", power);

    status = fast_power(2.0, -3, &power);

    if (status != STATUS_OK)
        goto EXIT;

    printf("5. 2^-3 = %.15f\n", power);

    printf("6. Bisection:\n");

    demo_bisection("x^2 - 2", 0.0, 2.0, epsilon, equation_one);
    demo_bisection("x^2 - 2", 0.0, 2.0, 1e-3, equation_one);
    demo_bisection("x^2 - 2", -2.0, 0.0, 1e-9, equation_one);
    demo_bisection("x^3 - x - 2", 1.0, 2.0, epsilon, equation_two);
    demo_bisection("x^3 - x - 2", 0.0, 5.0, 1e-9, equation_two);
    demo_bisection("cos(x) - x", 0.0, 1.0, 1e-12, equation_three);
    demo_bisection("x^2 - 2", 2.0, 3.0, epsilon, equation_one);

EXIT:
    printf("Status: %d\n", status);

    return status;
}