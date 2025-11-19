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

int reverseCopyFile(const char *src_path, const char *dest_path)
{
    struct stat src_stat;
    if (stat(src_path, &src_stat) < 0)
    {
        perror("stat error");
        return -1;
    }

    mode_t original_mode = src_stat.st_mode;

    if (chmod(src_path, original_mode | S_IWUSR | S_IRUSR) < 0)
    {
        perror("Failed to set temporary permissions on source file");
        return -1;
    }

    int src_file = open(src_path, O_RDONLY);
    if (src_file < 0)
    {
        perror("Error opening source file");
        chmod(src_path, original_mode);
        return -1;
    }

    int dest_file = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, original_mode | S_IWUSR | S_IRUSR);
    if (dest_file < 0)
    {
        perror("Error creating destination file");
        close(src_file);
        chmod(src_path, original_mode);
        return -1;
    }

    off_t file_size = lseek(src_file, 0, SEEK_END);
    if (file_size < 0)
    {
        perror("Error seeking source file");
        close(src_file);
        close(dest_file);
        chmod(src_path, original_mode);
        chmod(dest_path, original_mode);
        return -1;
    }

    int len = 4096;
    char buffer[len];
    ssize_t bytes_read;

    while (file_size > 0)
    {
        ssize_t to_read = file_size > len ? len : file_size;
        file_size -= to_read;

        if (lseek(src_file, file_size, SEEK_SET) < 0)
        {
            perror("Error seeking source file");
            close(src_file);
            close(dest_file);
            chmod(src_path, original_mode);
            chmod(dest_path, original_mode);
            return -1;
        }

        bytes_read = read(src_file, buffer, to_read);
        if (bytes_read < 0)
        {
            perror("Error reading source file");
            close(src_file);
            close(dest_file);
            chmod(src_path, original_mode);
            chmod(dest_path, original_mode);
            return -1;
        }

        for (ssize_t i = 0; i < bytes_read / 2; i++)
        {
            char temp = buffer[i];
            buffer[i] = buffer[bytes_read - i - 1];
            buffer[bytes_read - i - 1] = temp;
        }

        ssize_t total_written = 0;
        while (total_written < bytes_read)
        {
            ssize_t bytes_written = write(dest_file, buffer + total_written, bytes_read - total_written);
            if (bytes_written < 0)
            {
                perror("Error writing to destination file");
                close(src_file);
                close(dest_file);
                chmod(src_path, original_mode);
                chmod(dest_path, original_mode);
                return -1;
            }
            total_written += bytes_written;
        }
    }

    close(src_file);
    close(dest_file);

    if (chmod(src_path, original_mode) < 0)
    {
        perror("Failed to restore permissions on source file");
    }
    if (chmod(dest_path, original_mode) < 0)
    {
        perror("Failed to set permissions on destination file");
    }

    return 0;
}

void copyDirectoryReverse(const char *src_dir_path, const char *dest_dir_path)
{
    struct stat src_stat;
    if (stat(src_dir_path, &src_stat) < 0)
    {
        perror("Failed to stat source directory");
        return;
    }

    mode_t original_mode = src_stat.st_mode;

    if (chmod(src_dir_path, original_mode | S_IWUSR | S_IRUSR | S_IXUSR) < 0)
    {
        perror("Failed to set temporary permissions on source directory");
        return;
    }

    DIR *src_dir = opendir(src_dir_path);
    if (!src_dir)
    {
        perror("Failed to open source directory");
        chmod(src_dir_path, original_mode); 
        return;
    }

    if (mkdir(dest_dir_path, original_mode| S_IWUSR | S_IRUSR | S_IXUSR) < 0 && errno != EEXIST)
    {
        perror("Failed to create destination directory");
        closedir(src_dir);
        chmod(src_dir_path, original_mode); 
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(src_dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char src_path[4096], dest_path[4096];
        snprintf(src_path, sizeof(src_path), "%s/%s", src_dir_path, entry->d_name);

        char reversed_name[256];
        strncpy(reversed_name, entry->d_name, sizeof(reversed_name) - 1);
        reversed_name[sizeof(reversed_name) - 1] = '\0';
        reverseFileName(reversed_name);

        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir_path, reversed_name);

        if (entry->d_type == DT_REG)
        {
            if (reverseCopyFile(src_path, dest_path) < 0)
            {
                fprintf(stderr, "Failed to copy file: %s\n", src_path);
            }
        }
        else if (entry->d_type == DT_DIR)
        {
            copyDirectoryReverse(src_path, dest_path);
        }
    }

    closedir(src_dir);

    if (chmod(src_dir_path, original_mode) < 0)
    {
        perror("Failed to restore permissions on source directory");
    }

    if (chmod(dest_dir_path, original_mode) < 0)
    {
        perror("Failed to set permissions on destination directory");
    }
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }
    int len = 4096;
    char dest_dir_path[len];

    strncpy(dest_dir_path, argv[1], len - 1);
    dest_dir_path[len - 1] = '\0';
    reverseDirName(dest_dir_path);
    copyDirectoryReverse(argv[1], dest_dir_path);

    return 0;
}
