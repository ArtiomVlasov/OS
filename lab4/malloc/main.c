#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#define HEAP (1024 * 1024)
void *heap_start = NULL;

typedef struct block {
    size_t size;
    struct block *next;
    char free;
} block;
block *memory_blocks;

void *my_malloc(size_t size) {
    block *current = memory_blocks;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            if (current->size > size + sizeof(block)) {
                block *new_memory = (block *) ((char *) current + sizeof(block) + size);
                new_memory->size = current->size - size - sizeof(block);
                new_memory->free = 1;
                new_memory->next = current->next;

                current->size = size;
                current->free = 0;
                current->next = new_memory;
            } else {
                current->free = 0;
            }
            return (char *) current + sizeof(block);
        }
        current = current->next;
    }
    return NULL;
}

void merge_blocks() {
    block *current = memory_blocks;
    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            current->size += current->next->size + sizeof(block);
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void my_free(void *memory) {
    if (memory == NULL) {
        return;
    }
    block *current = (block *) ((char *) memory - sizeof(block));
    current->free = 1;
    merge_blocks();
}

void initialize() {
    heap_start = mmap(NULL, HEAP, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (heap_start == MAP_FAILED) {
        perror("init mmap");
        exit(1);
    }
    memory_blocks = (block *) heap_start;
    memory_blocks->size = HEAP - sizeof(block);
    memory_blocks->free = 1;
    memory_blocks->next = NULL;
}

int main() {
    initialize();
    char *ptr1 = (char *) my_malloc(100);
    if (ptr1) {
        strcpy(ptr1, "Hello, world!");
        fprintf(stdout, "ptr1: %s %p\n", ptr1, ptr1);
    }

    char *ptr2 = (char *) my_malloc(200);
    if (ptr2) {
        strcpy(ptr2, "Bye, my pretty nice world");
        fprintf(stdout, "ptr2: %s %p\n", ptr2, ptr2);
    }
    char *ptr3 = (char *) my_malloc(HEAP);
    fprintf(stdout, "ptr3: %s %p \n", ptr3, ptr3);
    my_free(ptr1);
    char *ptr4 = my_malloc(70);
    char *ptr5 = my_malloc(60);
    if (ptr4) {
        strcpy(ptr4, "Bye, my pretty nice world");
        fprintf(stdout, "ptr4: %s %p\n", ptr4, ptr4);
    }
    if (ptr5) {
        strcpy(ptr5, "Bye, my pretty nice world");
        fprintf(stdout, "ptr5: %s %p\n", ptr5, ptr5);
    }
    my_free(ptr2);

    munmap(heap_start, HEAP);
    exit(0);
}