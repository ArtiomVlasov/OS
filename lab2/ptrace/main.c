#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    int fd = open("testfile.txt", O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    const char *text = "Hello, ptrace!\n";
    if (write(fd, text, 15) == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    char buffer[16];
    if (read(fd, buffer, 15) == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    if (write(STDOUT_FILENO, buffer, 15) == -1) {
        perror("write");
        close(fd);
        return 1;
    }
    close(fd);
    return 0;
}