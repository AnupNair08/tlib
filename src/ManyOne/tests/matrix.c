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
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "tests.h"

int **arr1 = NULL, **arr2 = NULL, **res = NULL;
int r1, r2, c1, c2;
thread_attr *a = NULL;

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

void *partMatMul(void *index)
{
    int sum = 0;
    int i = *((int *)index);
    for (int j = 0; j < c2; j++)
    {
        sum = 0;
        for (int k = 0; k < c1; k++)
        {
            sum += arr1[i][k] * arr2[k][j];
        }
        res[i][j] = sum;
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
    //check if sched params are given
    if (argc == 3)
    {
        a = (thread_attr *)malloc(sizeof(thread_attr));
        thread_attr_init(a);
        a->schedInterval.sc = atoi(argv[2]) / 1000000;
        a->schedInterval.ms = atoi(argv[2]) % 1000000;
    }
    scanf("%d", &r1);
    scanf("%d", &c1);
    if (readarray(&arr1, r1, c1))
    {
        thread_attr_destroy(a);
        return errno;
    }
    scanf("%d", &r2);
    scanf("%d", &c2);
    if (c1 != r2)
    {
        printf("Incompatible arrays, multiplication not possible.\n");
        thread_attr_destroy(a);
        return 0;
    }
    if (readarray(&arr2, r2, c2))
    {
        thread_attr_destroy(a);
        return errno;
    }
    res = (int **)malloc(sizeof(int *) * r1);
    if (res == NULL)
    {
        perror("Error in allocating result array");
        thread_attr_destroy(a);
        return errno;
    }
    for (int i = 0; i < r1; i++)
    {
        res[i] = (int *)malloc(c2 * sizeof(int));
        if (res[i] == NULL)
        {
            perror("Error");
            thread_attr_destroy(a);
            return errno;
        }
    }
    if (strcmp(argv[1], "multi") == 0)
    {
        thread threads[r1];
        int args[r1];
        for (int i = 0; i < r1; i++)
        {
            args[i] = i;
        }
        for (int i = 0; i < r1; i++)
        {
            thread_create(&threads[i], a, partMatMul, (void *)(args + i));
        }
        for (int i = 0; i < r1; i++)
        {
            thread_join(threads[i], NULL);
        }
        // printf("%d %d\n", r1, c2);
        // for (int i = 0; i < r1; i++)
        // {
        //     for (int j = 0; j < c2; j++)
        //     {
        //         printf("%d ", res[i][j]);
        //     }
        //     printf("\n");
        // }
        for (int i = 0; i < r1; i++)
        {
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
    thread_attr_destroy(a);
    return 0;
}