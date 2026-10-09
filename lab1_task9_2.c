#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_VAL     -1000
#define MAX_VAL     1000
#define MIN_SIZE    10
#define MAX_SIZE    10000

typedef enum {
    STATUS_OK = 0,
    STATUS_INVALID_ARGS = -2,
    STATUS_ERR_MALLOC = -8,
} Status;

static Status quick_sort(int *array, int left, int right)
{
    if (array == NULL)
        return STATUS_INVALID_ARGS;

    if (left >= right)
        return STATUS_OK;

    int i = left;
    int j = right;
    int pivot = array[left + (right - left) / 2];

    while (i <= j) {
        while (array[i] < pivot)
            i++;

        while (array[j] > pivot)
            j--;

        if (i <= j) {
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;

            i++;
            j--;
        }
    }

    Status status = quick_sort(array, left, j);

    if (status != STATUS_OK)
        return status;

    return quick_sort(array, i, right);
}

static Status find_nearest(const int *B, int sizeB, int value, int *nearest)
{
    if (B == NULL || nearest == NULL || sizeB <= 0)
        return STATUS_INVALID_ARGS;

    int left = 0;
    int right = sizeB - 1;

    if (value <= B[0]) {
        *nearest = B[0];
        return STATUS_OK;
    }

    if (value >= B[sizeB - 1]) {
        *nearest = B[sizeB - 1];
        return STATUS_OK;
    }

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (B[mid] == value) {
            *nearest = B[mid];
            return STATUS_OK;
        }

        if (B[mid] < value)
            left = mid + 1;
        else
            right = mid - 1;
    }

    if (value - B[right] <= B[left] - value)
        *nearest = B[right];
    else
        *nearest = B[left];

    return STATUS_OK;
}

static Status fill_array(int *array, int size)
{
    if (array == NULL || size <= 0)
        return STATUS_INVALID_ARGS;

    for (int i = 0; i < size; i++)
        array[i] = MIN_VAL + rand() % (MAX_VAL - MIN_VAL + 1);

    return STATUS_OK;
}

static Status create_array(int **array, int size)
{
    if (array == NULL || size <= 0)
        return STATUS_INVALID_ARGS;

    *array = malloc((size_t)size * sizeof(int));

    if (*array == NULL)
        return STATUS_ERR_MALLOC;

    return STATUS_OK;
}

int main(void)
{
    srand((unsigned)time(NULL));

    Status status = STATUS_OK;

    int sizeA = MIN_SIZE + rand() % (MAX_SIZE - MIN_SIZE + 1);
    int sizeB = MIN_SIZE + rand() % (MAX_SIZE - MIN_SIZE + 1);

    int *A = NULL;
    int *B = NULL;
    int *C = NULL;

    status = create_array(&A, sizeA);

    if (status != STATUS_OK)
        goto EXIT;

    status = create_array(&B, sizeB);

    if (status != STATUS_OK)
        goto EXIT;

    status = create_array(&C, sizeA);

    if (status != STATUS_OK)
        goto EXIT;

    status = fill_array(A, sizeA);

    if (status != STATUS_OK)
        goto EXIT;

    status = fill_array(B, sizeB);

    if (status != STATUS_OK)
        goto EXIT;

    status = quick_sort(B, 0, sizeB - 1);

    if (status != STATUS_OK)
        goto EXIT;

    for (int i = 0; i < sizeA; i++) {
        int nearest = 0;

        status = find_nearest(B, sizeB, A[i], &nearest);

        if (status != STATUS_OK)
            goto EXIT;

        C[i] = A[i] + nearest;
    }

    printf("Size A: %d\n", sizeA);
    printf("Size B: %d\n", sizeB);

    printf("\nA:\n");
    for (int i = 0; i < sizeA; i++)
        printf("%d ", A[i]);

    printf("\n\nB:\n");
    for (int i = 0; i < sizeB; i++)
        printf("%d ", B[i]);

    printf("\n\nC:\n");
    for (int i = 0; i < sizeA; i++)
        printf("%d ", C[i]);

    printf("\n");

EXIT:
    free(A);
    free(B);
    free(C);

    printf("\nStatus: %d\n", status);

    return status;
}