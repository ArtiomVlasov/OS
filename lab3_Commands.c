#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>

void create_directory(const char *path)
{
    if (mkdir(path, 0755) == 0)
    {
        printf("Directory '%s' created successfully.\n", path);
    }
    else
    {
        perror("Failed to create directory");
    }
}

void list_directory(const char *path)
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

void delete_directory(const char *path)
{
    printf("Trying to delete dir by path: %s \n", path);
    size_t path_length;
    DIR *dir;
    struct stat stat_path, stat_entry;
    struct dirent *entry = NULL;
    stat(path, &stat_path);
    if (S_ISDIR(stat_path.st_mode) == 0)
    {
        fprintf(stderr, "This path %s is not dir. \n", path);
        return;
    }
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
        char *full_path = calloc(path_length + 1 + strlen(entry->d_name) + 1, sizeof(char));
        if (full_path == NULL)
        {
            fprintf(stderr, "This path %s is NULL. \n", full_path);
            return;
        }
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);
        stat(full_path, &stat_entry);
        if (S_ISDIR(stat_entry.st_mode) != 0)
        {
            delete_directory(full_path);
            free(full_path);
            continue;
        }
        if (unlink(full_path) == 0)
        {
            printf("The file %s was removed.\n", full_path);
            free(full_path);
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
    }
    else
    {
        printf("Directory %s was deleted successfully! \n", path);
    }
    int close = closedir(dir);
    if (close == -1)
    {
        perror("Error with closing directory.");
    }

    // if (rmdir(path) == 0)
    // {
    //     printf("Directory '%s' deleted successfully.\n", path);
    // }
    // else
    // {
    //     perror("Failed to delete directory");
    // }
}

void create_file(const char *path)
{
    int fd = open(path, O_CREAT | O_WRONLY, 0644);
    if (fd >= 0)
    {
        printf("File '%s' created successfully.\n", path);
        close(fd);
    }
    else
    {
        perror("Failed to create file");
    }
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
    while (fgets(buffer, sizeof(buffer), file))
    {
        printf("%s\n", buffer);
    }
    fclose(file);
}

void delete_file(const char *path)
{
    if (unlink(path) == 0)
    {
        printf("File '%s' deleted successfully.\n", path);
    }
    else
    {
        perror("Failed to delete file");
    }
}

void create_symlink(const char *target, const char *linkpath)
{
    if (symlink(target, linkpath) == 0)
    {
        printf("Symlink '%s' -> '%s' created successfully.\n", linkpath, target);
    }
    else
    {
        perror("Failed to create symlink");
    }
}

void read_symlink(const char *linkpath)
{
    char buffer[1024];
    ssize_t len = readlink(linkpath, buffer, sizeof(buffer) - 1);
    if (len >= 0)
    {
        buffer[len] = '\0';
        printf("Symlink '%s' points to '%s'.\n", linkpath, buffer);
    }
    else
    {
        perror("Failed to read symlink");
    }
}

void read_symlink_target(const char *linkpath)
{
    char buffer[1024];
    ssize_t len = readlink(linkpath, buffer, sizeof(buffer) - 1);
    if (len >= 0)
    {
        printf("HERE\n");
        buffer[len] = '\0';
        read_file(buffer);
    }
    else
    {
        perror("Failed to read symlink target");
    }
}

void delete_symlink(const char *linkpath)
{
    if (unlink(linkpath) == 0)
    {
        printf("Symlink '%s' deleted successfully.\n", linkpath);
    }
    else
    {
        perror("Failed to delete symlink");
    }
}

void create_hardlink(const char *target, const char *linkpath)
{
    if (link(target, linkpath) == 0)
    {
        printf("Hardlink '%s' -> '%s' created successfully.\n", linkpath, target);
    }
    else
    {
        perror("Failed to create hardlink");
    }
}

void delete_hardlink(const char *linkpath)
{
    if (unlink(linkpath) == 0)
    {
        printf("Hardlink '%s' deleted successfully.\n", linkpath);
    }
    else
    {
        perror("Failed to delete Hardlink");
    }
}

void show_permissions(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
    {
        printf("Permissions of '%s': %o\n", path, st.st_mode);
        printf("Number of hard links: %ld\n", st.st_nlink);
    }
    else
    {
        perror("Failed to get file permissions");
    }
}

void change_permissions(const char *path, mode_t mode)
{
    if (chmod(path, mode) == 0)
    {
        printf("Permissions of '%s' changed to %o successfully.\n", path, mode);
    }
    else
    {
        perror("Failed to change file permissions");
    }
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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <command> [arguments]\n", argv[0]);
        print_help();
        return EXIT_FAILURE;
    }

    const char *command = argv[1];

    if (strcmp(command, "--help") == 0)
    {
        print_help();
        return EXIT_SUCCESS;
    }

    if (command[0] == 'l') // костыль говна
    {
        if (strcmp(command, "ln") == 0 && argc == 5 && strcmp(argv[2], "-s") == 0)
        {
            if (argc < 4)
            {
                perror("Usage: ln -s <target> <linkpath> \n");
                return EXIT_FAILURE;
            }
            create_symlink(argv[3], argv[4]);
        }
        else if (strcmp(command, "ln") == 0)
        {
            if (argc < 3)
            {
                perror("Usage: ln <target> <linkpath>\n");
                return EXIT_FAILURE;
            }
            create_hardlink(argv[2], argv[3]);
        }
    }

    else if (strcmp(command, "mkdir") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: madir <path>\n");
            return EXIT_FAILURE;
        }
        create_directory(argv[2]);
    }

    else if (strcmp(command, "ls") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: ls <path>\n");
            return EXIT_FAILURE;
        }
        list_directory(argv[2]);
    }

    else if (strcmp(command, "rmdir") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: rmdir <path>\n");
            return EXIT_FAILURE;
        }
        delete_directory(argv[2]);
    }
    else if (strcmp(command, "rm") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: rm <path>\n");
            return EXIT_FAILURE;
        }
        delete_file(argv[2]);
    }
    else if (strcmp(command, "readlink") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: readlink <path>\n");
            return EXIT_FAILURE;
        }
        read_symlink(argv[2]);
    }
    else if (strcmp(command, "symlink_target") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: symlink_target <path>\n");
            return EXIT_FAILURE;
        }
        read_symlink_target(argv[2]);
    }
    else if (strcmp(command, "rmHard") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: rmHard <path>\n");
            return EXIT_FAILURE;
        }
        delete_hardlink(argv[2]);
    }

    else if (strcmp(command, "touch") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: touch <path>\n");
            return EXIT_FAILURE;
        }
        create_file(argv[2]);
    }

    else if (strcmp(command, "cat") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: cat <path>\n");
            return EXIT_FAILURE;
        }
        read_file(argv[2]);
    }
    else if (strcmp(command, "chmod") == 0)
    {
        if (argc < 4)
        {
            fprintf(stderr, "Usage: chmod <path> <mode>\n");
            return EXIT_FAILURE;
        }
        change_permissions(argv[2], strtol(argv[3], NULL, 8));
    }

    else if (strcmp(command, "stat") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: stat <path>\n");
            return EXIT_FAILURE;
        }
        show_permissions(argv[2]);
    }

    else if (strcmp(command, "unlink") == 0)
    {
        if (argc < 2)
        {
            fprintf(stderr, "Usage: unlink <path>\n");
            return EXIT_FAILURE;
        }
        delete_symlink(argv[2]);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        print_help();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
