#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Ошибка привязки сокета");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP-сервер запущен на порту %d\n", SERVER_PORT);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (recv_len < 0) {
            perror("Ошибка при приеме данных");
            continue;
        }
        printf("recev_len: %d \n", recv_len);
        printf("Получено сообщение: %s\n", buffer);

        ssize_t sent_len = sendto(sockfd, buffer, recv_len, 0, (struct sockaddr *)&client_addr, addr_len);
        if (sent_len < 0) {
            perror("Ошибка отправки данных");
        }
    }

    close(sockfd);
    return 0;
}
