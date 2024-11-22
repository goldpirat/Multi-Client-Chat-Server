#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

int clients[MAX_CLIENTS];
int client_count = 0;
int next_client_id = 1;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Broadcast message to all clients except the sender
void broadcast_message(const char *message, int sender_fd, int sender_id) {
    char formatted_message[BUFFER_SIZE];
    snprintf(formatted_message, sizeof(formatted_message), "Client %d: %s", sender_id, message);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender_fd) {
            send(clients[i], formatted_message, strlen(formatted_message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Handle communication with a single client
void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    int client_id;

    // Assign a unique ID to the client
    pthread_mutex_lock(&clients_mutex);
    client_id = next_client_id++;
    pthread_mutex_unlock(&clients_mutex);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Notify the client of its ID
    snprintf(buffer, sizeof(buffer), "Your client ID is: %d\n", client_id);
    send(client_fd, buffer, strlen(buffer), 0);

    // Handle incoming messages
    while ((bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        printf("Client %d: %s", client_id, buffer);
        broadcast_message(buffer, client_fd, client_id);
    }

    // Handle client disconnection
    close(client_fd);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_fd) {
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("Client %d disconnected.\n", client_id);
    free(arg);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    // Create a socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listening failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d...\n", PORT);

    // Accept and handle clients
    while (1) {
        client_len = sizeof(client_addr);
        new_client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (new_client_fd < 0) {
            perror("Connection accept failed");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count++] = new_client_fd;

            pthread_t client_thread;
            int *new_sock = malloc(sizeof(int));
            *new_sock = new_client_fd;
            pthread_create(&client_thread, NULL, handle_client, new_sock);
        } else {
            printf("Max clients reached. Connection refused.\n");
            close(new_client_fd);
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_fd);
    return 0;
}
