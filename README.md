# ExamRank06

## Description
This repository contains the solution for the "mini_serv" assignment. The assignment requires writing a server program in C that listens for clients to connect on a specified port (127.0.0.1) and enables clients to communicate with each other.

## Usage
Compile and run the program using the following commands:
```
gcc mini_serv.c -o mini_serv
./mini_serv <port_number>
```
Replace **<port_number>** with the desired port to bind.

## Functionality

- Handles client connections, assigns IDs, and allows clients to send messages.
- Error handling for missing arguments, system call errors, and memory allocation failures.
- Non-blocking behavior: Clients are not forcibly disconnected if they delay reading messages.

## Client Interaction

- Upon client connection, broadcasts "server: client %d just arrived\n" to all clients.
- Clients can send messages containing printable characters and multiple newlines.
- Received messages are broadcasted to all other clients with the format "client %d: " before each line.
- Upon client disconnection, broadcasts "server: client %d just left\n" to all clients.

## Testing
Use nc or netcat to test the server:
```
nc 127.0.0.1 <port_number>
```

## Notes
- The program avoids using #define preprocessor directives.
- Listens only on 127.0.0.1.
- Utilizes non-blocking techniques; clients are not forcibly disconnected for delayed reads.
