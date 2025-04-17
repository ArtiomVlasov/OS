#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <limits.h>

#define PAGE_SIZE sysconf(_SC_PAGESIZE)

void handle_sigsegv() {
    write(0, "Received SIGSEGV signal.", 24);
    exit(1);
}
int j = 1;
void overloading_stack() {
    int array[4096];
    printf("%d: Array pointer : %p \n",j++ , (void *) &array);
    sleep(1);
    overloading_stack();
}

void heap_loading() {
    int *array[200];
    for(int i = 0; i< 200; i++) {
        array[i] = malloc(4096*(i+1));
        printf("%d: Array pointer : %p \n",i, (void *) &array[i]);
        // sleep(1);
    }
    printf("\n--------------- Unmapping ---------------\n");
    sleep(10);
    for (int i = 0; i < 200; i++)
    {
        free(array[i]);
    }
    sleep(20);
}

void create_changePermition_tryToReadWrite() {
    size_t region_size = 10 * PAGE_SIZE;
    void *region_addr = mmap(NULL, region_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region_addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    printf("Mapped a region at address %p with size %zu bytes\n", region_addr, region_size);
    // sleep(60);
    printf("Change mprotect. \n");
    if (mprotect(region_addr, region_size, PROT_WRITE) == -1) {
        perror("mprotect");
        exit(1);
    }
    signal(SIGSEGV, handle_sigsegv);
    printf("Trying to read from region after mprotect(PROT_WRITE):\n");
    printf("%s\n", (char *) region_addr);

    if (mprotect(region_addr, region_size, PROT_READ) == -1) {
        perror("mprotect");
        exit(1);
    }
    char *message = "hello world";
    printf("Trying to write to region after mprotect(PROT_READ):\n");
    sprintf(region_addr, "%s", message);
    // sigsegv here
    if (munmap(region_addr, region_size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

void unmap_4_6_pages() {
    size_t region_size = 10 * PAGE_SIZE;
    printf("Create new region address. \n");
    void *region_addr = mmap(NULL, region_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region_addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    printf("Mapped a region at address %p with size %zu bytes. \n", region_addr, region_size);
    sleep(1);

    size_t partial_size = 3 * PAGE_SIZE;
    void *partial_addr = region_addr + 3 * PAGE_SIZE;
    printf("Changing permission");
    // printf("Unmapping a part of the region at addr %p with size %zu bytes. \n", partial_addr, partial_size);
    // if (munmap(partial_addr, partial_size) == -1) {
    //     perror("munmap");
    //     exit(1);
    // }
    sleep(10);
}

int main() {
    printf("My pid: %d \n", getpid());
    sleep(10);
    // overloading_stack();
    // heap_loading();
    // create_changePermission_tryToReadWrite();
    unmap_4_6_pages();
    return 0;
}