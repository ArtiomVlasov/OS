#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <libgen.h>

#define Read_Target_Symlink_Mode 1
#define Read_Symlink_Mode 0


void create_directory(const char *path)
{
    if (mkdir(path, 0755) != 0)
    {
        perror("Failed to create directory");
        return;
    }
    printf("Directory '%s' created successfully.\n", path);
}

void list_directory(char *path)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        perror("Failed to open directory");
        return;
    }

    struct dirent *entry;
    printf("Contents of directory '%s':\n", path);
    while ((entry = readdir(dir)) != NULL)
    {
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
}

void delete_directory(char *path)
{
    printf("Trying to delete dir by path: %s \n", path);
    size_t path_length;
    DIR *dir;
    struct dirent *entry = NULL;
    
    dir = opendir(path);
    if (dir == NULL)
    {
        perror("Directory couldn't be opened.");
    }
    path_length = strlen(path);
    while ((entry = readdir(dir)) != NULL)
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }
        char full_path[path_length + 1 + strlen(entry->d_name) + 1];
        sprintf(full_path, "%s/%s", basename(path), entry->d_name);
        struct stat stat_entry;
        stat(full_path, &stat_entry);
        if (S_ISDIR(stat_entry.st_mode))
        {
            delete_directory(full_path);
            continue;
        }
        if (unlink(full_path) == 0)
        {
            printf("The file %s was removed.\n", full_path);
        }
        else
        {

            printf("Current file: %s \n", full_path);
            perror("The file was not removed;\n");
        }
    }
    int rm = rmdir(path);
    if (rm != 0)
    {
        perror("Directory wasn't deleted.\n");
        return;
    }
    printf("Directory %s was deleted successfully! \n", path);

    int close = closedir(dir);
    if (close == -1)
    {
        perror("Error with closing directory.");
        return;
    }
}

void create_file(const char *path)
{
    int fd = open(path, O_CREAT | O_WRONLY, 0644);
    if (fd < 0)
    {
        perror("Failed to create file");
        return;
    }
    printf("File '%s' created successfully.\n", path);
    close(fd);
}

void read_file(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    char buffer[1024];
    printf("Contents of file '%s':\n", path);
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        printf("%s", buffer);
    }
    if (ferror(file))
    {
        perror("Error reading file");
    }
    fclose(file);
}

void delete_file(const char *path)
{
    if (unlink(path) != 0)
    {
        perror("Failed to delete file");
        return;
    }
    printf("File '%s' deleted successfully.\n", path);
}

void create_symlink(const char *target, const char *linkpath)
{
    if (symlink(target, linkpath) != 0)
    {
        perror("Failed to create symlink");
        return;
    }
    printf("Symlink '%s' -> '%s' created successfully.\n", linkpath, target);
}

void read_symlink(const char *linkpath, int mode)
{
    struct stat link_stat;
    if(lstat(linkpath, &link_stat)<0){
        perror("Failed to get stat of link");
        return;
    }
    char buffer[link_stat.st_size + 1];
    ssize_t len = readlink(linkpath, buffer, sizeof(buffer) - 1);
    if (len < 0)
    {
        perror("Failed to read symlink");
        return;
    }
    buffer[len] = '\0';
    printf("Symlink '%s' points to '%s'.\n", linkpath, buffer);
    if(mode == 1){
        read_file(buffer);
    }
}

void delete_symlink(const char *linkpath)
{
    if (unlink(linkpath) != 0)
    {
        perror("Failed to delete symlink");
        return;
    }
    printf("Symlink '%s' deleted successfully.\n", linkpath);
}

void create_hardlink(const char *target, const char *linkpath)
{
    if (link(target, linkpath) != 0)
    {
        perror("Failed to create hardlink");
        return;
    }
    printf("Hardlink '%s' -> '%s' created successfully.\n", linkpath, target);
}

void delete_hardlink(const char *linkpath)
{
    if (unlink(linkpath) != 0)
    {
        perror("Failed to delete Hardlink");
        return;
    }
    printf("Hardlink '%s' deleted successfully.\n", linkpath);
}

void convertPermission(mode_t permission, char *convertedPermission)
{

    if (permission & S_IRUSR)
        convertedPermission[0] = 'r';
    if (permission & S_IWUSR)
        convertedPermission[1] = 'w';
    if (permission & S_IXUSR)
        convertedPermission[2] = 'x';

    if (permission & S_IRGRP)
        convertedPermission[3] = 'r';
    if (permission & S_IWGRP)
        convertedPermission[4] = 'w';
    if (permission & S_IXGRP)
        convertedPermission[5] = 'x';

    if (permission & S_IRGRP)
        convertedPermission[6] = 'r';
    if (permission & S_IWGRP)
        convertedPermission[7] = 'w';
    if (permission & S_IXGRP)
        convertedPermission[8] = 'x';
}

void show_permissions(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
    {
        perror("Failed to get file permissions");
        return;
    }
    char convertedPermission[10] = "----------";
    convertPermission(st.st_mode, convertedPermission);
    printf("Permissions of '%s': %o/%s\n", path, st.st_mode, convertedPermission);
    printf("Number of hard links: %ld\n", st.st_nlink);
}

void change_permissions(const char *path, mode_t mode)
{
    if (chmod(path, mode) != 0)
    {
        perror("Failed to change file permissions");
        return;
    }
    printf("Permissions of '%s' changed to %o successfully.\n", path, mode);
}

void print_help()
{
    printf("Available commands:\n");
    printf("  mkdir <path>                - Create a directory\n");
    printf("  ls <path>                   - List contents of a directory\n");
    printf("  rmdir <path>                - Delete a directory\n");
    printf("  touch <path>                - Create a file\n");
    printf("  cat <path>                  - Read and print a file\n");
    printf("  rm <path>                   - Delete a file\n");
    printf("  ln -s <target> <linkpath>   - Create a symbolic link\n");
    printf("  readlink <linkpath>         - Read a symbolic link\n");
    printf("  symlink_target <linkpath>   - Read the target of a symbolic link\n");
    printf("  unlink <linkpath>           - Delete a symbolic link\n");
    printf("  ln <target> <linkpath>      - Create a hard link\n");
    printf("  rmHard <linkpath>           - Delete a hard link\n");
    printf("  stat <path>                 - Show file permissions and hard link count\n");
    printf("  chmod <path> <mode>         - Change file permissions\n");
    printf("  --help                      - Show this help message\n");
}

void get_command(char *comand)
{
    int len = strlen(comand);

    int last_slash = 0;
    for (size_t i = len - 1; i > 0; i--)
    {
        if (comand[i] == '/')
        {
            last_slash = i;
            break;
        }
    }
    if (last_slash != -1)
    {
        memmove(comand, comand + last_slash + 1, len - last_slash);
    }
    comand[len - last_slash - 1] = '\0';
}

int main(int argc, char *argv[])
{
    char *command = argv[0];
    // basename(command);
    get_command(command);
    if (strcmp(command, "help") == 0)
    {
        print_help();
        return EXIT_SUCCESS;
    }
    if (argc < 1)
    {
        fprintf(stderr, "Usage: %s <command> [arguments]\n", command);
        print_help();
        return EXIT_FAILURE;
    }
    if (argc > 4)
    {
        perror("Too many args");
        return EXIT_FAILURE;
    }

    if (strcmp(command, "ln") == 0 && argc == 4 && strcmp(argv[1], "-s") == 0)
    {
        create_symlink(argv[2], argv[3]);
    }
    else if (strcmp(command, "ln") == 0)
    {
        if (argc < 2)
        {
            perror("Usage: ln <target> <linkpath>\n");
            return EXIT_FAILURE;
        }
        create_hardlink(argv[1], argv[2]);
    }

    else if (strcmp(command, "mkdir") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: madir <path>\n");
            return EXIT_FAILURE;
        }
        create_directory(argv[1]);
    }

    else if (strcmp(command, "ls") == 0)
    {
        if (argc < 2)
        {
            list_directory("./");
            return EXIT_SUCCESS;
        }
        list_directory(argv[1]);
    }

    else if (strcmp(command, "rmdir") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: rmdir <path>\n");
            return EXIT_FAILURE;
        }
        delete_directory(argv[1]);
    }
    else if (strcmp(command, "rm") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: rm <path>\n");
            return EXIT_FAILURE;
        }
        delete_file(argv[1]);
    }
    else if (strcmp(command, "readlink") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: readlink <path>\n");
            return EXIT_FAILURE;
        }
        read_symlink(argv[1], Read_Symlink_Mode);
    }
    else if (strcmp(command, "symlink_target") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: symlink_target <path>\n");
            return EXIT_FAILURE;
        }
        read_symlink(argv[1], Read_Target_Symlink_Mode);
    }
    else if (strcmp(command, "rmHard") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: rmHard <path>\n");
            return EXIT_FAILURE;
        }
        delete_hardlink(argv[1]);
    }

    else if (strcmp(command, "touch") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: touch <path>\n");
            return EXIT_FAILURE;
        }
        create_file(argv[1]);
    }

    else if (strcmp(command, "cat") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: cat <path>\n");
            return EXIT_FAILURE;
        }
        read_file(argv[1]);
    }
    else if (strcmp(command, "chmod") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "Usage: chmod <path> <mode>\n");
            return EXIT_FAILURE;
        }
        change_permissions(argv[1], strtol(argv[2], NULL, 8));
    }

    else if (strcmp(command, "stat") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: stat <path>\n");
            return EXIT_FAILURE;
        }
        show_permissions(argv[1]);
    }

    else if (strcmp(command, "unlink") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: unlink <path>\n");
            return EXIT_FAILURE;
        }
        delete_symlink(argv[1]);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        print_help();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}