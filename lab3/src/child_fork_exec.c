#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() 
{
    pid_t pid = fork();

    if (pid < 0) 
    {
        perror("Error creating process");
        return 1;
    } 
    else if (pid == 0) 
    {
        execl("./sequential_min_max", "sequential_min_max", "1", "40000", NULL);
        perror("Error executing execl");
        return 1;
    } 
    else 
    {
        printf("Process sequential_min_max started with PID: %d\n", pid);
        wait(NULL);
    }

    return 0;
}