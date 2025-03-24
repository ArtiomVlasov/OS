#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

ssize_t my_write(int file_descriptor, const void* buf, size_t count){
    return syscall(SYS_write, file_descriptor, buf, count);
}

int main(int argc, char const *argv[])
{
    const char* buf = "Hello world";
    my_write(1, buf, 11);
    return 0;
}
