#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROCESSES 5

pid_t pids[NUM_PROCESSES];

void handle_alarm(int sig) 
{
    for (int i = 0; i < NUM_PROCESSES; i++) 
    {
        if (kill(pids[i], 0) == 0) 
        {
            printf("Killing child process %d (PID: %d)\n", i + 1, pids[i]);
            kill(pids[i], SIGKILL);
        }
    }
}

int main() 
{
    signal(SIGALRM, handle_alarm);

    for (int i = 0; i < NUM_PROCESSES; i++) 
    {
        pids[i] = fork();

        if (pids[i] < 0) 
        {
            perror("Fork failed");
            exit(1);
        } 
        else if (pids[i] == 0) 
        {
            printf("Child process %d (PID: %d) is running...\n", i + 1, getpid());
            sleep(i*22);
            printf("Child process %d (PID: %d) completed.\n", i + 1, getpid());
            exit(0);
        }
    }

    alarm(2);

    for (int i = 0; i < NUM_PROCESSES; i++) 
    {
        printf("kek %d\n", i);
        waitpid(pids[i], NULL, 0);
    }
    printf("lol");
    return 0;
}