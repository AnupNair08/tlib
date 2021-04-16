#include <stdio.h>
#include <unistd.h>
#ifdef BUILD
#include <tlib.h>
#else
#include "../thread.h"
#endif
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "tests.h"
typedef struct arg_struct
{
    int rowFrom;
    int rowTo;
    int numCols1;
    int numCols2;
    int **res;
    int **arr1;
    int **arr2;
} arg_struct;
int readarray(int ***arr, int r, int c)
{
    *arr = (int **)malloc(r * sizeof(int *));
    if (*arr == NULL)
    {
        perror("Error");
        return errno;
    }
    for (int i = 0; i < r; i++)
    {
        (*arr)[i] = (int *)malloc(c * sizeof(int));
        if ((*arr)[i] == NULL)
        {
            perror("Error");
            return errno;
        }
    }
    for (int i = 0; i < r; i++)
    {
        for (int j = 0; j < c; j++)
        {
            scanf("%d", &((*arr)[i][j]));
        }
    }
    return 0;
}
void *partMatMul(void *argsStruct)
{
    arg_struct *args = (arg_struct *)argsStruct;
    int rowFrom = args->rowFrom;
    int rowTo = args->rowTo;
    int numCols1 = args->numCols1;
    int numCols2 = args->numCols2;
    int **arr1 = args->arr1;
    int **arr2 = args->arr2;
    int sum = 0;
    if (rowFrom == -1 || rowTo == -1)
    {
        return NULL;
    }
    for (int i = rowFrom; i <= rowTo; i++)
    {
        for (int j = 0; j < numCols2; j++)
        {
            sum = 0;
            for (int k = 0; k < numCols1; k++)
            {
                sum += arr1[i][k] * arr2[k][j];
            }
            (args->res)[i][j] = sum;
        }
    }
    return NULL;
}

void multiplySingle(int **mat1, int **mat2, int **res, int a, int b, int c)
{
    int i, j, k;
    for (i = 0; i < a; i++)
    {
        for (j = 0; j < c; j++)
        {
            res[i][j] = 0;
            for (k = 0; k < b; k++)
                res[i][j] += mat1[i][k] * mat2[k][j];
        }
    }
}

int main(int argc, char *argv[])
{
    printf(BLUE "Running benchmark test\n\n" RESET);
    int r1, r2, c1, c2, **res;
    int **arr1 = NULL, **arr2 = NULL;
    scanf("%d", &r1);
    scanf("%d", &c1);
    if (readarray(&arr1, r1, c1))
    {
        return errno;
    }
    scanf("%d", &r2);
    scanf("%d", &c2);
    if (c1 != r2)
    {
        printf("Incompatible arrays, multiplication not possible.\n");
        return 0;
    }
    if (readarray(&arr2, r2, c2))
    {
        return errno;
    }
    res = (int **)malloc(sizeof(int *) * r1);
    if (res == NULL)
    {
        perror("Error in allocating result array");
        return errno;
    }
    for (int i = 0; i < r1; i++)
    {
        res[i] = (int *)malloc(c2 * sizeof(int));
        if (res[i] == NULL)
        {
            perror("Error");
            return errno;
        }
    }
    if (strcmp(argv[1], "multi") == 0)
    {

        struct arg_struct args[3];
        int row_parts = r1 / 3;
        for (int i = 0; i < 3; i++)
        {
            args[i].rowFrom = -1;
            args[i].rowTo = -1;
            args[i].res = res;
            args[i].arr1 = arr1;
            args[i].arr2 = arr2;
            args[i].numCols1 = c1;
            args[i].numCols2 = c2;
        }
        args[0].rowFrom = 0;
        for (int i = 0; i < 3; i++)
        {
            args[i].rowTo = row_parts - 1;
        }
        for (int i = 0; i < r1 % 3; i++)
        {
            args[i].rowTo += 1;
        }
        for (int i = 1; i <= 2; i++)
        {
            args[i].rowFrom = args[i - 1].rowTo + 1;
            args[i].rowTo += args[i].rowFrom;
        }
        thread threads[3];
        for (int i = 0; i < 3; i++)
        {
            thread_create(&threads[i], NULL, partMatMul, &args[i]);
        }
        for (int i = 0; i < 3; i++)
        {
            thread_join(threads[i], NULL);
        }
        for(int i = 0; i < r1; i++){
            free(res[i]);
        }
        free(res);
        for (int i = 0; i < r1; i++)
        {
            free(arr1[i]);
        }
        free(arr1);
        for (int i = 0; i < r2; i++)
        {
            free(arr2[i]);
        }
        free(arr2);
    }
    else
    {
        multiplySingle(arr1, arr2, res, r1, r2, c2);
    }
    return 0;
}