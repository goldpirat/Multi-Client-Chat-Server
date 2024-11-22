#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int client_id = -1; // Default value until the server assigns an ID

// Receive messages and handle client-specific formatting
void *receive_messages(void *sock_fd) {
    int sock = *(int *)sock_fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';

        // Check if it's the client ID assignment
        if (client_id == -1 && strstr(buffer, "Your client ID is:") == buffer) {
            sscanf(buffer, "Your client ID is: %d", &client_id);
            printf("%s", buffer);
        } else {
            printf("%s", buffer); // Print the message from the server
        }
    }

    if (bytes_received == 0) {
        printf("Server disconnected.\n");
    } else {
        perror("recv");
    }

    close(sock);
    pthread_exit(NULL);
}

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;
    pthread_t recv_thread;

    // Create a socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server. Type your messages below:\n");

    // Create a thread to receive messages from the server
    pthread_create(&recv_thread, NULL, receive_messages, (void *)&sock_fd);

    // Main loop to send messages
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        char formatted_message[BUFFER_SIZE];
        snprintf(formatted_message, sizeof(formatted_message), "Me: %s", buffer);
        printf("%s", formatted_message); // Display "Me:" for the user's message
        send(sock_fd, buffer, strlen(buffer), 0);
    }

    // Clean up and exit
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);
    close(sock_fd);
    return 0;
}
