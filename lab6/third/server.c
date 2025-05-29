#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 32

int server_socket;
int client_sockets[MAX_CLIENTS] = {0};

void sigint_handler(int sig) {
    printf("\nServer shutting down...\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != 0) {
            send(client_sockets[i], "Server is shutting down.\n", 26, 0);
            close(client_sockets[i]);
        }
    }
    close(server_socket);
    exit(0);
}

void handle_client_message(int sd) {
    char buffer[BUFFER_SIZE];
    int valread = read(sd, buffer, BUFFER_SIZE);

    if (valread < 0) {
        perror("Read error");
        close(sd);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == sd) {
                client_sockets[i] = 0;
                break;
            }
        }
    } else if (valread == 0) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        getpeername(sd, (struct sockaddr*)&client_addr, &client_addr_len);
        printf("Host disconnected | ip %s | port %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        close(sd);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == sd) {
                client_sockets[i] = 0;
                break;
            }
        }
    } else {
        if (valread < BUFFER_SIZE) {
            buffer[valread] = '\0';
        }
        if (send(sd, buffer, valread, 0) != valread) {
            perror("Send error");
        }
    }
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    fd_set read_fds;
    int max_sd, new_socket, activity;
    int opt = 1;

    signal(SIGINT, sigint_handler);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket create error");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Server bind error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Listen error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("TCP-server started. Listening on port: %d\n", PORT);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        max_sd = server_socket;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &read_fds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            perror("Select error");
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            if ((new_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len)) < 0) {
                perror("Accept error");
                continue;
            }

            printf("New connection %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            int added = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    printf("Socket added to the sockets list as %d\n", i);
                    added = 1;
                    break;
                }
            }

            if (!added) {
                printf("Max clients reached. Connection from %s:%d refused.\n", inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port));
                send(new_socket, "Server is full. Try again later.\n", 33, 0);
                close(new_socket);
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &read_fds)) {
                handle_client_message(sd);
            }
        }
    }
}