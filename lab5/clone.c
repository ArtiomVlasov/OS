#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

#define SIZE_STACK (5*1024)
#define RECURSION 10
#define FILENAME "file.txt"

void *allocator() {
    int fd = open(FILENAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    if (ftruncate(fd, SIZE_STACK) == -1) {
        perror("ftruncate");
        close(fd);
        exit(EXIT_FAILURE);
    }

    void *memory = mmap(NULL, SIZE_STACK, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
    return memory;
}

void recursion(int iter) {
    char str[] = "hello world";
    printf("next iter\n");
    sleep(15);

    if (iter > 0) {
        recursion(iter - 1);
    }
}

int child_enter() {
    printf("%p\n", (void*)recursion);
    printf("PID = %d, PPID = %d\n", getpid(), getppid());
    recursion(10);
    return 0;
}

int main() {
    void *memory = allocator();
    char *child_stack = (char *) memory + SIZE_STACK - 1;
    if (child_stack == (char *) -1) {
        perror("mmap");
        munmap(memory, SIZE_STACK);

        exit(EXIT_FAILURE);
    }

    pid_t pid = clone(child_enter, child_stack, SIGCHLD, NULL);
    if (pid == -1) {
        perror("clone");
        munmap(memory, SIZE_STACK);
        exit(EXIT_FAILURE);
    }

    if (waitpid(pid, NULL, 0) == -1) {
        perror("waitpid");
        munmap(memory, SIZE_STACK);
        exit(EXIT_FAILURE);
    }

    if (msync(memory, SIZE_STACK, MS_SYNC) == -1) {
        perror("msync");
        munmap(memory, SIZE_STACK);
        exit(EXIT_FAILURE);
    }

    if (munmap(memory, SIZE_STACK) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    printf("Parent process finished\n");
    return 0;
}