#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
    int local_var = 1;
    printf("P_proc: local var %d and its address %p \n", local_var, &local_var);
    pid_t pid_parent = getpid();
    printf("My pid: %d \n", pid_parent);
    pid_t pid_fork = fork();
    sleep(1000);
    if (pid_fork == -1) {
        perror("Fork");
        return 1;
    } else if (pid_fork == 0) {
        printf("Child pid : %d \n", getpid());
        printf("Parent pid: %d \n", getppid());
        printf("C_proc: local var %d and its address %p \n", local_var, &local_var);
        sleep(30);
        local_var = 2;
        sleep(30);
        exit(2);
    } else {
        printf("P_proc: local var %d and its address %p \n", local_var, &local_var);
        sleep(60);
        int status;
        pid_t child = wait(&status);
        if (WIFEXITED(status)) {
            printf("Child process %d ended with %d status\n", child, WIFEXITED(status));
        } else {
            printf("Child process %d ended without ok. Status: %d \n", child, WIFEXITED(status));
        }
    }
    return 0;
}