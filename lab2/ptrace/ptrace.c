#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Структура для хранения имени системного вызова и его номера
typedef struct {
    int syscall_num;
    char syscall_name[256];
} Syscall;

#define MAX_SYSCALLS 1024
Syscall syscall_table[MAX_SYSCALLS];
int syscall_count = 0;

// Функция для загрузки системных вызовов из файла
void load_syscalls(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    char line[512];
    int line_number = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        if (line_number < 40 || line_number > 599) {
            continue;
        }

        // Если строка начинается с #define
        if (strncmp(line, "#define", 7) == 0) {
            char name[256];
            int num;
            if (sscanf(line, "#define %s %d", name, &num) == 2) {
                syscall_table[syscall_count].syscall_num = num;
                strncpy(syscall_table[syscall_count].syscall_name, name, sizeof(syscall_table[syscall_count].syscall_name) - 1);
                syscall_count++;
                if (syscall_count >= MAX_SYSCALLS) {
                    break;
                }
            }
        }
    }
    fclose(file);
}

// Функция для поиска имени системного вызова по номеру
const char *get_syscall_name(int syscall_num) {
    for (int i = 0; i < syscall_count; i++) {
        if (syscall_table[i].syscall_num == syscall_num) {
            return syscall_table[i].syscall_name;
        }
    }
    return "UNKNOWN_SYSCALL";
}

void run_child(const char *program_name) {
    printf("Prog to run '%s'\n", program_name);
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
        perror("ptrace");
        exit(1);
    }
    int execl_err = execl(program_name, program_name, NULL);
    if (execl_err == -1) {
        perror("execl");
        exit(1);
    }
}

void run_parent(pid_t child_pid) {
    int wait_status;
    unsigned long long syscall;
    wait(&wait_status);
    while (WIFSTOPPED(wait_status)) {
        struct user_regs_struct regs;
        if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0) < 0) {
            perror("ptrace");
            exit(1);
        }
        wait(&wait_status);
        if (WIFEXITED(wait_status)) {
            break;
        }
        if (ptrace(PTRACE_GETREGS, child_pid, 0, &regs) < 0) {
            perror("ptrace");
            exit(1);
        }
        syscall = regs.orig_rax;
        const char *syscall_name = get_syscall_name(syscall);
        printf("Current syscall: %llu (%s)\n", syscall, syscall_name);
        if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0) < 0) {
            perror("ptrace");
            exit(1);
        }
        wait(&wait_status);
    }
}

int main(int argc, char *argv[]) {
    pid_t child_pid;
    if (argc < 2) {
        fprintf(stderr, "Use: ./<this program> <syscalls file> <program to run>\n");
        return 1;
    }

    // Загружаем определения системных вызовов
    load_syscalls("/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/sys/syscall.h");

    child_pid = fork();
    if (child_pid == 0) {
        run_child(argv[1]);
    } else if (child_pid > 0) {
        run_parent(child_pid);
    } else {
        perror("fork");
        return 1;
    }
    return 0;
}
