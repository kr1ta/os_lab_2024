#include "sum.h"

#include <stdio.h>
#include <stdlib.h>

int Sum(const struct SumArgs *args) {
    int sum = 0;
    for (int i = args->begin; i < args->end; i++) {
        sum += args->array[i]; // Добавляем текущий элемент к сумме
    }
    return sum; 
}

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  int sum = Sum(sum_args);
  // printf("Thread %lu summing from %d to %d, sum:%d\n", pthread_self(), sum_args->begin, sum_args->end, sum);
  return (void *)(size_t)sum;
}
