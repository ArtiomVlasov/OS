#include <stdio.h>
#include <unistd.h>

int main() {
    printf("My pid%d \n", getpid());
    sleep(1);
    execvp("./a", NULL);
    printf("Hello world \n");
    return 0;
}