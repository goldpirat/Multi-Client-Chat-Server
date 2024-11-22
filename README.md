# Chat Server and Client

This project implements a basic chat application using **C**. It consists of two parts:

1. **Server**: A multi-client server that handles incoming connections, assigns each client a unique ID, and broadcasts messages to all connected clients.
2. **Client**: A program that connects to the server, sends messages, and receives messages from the server. Each client is identified by a unique client number.

## Features

- Clients are assigned a unique ID upon connection.
- Clients can send and receive messages to/from all other connected clients.
- Messages from a client are prefixed with `Client <ID>:` for others, while the sender's messages are shown with `Me:`.
- Handles multiple clients concurrently using **threads**.

## Requirements

- C Compiler (e.g., `gcc`)
- pthread library for threading support

## Installation

1. **Clone the repository** (or just download the files):

   git clone <repository-url>

2. **Compile the server**:

   gcc -o chat_server chat_server.c -pthread
   
3. **Compile the client**:

  gcc -o chat_client chat_client.c -pthread

4. **Running the Application**:
  Starting the Server:
  Run the server to listen for incoming client connections:

    ./chat_server
  Starting the Clients:
  Run one or more instances of the client program to connect to the server:
    ./chat_client
5. **Interaction**:
  Once connected, the server will assign a unique ID to the client.
  Clients can send messages by typing them into the terminal. Each message will be displayed with either Me: or Client <ID>:.


6. **Example Output**:
    Server:
    Server is listening on port 8080...
    Client 1: Hello from client 1!
    Client 2: Hello from client 2!


    Client:
    Connected to the server. Type your messages below:
    Me: Hello, everyone!
    Client 1: Hello, everyone!
