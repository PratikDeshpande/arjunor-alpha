#include <iostream>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "7379"
#define BACKLOG 10

// TODO: Use Modern Logging library (ie boost, google, etc)
// TODO: Configurable log levels
// TODO: Use smart pointers | RAII
// TODO: Retrieve port, backlog, etc from command line flags OR config OR DEFAULT
// TODO: Support more than one client connection
// TODO: Add unit tests for IPv6 logic

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
    char buffer[1024] = {0};


    struct sockaddr_storage their_addr; // Type used to encapsulate address info for both IPv4 and IPv6 sockets. Inter castable pointer with sockaddr*
    socklen_t sin_size;
    char incoming_connection_details[INET6_ADDRSTRLEN]; // TODO: Rename these variables to relect function
    
    // tcp server loop
    while(1) {
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

        int valread = 0;

        do {
            // Read from client
            valread = read(new_fd, buffer, 1024); // read 1024 characters in each loop
            printf("Received %s\n", buffer);

            // TODO: Fix Improper termination

        } while (valread > 0);

        // Clear buffer?
        memset(buffer, 0, 1024);
        concurrent_clients--;
        printf("client session with address %s terminated\n", incoming_connection_details);


        close(new_fd);
    }

    return 0;
}