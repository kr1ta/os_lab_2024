#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#include <getopt.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include "array_random.h"
#include "sum.h"

int main(int argc, char **argv) {
  
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  while (true) {
		int current_optind = optind ? optind : 1;

		static struct option options[] = { {"seed", required_argument, 0, 0},
										  {"array_size", required_argument, 0, 0},
										  {"threads_num", required_argument, 0, 0},
										  {0, 0, 0, 0} };

		int option_index = 0; 
		int c = getopt_long(argc, argv, "f", options, &option_index);

		if (c == -1) break; 
		switch (c) {
		case 0: 
			switch (option_index) {
			case 0:
				seed = atoi(optarg);
				if (seed < 0)
				{
					printf("Seed must be a non-negative integer.");
				}
				break;
			case 1:
				array_size = atoi(optarg);
				if (array_size <= 0)
				{
					printf("Array size must be a positive integer.");
				}
				break;
			case 2:
				threads_num = atoi(optarg);
				if (threads_num <= 0)
				{
					printf("Number of threads must be a positive integer.");
				}
				break;

			defalut:
				printf("Index %d is out of options\n", option_index);
			}
			break;
		case '?': 
			printf("Unrecognized option. Please check your input.");
			break;

		default:
			printf("getopt returned character code 0%o?\n", c);
		}
	}

	if (seed == 0 || array_size == 0 || threads_num == 0) {
		printf("Usage: %s --seed \"num\" --array_size \"num\" --threads_num \"num\" \n",
			argv[0]);
		return 1;
	}

  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  struct SumArgs args[threads_num];
  for (uint32_t i = 0; i < threads_num; i++) {
        args[i].array = array;
        args[i].begin = i * (array_size / threads_num);
        args[i].end = (i + 1) * (array_size / threads_num);
        
        if (i == threads_num - 1) {
            args[i].end = array_size;
        }

        if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
            printf("Error: pthread_create failed!\n");
            return 1;
        }
    }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int sum = 0;
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }

  struct timeval finish_time;
	gettimeofday(&finish_time, NULL);

	double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
	elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(threads);
  printf("Total: %d\n", total_sum);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
