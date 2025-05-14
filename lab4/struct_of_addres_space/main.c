#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>

int global_int = 10;
int global_var;
const int global_const = 2;

void print_vars() {
    printf("Local variables in function: \n");
    int local_a;
    long long local_b;
    static int static_a;
    static long long static_b;
    const int const_a;
    const long long const_b;
    printf("int 'a' address: %p \n", (void *) &local_a);
    printf("long long 'b' address: %p \n", (void *) &local_b);

    printf("\n\n ______________________________________________\nStatic variables in function: \n");
    printf("int 'a' address: %p \n", (void *) &static_a);
    printf("long long 'b' address: %p \n", (void *) &static_b);

    printf("\n\n ______________________________________________\nConst variables in function: \n");
    printf("int 'a' address: %p \n", (void *) &const_a);
    printf("long long 'b' address: %p \n", (void *) &const_b);

    printf("\n\n ______________________________________________\nGlobal variables in function: \n");
    printf("int 'global_int' address: %p \n", (void *) &global_int);
    printf("int 'global_var' address: %p \n", (void *) &global_var);
    printf("int 'global_const' address: %p \n\n", (void *) &global_const);

}

int *variable_address() {
    int a = 10;
    printf("Variable address in func: %p \n", (void *) &a);
    return &a;
}

void buffer_address() {
    char *buffer1 = malloc(100);
    assert(buffer1 != NULL);
    strcpy(buffer1, "hello world");
    printf("Contents of buffer1 before free: %s \n", buffer1);
    free(buffer1);
    printf("Contents of buffer1 after free: %s \n", buffer1);

    char *buffer2 = malloc(100);
    strcpy(buffer2, "Goodby world");
    printf("Contents of buffer2 before free: %s \n", buffer2);

    char *middle_buffer2 = (buffer2 + 50);

    // printf("Content in the middle of the second buffer: %s \n", middle_buffer2);
    free(middle_buffer2);
    printf("Content from the start of the second buffer after free: %s \n", buffer2);
    free(buffer2);
}

int env_varible() {
    printf("Environment variable value: %s  %p\n", getenv("TEST_ENV_VAR"), getenv("TEST_ENV_VAR"));

    setenv("TEST_ENV_VAR", "set value in environment value", 1);
    

    printf("New environment variable value: %s  %p\n", getenv("TEST_ENV_VAR"), getenv("TEST_ENV_VAR"));
}

int main() {
    pid_t pid = getpid();
    printf("PID of self  process : %d \n\n", pid);
    // print_vars();


// //    void *someRegion = mmap(NULL, 1, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
//    sleep(3000);

//    printf("Variable address in main: %p \n\n", (void *) variable_address());
//    sleep(30);
    // buffer_address();
    // sleep(30);
    env_varible();
    sleep(30000);
    return 0;
}