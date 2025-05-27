#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        pid_t pidn = fork();

        if (pidn > 0) {
            printf("Child pid: %d\n", getpid());
            exit(0);
        } else if (pidn == 0) {
            printf("Grandchild pid: %d\n", getpid());
            sleep(5);
            printf("Grandchild parent after die pid: %d\n", getppid());
            sleep(5);
            exit(0);
        } else {
            perror("Failed to create grandchild");
            exit(1);
        }
    } else if (pid > 0) {
        printf("Parent pid: %d\n", getpid());
        sleep(40);
        exit(0);
    } else {
        perror("Failed to create child");
        exit(1);
    }

    return 0;
}