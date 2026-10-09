#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <float.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_COUNT_ARGS_ERR = -1,
    STATUS_INVALID_ARGS = -2,
    STATUS_WRONG_FLAG = -3,
    STATUS_OVERFLOW = -4,
    STATUS_WRONG_NUMBER = -5
} Status;

static Status parse_int_number(const char *str1, const char *str2,
                               long long *num1, long long *num2)
{
    if (!str1 || !str2 || !num1 || !num2) return STATUS_INVALID_ARGS;

    *num1 = 0;
    *num2 = 0;
    char sign1 = 0;
    char sign2 = 0;

    for (int i = 0; str1[i] != '\0'; i++) {
        if (str1[i] == '-' && i == 0) {
            sign1 = 1;
            continue;
        }

        if (str1[i] < '0' || str1[i] > '9')
                return STATUS_WRONG_NUMBER;

        int digit = str1[i] - '0';

        if (*num1 > (LLONG_MAX - digit) / 10)
            return STATUS_OVERFLOW;

        *num1 = *num1 * 10 + digit;  
    }

    for (int i = 0; str2[i] != '\0'; i++) {
        if (str2[i] == '-' && i == 0) {
            sign2 = 1;
            continue;
        }

        if (str2[i] < '0' || str2[i] > '9')
            return STATUS_WRONG_NUMBER;

        int digit = str2[i] - '0';

        if (*num2 > (LLONG_MAX - digit) / 10)
            return STATUS_OVERFLOW;

        *num2 = *num2 * 10 + digit;
    }

    if (sign1 == 1) *num1 *= -1;
    if (sign2 == 1) *num2 *= -1;

    if (*num1 == 0 || *num2 == 0)
        return STATUS_WRONG_NUMBER;

    return STATUS_OK;
}

static Status parse_real_number(const char *str1, const char *str2,
                                const char *str3, const char *str4,
                                long double *num1, long double *num2,
                                long double *num3, long double *num4)
{
    if (!str1 || !str2 || !str3 || !str4 ||
        !num1 || !num2 || !num3 || !num4)
        return STATUS_INVALID_ARGS;

    char *end1;
    char *end2;
    char *end3;
    char *end4;

    *num1 = strtold(str1, &end1);
    *num2 = strtold(str2, &end2);
    *num3 = strtold(str3, &end3);
    *num4 = strtold(str4, &end4);

    if (end1 == str1 || end2 == str2 ||
        end3 == str3 || end4 == str4)
        return STATUS_WRONG_NUMBER;

    if (*end1 != '\0' || *end2 != '\0' ||
        *end3 != '\0' || *end4 != '\0')
        return STATUS_WRONG_NUMBER;

    if (!isfinite(*num1) || !isfinite(*num2) ||
        !isfinite(*num3) || !isfinite(*num4))
        return STATUS_OVERFLOW;

    return STATUS_OK;
}

static Status is_multiple(const long long *a,
                          const long long *b,
                          char *ismultiple)
{
    if (!a || !b || !ismultiple)
        return STATUS_INVALID_ARGS;

    if (*a == 0 || *b == 0)
        return STATUS_WRONG_NUMBER;

    if (abs(*a) % abs(*b) == 0)
        *ismultiple = 1;
    else
        *ismultiple = 0;

    return STATUS_OK;
}

static Status is_right_triangle(const long double *eps,
                                const long double *x,
                                const long double *y,
                                const long double *z,
                                char *isright)
{
    if (!eps || !x || !y || !z || !isright)
        return STATUS_INVALID_ARGS;

    if (*eps <= 0 || *x <= 0 || *y <= 0 || *z <= 0)
        return STATUS_INVALID_ARGS;

    long double a = *x;
    long double b = *y;
    long double c = *z;

    if (a < b) {
        long double t = a;
        a = b;
        b = t;
    }

    if (a < c) {
        long double t = a;
        a = c;
        c = t;
    }

    if (a > LDBL_MAX / a ||
        b > LDBL_MAX / b ||
        c > LDBL_MAX / c)
        return STATUS_OVERFLOW;

    a *= a;
    b *= b;
    c *= c;

    if (a > LDBL_MAX - b)
        return STATUS_OVERFLOW;

    a += b;

    if (fabsl(a - c) < *eps)
        *isright = 1;
    else
        *isright = 0;

    return STATUS_OK;
}

static Status solve_equation(const long double *eps,
                             const long double *a,
                             const long double *b,
                             const long double *c,
                             long double *x1,
                             long double *x2,
                             int *root_count)
{
    if (!eps || !a || !b || !c ||
        !x1 || !x2 || !root_count)
        return STATUS_INVALID_ARGS;

    if (*eps <= 0)
        return STATUS_INVALID_ARGS;

    if (fabsl(*a) < *eps) {
        if (fabsl(*b) < *eps) {
            if (fabsl(*c) < *eps)
                *root_count = -1;
            else
                *root_count = 0;

            return STATUS_OK;
        }

        *x1 = -*c / *b;

        if (!isfinite(*x1))
            return STATUS_OVERFLOW;

        *root_count = 1;
        return STATUS_OK;
    }

    if (fabsl(*b) > sqrtl(LDBL_MAX))
        return STATUS_OVERFLOW;

    long double bb = *b * *b;

    if (*c != 0 && fabsl(*a) > LDBL_MAX / fabsl(*c))
        return STATUS_OVERFLOW;

    long double ac = *a * *c;

    if (fabsl(ac) > LDBL_MAX / 4.0L)
        return STATUS_OVERFLOW;

    long double four_ac = 4.0L * ac;

    if ((bb >= 0 && four_ac < 0) ||
        (bb < 0 && four_ac >= 0)) {
        if (fabsl(bb) > LDBL_MAX - fabsl(four_ac))
            return STATUS_OVERFLOW;
    }

    long double d = bb - four_ac;

    if (fabsl(d) < *eps)
        d = 0;

    if (d < 0) {
        *root_count = 0;
        return STATUS_OK;
    }

    long double denominator = 2.0L * *a;

    if (!isfinite(denominator))
        return STATUS_OVERFLOW;

    if (d == 0) {
        *x1 = -*b / denominator;

        if (!isfinite(*x1))
            return STATUS_OVERFLOW;

        *root_count = 1;
        return STATUS_OK;
    }

    long double sqrt_d = sqrtl(d);

    *x1 = (-*b + sqrt_d) / denominator;
    *x2 = (-*b - sqrt_d) / denominator;

    if (!isfinite(*x1) || !isfinite(*x2))
        return STATUS_OVERFLOW;

    *root_count = 2;

    return STATUS_OK;
}

static Status make_unique_permutations(
    const long double *eps,
    const long double *a,
    const long double *b,
    const long double *c,
    long double permutations[6][3],
    int *count)
{
    if (!eps || !a || !b || !c || !permutations || !count)
        return STATUS_INVALID_ARGS;

    if (*eps <= 0)
        return STATUS_INVALID_ARGS;

    long double all[6][3] = {
        {*a, *b, *c},
        {*a, *c, *b},
        {*b, *a, *c},
        {*b, *c, *a},
        {*c, *a, *b},
        {*c, *b, *a}
    };

    *count = 0;

    for (int i = 0; i < 6; i++) {
        int duplicate = 0;

        for (int j = 0; j < *count; j++) {
            if (fabsl(all[i][0] - permutations[j][0]) < *eps &&
                fabsl(all[i][1] - permutations[j][1]) < *eps &&
                fabsl(all[i][2] - permutations[j][2]) < *eps) {
                duplicate = 1;
                break;
            }
        }

        if (!duplicate) {
            permutations[*count][0] = all[i][0];
            permutations[*count][1] = all[i][1];
            permutations[*count][2] = all[i][2];
            (*count)++;
        }
    }

    return STATUS_OK;
}

int main(int argc, char *argv[])
{
    if (argc <= 2) {
        printf("Status: %d\n", STATUS_COUNT_ARGS_ERR);
        return STATUS_COUNT_ARGS_ERR;
    }

    if (strlen(argv[1]) != 2) {
        printf("Status: %d\n", STATUS_WRONG_FLAG);
        return STATUS_WRONG_FLAG;
    }

    if (argv[1][0] != '-' && argv[1][0] != '/') {
        printf("Status: %d\n", STATUS_WRONG_FLAG);
        return STATUS_WRONG_FLAG;
    }

    char flag = argv[1][1];

    switch (flag) {
        case 'q': {
            if (argc != 6) {
                printf("Status: %d\n", STATUS_COUNT_ARGS_ERR);
                return STATUS_COUNT_ARGS_ERR;
            }

            long double eps;
            long double a;
            long double b;
            long double c;

            Status status_q =
                parse_real_number(argv[2], argv[3], argv[4], argv[5],
                                  &eps, &a, &b, &c);

            if (status_q != STATUS_OK) {
                printf("Status: %d\n", status_q);
                return status_q;
            }

            if (eps <= 0) {
                printf("Status: %d\n", STATUS_INVALID_ARGS);
                return STATUS_INVALID_ARGS;
            }

            long double permutations[6][3];
            int permutation_count;

            status_q = make_unique_permutations(
                &eps, &a, &b, &c,
                permutations, &permutation_count
            );

            if (status_q != STATUS_OK) {
                printf("Status: %d\n", status_q);
                return status_q;
            }

            for (int i = 0; i < permutation_count; i++) {
                long double x1 = 0;
                long double x2 = 0;
                int root_count = 0;

                status_q = solve_equation(
                    &eps,
                    &permutations[i][0],
                    &permutations[i][1],
                    &permutations[i][2],
                    &x1,
                    &x2,
                    &root_count
                );

                if (status_q != STATUS_OK) {
                    printf("Status: %d\n", status_q);
                    return status_q;
                }

                printf("%Lf %Lf %Lf:\n",
                       permutations[i][0],
                       permutations[i][1],
                       permutations[i][2]);

                if (root_count == -1) {
                    printf("Any real number is a solution\n");
                } else if (root_count == 0) {
                    printf("No solutions\n");
                } else if (root_count == 1) {
                    printf("%Lf\n", x1);
                } else {
                    printf("%Lf %Lf\n", x1, x2);
                }
            }

            break;
        }

        case 'm': {
            if (argc != 4) {
                printf("Status: %d\n", STATUS_COUNT_ARGS_ERR);
                return STATUS_COUNT_ARGS_ERR;
            }

            long long a;
            long long b;
            char ismultiple;

            Status status_m =
                parse_int_number(argv[2], argv[3], &a, &b);

            if (status_m != STATUS_OK) {
                printf("Status: %d\n", status_m);
                return status_m;
            }

            status_m = is_multiple(&a, &b, &ismultiple);

            if (status_m != STATUS_OK) {
                printf("Status: %d\n", status_m);
                return status_m;
            }

            if (ismultiple == 0)
                printf("%lld is not multiple of %lld\n", a, b);
            else
                printf("%lld is multiple of %lld\n", a, b);

            break;
        }

        case 't': {
            if (argc != 6) {
                printf("Status: %d\n", STATUS_COUNT_ARGS_ERR);
                return STATUS_COUNT_ARGS_ERR;
            }

            long double eps;
            long double x;
            long double y;
            long double z;
            char isright;

            Status status_t =
                parse_real_number(argv[2], argv[3], argv[4], argv[5],
                                  &eps, &x, &y, &z);

            if (status_t != STATUS_OK) {
                printf("Status: %d\n", status_t);
                return status_t;
            }

            if (eps <= 0) {
                printf("Status: %d\n", STATUS_INVALID_ARGS);
                return STATUS_INVALID_ARGS;
            }

            status_t = is_right_triangle(
                &eps, &x, &y, &z, &isright
            );

            if (status_t != STATUS_OK) {
                printf("Status: %d\n", status_t);
                return status_t;
            }

            if (isright == 0)
                printf("%Lf %Lf %Lf is not right triangle\n",
                       x, y, z);
            else
                printf("%Lf %Lf %Lf is right triangle\n",
                       x, y, z);

            break;
        }

        default:
            printf("Status: %d\n", STATUS_WRONG_FLAG);
            return STATUS_WRONG_FLAG;
    }

    printf("Status: %d\n", STATUS_OK);
    return STATUS_OK;
}