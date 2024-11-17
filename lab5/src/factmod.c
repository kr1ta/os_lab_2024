#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

long long result = 1; 
int MOD; 
int k; 
int threads_num;
pthread_mutex_t mutex; 

void* factorial(void* arg) 
{
    int thread_id = *((int*)arg);
    int start = thread_id * (k / threads_num) + 1; 
    int end = (thread_id + 1) * (k / threads_num); 

    if (thread_id == threads_num - 1) 
    {
        end = k; 
    }

    long long partial_result = 1;
    for (int i = start; i <= end; ++i) 
    {
        partial_result = (partial_result * i) % MOD; 
    }

    pthread_mutex_lock(&mutex);
    result = (result * partial_result) % MOD; 
    pthread_mutex_unlock(&mutex); 

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
    if (argc != 5) 
    {
        fprintf(stderr, "Usage: %s -k <number> --pnum=<threads> --mod=<modulus>\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            if (i + 1 < argc) {
                k = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: Missing value for -k\n");
                return 1;
            }
        } else if (strncmp(argv[i], "--pnum=", 7) == 0) {
            threads_num = atoi(argv[i] + 7);
        } else if (strncmp(argv[i], "--mod=", 6) == 0) {
            MOD = atoi(argv[i] + 6);
        }
    }

    if (k < 0 || threads_num <= 0 || MOD <= 0) 
    {
        fprintf(stderr, "Invalid input values. Ensure k is non-negative, pnum and mod are positive.\n");
        return 1;
    }

    pthread_t threads[threads_num];
    int args[threads_num];

    pthread_mutex_init(&mutex, NULL);

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    for (int i = 0; i < threads_num; ++i) 
    {
        args[i] = i; 
        pthread_create(&threads[i], NULL, factorial, (void*)&args[i]);
    }

    for (int i = 0; i < threads_num; ++i) 
    {
        pthread_join(threads[i], NULL);
    }

    struct timeval finish_time;
	gettimeofday(&finish_time, NULL);
    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
	elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    printf("Elapsed time: %fms\n", elapsed_time);
    printf("Factorial of %d mod %d is: %lld\n", k, MOD, result);

    pthread_mutex_destroy(&mutex);
    return 0;
}