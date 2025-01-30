#include <netdb.h> // struct addrinfo, getaddrinfo, gai_strerror, freeaddrinfo
#include <unistd.h>
#include <sys/socket.h> // socket, setsockopt, bind, accept, sockaddr_storage, socklen_t, address families constants
#include <string.h> // memset
#include <arpa/inet.h>  // inet_ntop
#include <sys/types.h>
#include <netinet/in.h> // INET6_ADDRSTRLEN
#include <stdio.h>  // fprintf, perror
#include <stdlib.h> // exit
#include <cstddef> // std::byte
#include <iostream>
#include "resp.h" // needs to be added to VSCode include path to be part of intellisense. Also needs to be defined as a target in Makefile
#include <vector>

#define PORT "7379"
#define BACKLOG 10


// Read from client. This is a blocking system call
std::vector<std::byte> read_command(int new_fd) {
    std::byte buffer[512]; // TODO: If bigger than 512 bytes, loop until you read EOF. Clear memory?
    auto bytes_read = recv(new_fd, buffer, 512, 0);
    printf("bytes received: %li\n", bytes_read);
    if (bytes_read <= 0) {
        std::cout << "Error reading incoming bytes" << std::endl;
        throw std::invalid_argument("Error: No data in buffer");
    }
    std::vector<std::byte> command(buffer, buffer + bytes_read);
    memset(&buffer, 0, sizeof buffer);
    return command;
    // TODO: when the function scope ends, what happens to the buffer? Does it get deallocated?
}

void send_command(int new_fd, std::vector<std::byte> command) {
    auto bytes_sent = send(new_fd, command.data(), command.size(), 0);
    printf("bytes sent: %li\n", bytes_sent);
    if (bytes_sent <= 0) {
        std::cout << "Error sending bytes" << std::endl;
        throw std::invalid_argument("Error sending bytes");
    }
}

// TODO: Use Modern Logging library (ie boost, google, etc)
// TODO: Configurable log levels
// TODO: Use smart pointers | RAII (make sure all resources [memory, file handles, sockets, etc are owned by an object and returned to os])
// TODO: Retrieve port, backlog, etc from command line flags OR config OR DEFAULT
// TODO: Support more than one client connection
// TODO: Add unit tests for IPv6 logic
// TODO: Make this cross platform (Windows)
// TODO: Find best practices for mixing C style Errors and C++ style Exceptions
// TODO: Create class for synchronous TCP Server
int main(void) {
    printf("Starting ArjunorDB\n");
    int concurrent_clients = 0;

    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof hints); // TODO: Use smart pointer to allocate and release
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int socker_reuse_addr_flag=1;
    int addrinfo_return_value;
    if ((addrinfo_return_value = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addrinfo_return_value));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {  // TODO: use modern iterator
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &socker_reuse_addr_flag, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    struct sockaddr_storage their_addr; // Type used to encapsulate address info for both IPv4 and IPv6 sockets. Inter castable pointer with sockaddr*
    socklen_t sin_size;
    char incoming_connection_details[INET6_ADDRSTRLEN]; // TODO: Rename these variables to relect function
    // tcp server loop
    while(1) {
        printf("loop start. about to accept connections...\n");
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        printf("accepted connection...\n");

        concurrent_clients++;

        //Print out the detail of the accepted connection
        // src needs struct struct in_addr for IPv4 addresses and struct in6_addr for IPv6 format
        // dest char buffer must be at least INET6_ADDRSTRLEN size parameter needs to be at least length 
        void * their_addr_inet_addr = (struct sockaddr *)&their_addr;
        if (their_addr.ss_family == AF_INET) {
            their_addr_inet_addr = &(((struct sockaddr_in *)&their_addr)->sin_addr);
        } else {
            their_addr_inet_addr = &(((struct sockaddr_in6 *)&their_addr)->sin6_addr); // TODO: add unit test for IPv6 addresses
        }

        inet_ntop(their_addr.ss_family, their_addr_inet_addr, incoming_connection_details, sizeof incoming_connection_details);
        printf("incoming IP connection details: %s\n", incoming_connection_details);
        printf("number of clients is now %i\n", concurrent_clients);

        // for loop for client session
        while(1) {
            std::vector<std::byte> command;
            try {
                command = read_command(new_fd);
            } catch (std::invalid_argument& e) {
                printf("Error reading command: %s\n", e.what());
                close(new_fd);
                concurrent_clients--;
                printf("client session with address %s disconnected\n", incoming_connection_details);
                break; // TODO: only break if its EOF character read
            }
            std::string command_string(reinterpret_cast<const char*>(command.data()), command.size());
            std::cout << "Incoming command: " << command_string << std::endl;

            try {
                send_command(new_fd, command);
            } catch (std::invalid_argument& e) {
                printf("Error sending command: %s\n", e.what());
            }
        };
    }

    return 0;
}