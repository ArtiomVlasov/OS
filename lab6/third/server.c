#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>

#define PORT 1334
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

void initialize_client_sockets(int client_sockets[], int size) {
    for (int i = 0; i < size; i++) {
        client_sockets[i] = -1;
    }
}

int add_client(int client_sockets[], int size, int client_socket) {
    for (int i = 0; i < size; i++) {
        if (client_sockets[i] == -1) {
            client_sockets[i] = client_socket;
            return 0;
        }
    }
    return -1; 
}

void remove_client(int client_sockets[], int index) {
    close(client_sockets[index]);
    client_sockets[index] = -1;
}

int setup_fd_set(fd_set *read_fds, int server_socket, int client_sockets[], int size) {
    FD_ZERO(read_fds);
    FD_SET(server_socket, read_fds);
    int max_fd = server_socket;

    for (int i = 0; i < size; i++) {
        if (client_sockets[i] > 0) {
            FD_SET(client_sockets[i], read_fds);
            if (client_sockets[i] > max_fd) {
                max_fd = client_sockets[i];
            }
        }
    }

    return max_fd;
}

void handle_new_connection(int server_socket, int client_sockets[]) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

    if (client_socket < 0) {
        perror("Accept error");
        return;
    }

    printf("New connection %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    if (add_client(client_sockets, MAX_CLIENTS, client_socket) < 0) {
        printf("Too many connections\n");
        close(client_socket);
    }
}

void handle_client_data(int client_sockets[], int index) {
    char buffer[BUFFER_SIZE];
    int read_size = recv(client_sockets[index], buffer, BUFFER_SIZE, 0);

    if (read_size <= 0) {
        printf("Connection closed by client (fd %d)\n", client_sockets[index]);
        remove_client(client_sockets, index);
    } else {
        send(client_sockets[index], buffer, read_size, 0);
    }
}

void run_server(int server_socket) {
    int client_sockets[MAX_CLIENTS];
    fd_set read_fds;

    initialize_client_sockets(client_sockets, MAX_CLIENTS);

    while (1) {
        int max_fd = setup_fd_set(&read_fds, server_socket, client_sockets, MAX_CLIENTS);

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            handle_new_connection(server_socket, client_sockets);
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0 && FD_ISSET(client_sockets[i], &read_fds)) {
                handle_client_data(client_sockets, i);
            }
        }
    }
}

int setup_server_socket() {
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket create error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Listen error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("TCP-server started. Listening port: %d\n", PORT);

    return server_socket;
}

int main() {
    int server_socket = setup_server_socket();
    run_server(server_socket);
    close(server_socket);
    return 0;
}
