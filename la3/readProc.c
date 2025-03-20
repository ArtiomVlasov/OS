#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define PAGEMAP_ENTRY 8
#define GET_BIT(X, Y) ((X & ((uint64_t)1 << Y)) >> Y)

void read_pagemap(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        perror("open");
        exit(1);
    }

    unsigned long virt_addr = 0;
    uint64_t read_val;
    unsigned char c_buf[PAGEMAP_ENTRY];

    while (fread(c_buf, 1, PAGEMAP_ENTRY, f) == PAGEMAP_ENTRY) {
        read_val = 0;
        for (int i = PAGEMAP_ENTRY - 1; i >= 0; --i) {
            read_val = (read_val << 8) | c_buf[i];
        }

        printf("0x%lx 0x%llx | ", virt_addr, (unsigned long long) read_val);

        if (GET_BIT(read_val, 63)) {
            printf("Page present\n");
        } else if (GET_BIT(read_val, 62)) {
            printf("Page swapped\n");
        } else {
            printf("Page not present\n");
        }

        virt_addr += getpagesize();
    }
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PID\n", argv[0]);
        exit(1);
    }
    char path_buf[64];
    if (strcmp(argv[1], "self") == 0) {
        snprintf(path_buf, sizeof(path_buf), "/proc/self/pagemap");
    } else {
        int pid = strtol(argv[1], NULL, 10);
        if (pid <= 0) {
            fprintf(stderr, "Check your pid, it's must be more than zero or 'self' \n");
            exit(1);
        }
        snprintf(path_buf, sizeof(path_buf), "/proc/%d/pagemap", pid);
    }
    read_pagemap(path_buf);
    exit(0);
}
