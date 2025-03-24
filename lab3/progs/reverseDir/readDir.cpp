#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

int getPosLastSlash(char *str)
{
    int len = strlen(str);
    if (str[len - 1] == '/')
    {
        str[len - 1] = '\0';
    }
    len = strlen(str);
    for (size_t i = len - 1; i > 0; i--)
    {
        if (str[i] == '/')
        {
            return i;
        }
    }
    return -1;
}

int getStartOfExtension(char *str)
{
    int len = strlen(str);
    for (size_t i = len; i > 0; i--)
    {
        if (str[i] == '.')
        {
            return i;
        }
    }
    return len;
}

void reverseDirName(char *str)
{

    int lastSlash = getPosLastSlash(str);
    size_t len = strlen(str);

    for (size_t i = lastSlash + 1; i < (len + lastSlash + 1) / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len + lastSlash - i];
        str[len + lastSlash - i] = temp;
    }
}

void reverseFileName(char *name)
{
    size_t len = getStartOfExtension(name);
    for (size_t i = 0; i < len / 2; i++)
    {
        char temp = name[i];
        name[i] = name[len - i - 1];
        name[len - i - 1] = temp;
    }
}

// Функция для копирования содержимого файла "задом наперед"
int reverseCopyFile(const char *src_path, const char *dest_path)
{
    struct stat src_stat;
    if (stat(src_path, &src_stat) < 0) {
        perror("stat error");
        return -1;
    }
    int src_file = open(src_path, O_RDONLY);
    if (src_file < 0)
    {
        if (chmod(src_path, S_IRUSR) < 0) {
            perror("Error opening source file");
            return -1;
        }
        src_file = open(src_path, O_RDONLY);
    }
    struct stat statistic;
    int dest_file;
    if (stat(src_path, &statistic) == 0)
    {
        printf("dir mode %o\n", statistic.st_mode);
        dest_file = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, statistic.st_mode);
    }
    else
    {
        dest_file = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if (dest_file < 0)
    {
        perror("Error creating destination file");
        close(src_file);
        return -1;
    }

    off_t file_size = lseek(src_file, 0, SEEK_END);
    if (file_size < 0)
    {
        perror("Error seeking source file");
        close(src_file);
        close(dest_file);
        return -1;
    }

    char buffer[4096];
    ssize_t bytes_read;
    while (file_size > 0)
    {
        ssize_t to_read = file_size > sizeof(buffer) ? sizeof(buffer) : file_size;
        file_size -= to_read;

        if (lseek(src_file, file_size, SEEK_SET) < 0)
        {
            perror("Error seeking source file");
            close(src_file);
            close(dest_file);
            return -1;
        }

        bytes_read = read(src_file, buffer, to_read);
        if (bytes_read < 0)
        {
            perror("Error reading source file");
            close(src_file);
            close(dest_file);
            return -1;
        }

        for (ssize_t i = 0; i < bytes_read / 2; i++)
        {
            char temp = buffer[i];
            buffer[i] = buffer[bytes_read - i - 1];
            buffer[bytes_read - i - 1] = temp;
        }

        if (write(dest_file, buffer, bytes_read) < 0)
        {
            perror("Error writing to destination file");
            close(src_file);
            close(dest_file);
            return -1;
        }
    }
    if (chmod(src_path, src_stat.st_mode) < 0) {
        perror("Error change mode");
        return -1;
    }
    close(src_file);
    close(dest_file);
    return 0;
}

void copyDirectoryReverse(const char *src_dir_path, const char *dest_dir_path)
{
    DIR *src_dir = opendir(src_dir_path);
    if (!src_dir)
    {
        perror("Error opening source directory");
        return;
    }
    // FIXME пееписать копирование прав доступа
    // FIXME переписать комирование прав доступа для фала даже если он нет права на чтение
    struct stat st;
    int status;
    if (stat(src_dir_path, &st) == 0)
    {
        // printf("dir mode %o\n", st.st_mode);
        status = mkdir(dest_dir_path, st.st_mode);
    }
    else
    {
        status = mkdir(dest_dir_path, 0755);
    }
    if (status < 0)
    {
        if (errno != EEXIST)
        {
            perror("Error creating destination directory");
            exit(EXIT_FAILURE);
        }
    }

    struct dirent *entry;
    while ((entry = readdir(src_dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        { // Только регулярные файлы
            char src_file_path[4096];

            snprintf(src_file_path, sizeof(src_file_path), "%s/%s", src_dir_path, entry->d_name);

            char reversed_name[256];
            strncpy(reversed_name, entry->d_name, sizeof(reversed_name) - 1);
            reversed_name[sizeof(reversed_name) - 1] = '\0';
            reverseFileName(reversed_name);

            char dest_file_path[4096];
            snprintf(dest_file_path, sizeof(dest_file_path), "%s/%s", dest_dir_path, reversed_name);

            if (reverseCopyFile(src_file_path, dest_file_path) < 0)
            {
                fprintf(stderr, "Failed to copy file: %s\n", src_file_path);
            }
        }
        else if (entry->d_type == DT_DIR && (entry->d_name[0] != '.' && entry->d_name[1] != '.' && entry->d_name[2] != '\0'))
        {
            char src_dir[4096];

            snprintf(src_dir, sizeof(src_dir), "%s/%s", src_dir_path, entry->d_name);

            char reversed_name[256];
            strncpy(reversed_name, entry->d_name, sizeof(reversed_name) - 1);
            reversed_name[sizeof(reversed_name) - 1] = '\0';
            reverseDirName(reversed_name);

            char dest_dir[4096];
            snprintf(dest_dir, sizeof(dest_dir), "%s/%s", dest_dir_path, reversed_name);
            copyDirectoryReverse(src_dir, dest_dir);
        }
    }

    closedir(src_dir);
}

// FIXME рекурсивное копирование директорий

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }

    char dest_dir_path[4096];
    strncpy(dest_dir_path, argv[1], sizeof(dest_dir_path) - 1);
    dest_dir_path[sizeof(dest_dir_path) - 1] = '\0';
    reverseDirName(dest_dir_path);
    copyDirectoryReverse(argv[1], dest_dir_path);

    return 0;
}
