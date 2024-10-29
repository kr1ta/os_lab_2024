#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <signal.h>

#include "find_min_max.h"
#include "utils.h"

pid_t *child_pids; 
int pnum; 
volatile sig_atomic_t alarm_triggered = 0;

void handle_alarm(int sig) 
{
    alarm_triggered = 1; 
    for (int i = 0; i < pnum; i++) 
    {
        if (kill(child_pids[i], 0) == 0) 
        {
            kill(child_pids[i], SIGKILL);
        }
    }
}

void handle_error(const char *message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int seed = -1;
    int array_size = -1;
    int timeout = -1;
    bool with_files = false;

    while (true)
    {
        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"timeout", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            switch (option_index)
            {
            case 0:
                seed = atoi(optarg);
                if (seed < 0)
                    handle_error("Seed must be a non-negative integer.");
                break;
            case 1:
                array_size = atoi(optarg);
                if (array_size <= 0)
                    handle_error("Array size must be a positive integer.");
                break;
            case 2:
                pnum = atoi(optarg);
                if (pnum <= 0)
                    handle_error("Number of processes must be a positive integer.");
                break;
            case 3:
                timeout = atoi(optarg);
                if (timeout < 0)
                    handle_error("Timeout must be a non-negative integer.");
                break;
            }
            break;
        case 'f':
            with_files = true;
            break;
        case '?':
            handle_error("Unrecognized option. Please check your input.");
            break;
        }
    }

    if (seed == -1 || array_size == -1 || pnum == -1)
    {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n", argv[0]);
        return 1;
    }

    child_pids = malloc(pnum * sizeof(pid_t));
    if (!child_pids) handle_error("Failed to allocate memory for child PIDs.");

    int *array = malloc(sizeof(int) * array_size);
    if (!array) handle_error("Failed to allocate memory for the array.");

    GenerateArray(array, array_size, seed);
    
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int chunk_size = array_size / pnum;

    for (int i = 0; i < pnum; i++)
    {
        pid_t child_pid = fork();
        if (child_pid < 0)
        {
            printf("Fork failed!\n");
            free(child_pids); 
            free(array);
            return 1;
        }

        child_pids[i] = child_pid;

        if (child_pid == 0) 
        {
            close(fd[0]);
            int begin = i * chunk_size;
            int end = (i + 1) * chunk_size;
            if (end > array_size)
            {
                end = array_size;
            }
            
            struct MinMax min_max = GetMinMax(array, begin, end);

            if (with_files)
            {
                char fname[1000];
                sprintf(fname, "/tmp/temp_result_%d", i);

                FILE *fptr = fopen(fname, "w");
                if (!fptr)
                {
                    printf("Failed to create file!\n");
                    return 1;
                }

                fprintf(fptr, "%d_%d", min_max.min, min_max.max);
                fclose(fptr);
            }
            else
            {
                write(fd[1], &min_max, sizeof(min_max));
                close(fd[1]);
            }
            
            return 0; 
        }
    }

    close(fd[1]);

    if (timeout > 0)
    {
        signal(SIGALRM, handle_alarm);
        alarm(timeout);

        for (int i = 0; i < pnum; i++)
        {
            waitpid(child_pids[i], NULL, 0);
        }
        
        if (alarm_triggered)
        {
            printf("Exiting due to alarm.\n");
            free(array);
            free(child_pids);
            return 1; 
        }
    }
	else 
	{
		for (int i = 0; i < pnum; i++)
		{
			wait(NULL); 
		}
	}

	struct MinMax min_max;
	min_max.min = INT_MAX;
	min_max.max = INT_MIN;

	for (int i = 0; i < pnum; i++)
	{
	    struct MinMax mm;

	    if (with_files)
	    {
	        char fname[1000];
	        sprintf(fname, "/tmp/temp_result_%d", i);

	        FILE *fptr = fopen(fname, "r");
	        if (!fptr)
	        {
	            printf("Failed to read file!\n");
	            free(child_pids); 
	            free(array);
	            return 1;
	        }

	        fscanf(fptr, "%d_%d", &mm.min, &mm.max);
	        fclose(fptr);
	    }
	    else
	    {
	        read(fd[0], &mm, sizeof(mm));
	    }

	    if (mm.min < min_max.min)
	        min_max.min = mm.min;

	    if (mm.max > min_max.max)
	        min_max.max = mm.max;
	}

	struct timeval finish_time;
	gettimeofday(&finish_time, NULL);

	double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
	elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

	free(array); 
	free(child_pids); 

	printf("Min: %d\n", min_max.min);
	printf("Max: %d\n", min_max.max);
	printf("Elapsed time: %fms\n", elapsed_time);

	return EXIT_SUCCESS;
}