#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

void handle_error(const char* message) {
	fprintf(stderr, "Error: %s\n", message);
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int seed = -1;
	int array_size = -1;
	int pnum = -1; // кол-во процессов
	bool with_files = false;

	while (true) {
		int current_optind = optind ? optind : 1; // индекс следующего аргумента для обработки

		static struct option options[] = { {"seed", required_argument, 0, 0},
										  {"array_size", required_argument, 0, 0},
										  {"pnum", required_argument, 0, 0},
										  {"by_files", no_argument, 0, 'f'},
										  {0, 0, 0, 0} };

		int option_index = 0; // индекс текущей длинной опции
		int c = getopt_long(argc, argv, "f", options, &option_index); // Обрабатывает короткие и длинные параметры

		if (c == -1) break; // если опции закончились - выход

		switch (c) {
		case 0: // если найдена длинная опция
			switch (option_index) {
			case 0:
				seed = atoi(optarg);
				if (seed < 0)
				{
					handle_error("Seed must be a non-negative integer.");
				}
				break;
			case 1:
				array_size = atoi(optarg);
				if (array_size <= 0)
				{
					handle_error("Array size must be a positive integer.");
				}
				break;
			case 2:
				pnum = atoi(optarg);
				if (pnum <= 0)
				{
					handle_error("Number of processes must be a positive integer.");
				}
				break;
			case 3:
				with_files = true;
				break;

			defalut:
				printf("Index %d is out of options\n", option_index);
			}
			break;
		case 'f':
			with_files = true;
			break;

		case '?': // нераспознанная опция
			handle_error("Unrecognized option. Please check your input.");
			break;

		default:
			printf("getopt returned character code 0%o?\n", c);
		}
	}

	if (optind < argc) {
		printf("Has at least one no option argument\n");
		return 1;
	}

	if (seed == -1 || array_size == -1 || pnum == -1) {
		printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
			argv[0]);
		return 1;
	}

	int* array = malloc(sizeof(int) * array_size);
	GenerateArray(array, array_size, seed);
	int active_child_processes = 0;

	struct timeval start_time;
	gettimeofday(&start_time, NULL);

	// pipes
	int fd[2];
	pipe(fd); 

	// fd[0] - read
	// fd[1] - write

	// Каждый дочерний процесс будет записывать min/max в пайп, 
	// а родительский процесс будет их считывать.

	// files
	char* fname_tmpl = "/tmp/temp_result_%d";

	int chunk_size = array_size / pnum;

	for (int i = 0; i < pnum; i++) {
		pid_t child_pid = fork();
		if (child_pid < 0) {
			printf("Fork failed!\n");
			return 1;
		}
		active_child_processes += 1;

		// код дочернего процесса
		if (child_pid == 0) {
			int begin = i * chunk_size;
			int end = (i + 1) * chunk_size;
			if (end > array_size) { end = array_size; }
			struct MinMax min_max = GetMinMax(array, begin, end);

			if (with_files) {
				char fname[1000];
				sprintf(fname, fname_tmpl, i); // fname = temp_result_0

				FILE* fptr = fopen(fname, "w"); // file pointer -> create and write
				if (fptr == NULL) {
					printf("failed to create file!\n");
					return 1;
				}

				fprintf(fptr, "%d_%d", min_max.min, min_max.max);
				fclose(fptr);
			}
			else {
				write(fd[1], &min_max, sizeof(&min_max)); // pipe
			}
			return 0;
		}

	}

	while (active_child_processes > 0) {
		wait(NULL);
		active_child_processes -= 1;
	}
	close(fd[1]);

	struct MinMax min_max;
	min_max.min = INT_MAX;
	min_max.max = INT_MIN;

	for (int i = 0; i < pnum; i++) { // collect data from all processes
		struct MinMax mm;

		if (with_files) {
			char fname[1000];
			sprintf(fname, fname_tmpl, i);

			FILE* fptr = fopen(fname, "r");
			if (fptr == NULL) {
				printf("failed to read file!\n");
				return 1;
			}

			fscanf(fptr, "%d_%d", &mm.min, &mm.max);
			fclose(fptr);
		}
		else {
			read(fd[0], &mm, sizeof(&mm));
		}

		if (mm.min < min_max.min) min_max.min = mm.min;
		if (mm.max > min_max.max) min_max.max = mm.max;
	}
	close(fd[0]);

	struct timeval finish_time;
	gettimeofday(&finish_time, NULL);

	double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
	elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

	free(array);

	printf("Min: %d\n", min_max.min);
	printf("Max: %d\n", min_max.max);
	printf("Elapsed time: %fms\n", elapsed_time);
	fflush(NULL);
	return 0;
}