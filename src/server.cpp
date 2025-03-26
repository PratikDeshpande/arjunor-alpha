#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include "comm.h"

#define PORT "7379"
#define BACKLOG 10
#define MAX_CLIENTS 20000

// TODO: This should be a singleton available from anywhere in the program. Find best practices on how to do this
std::shared_ptr<store::ObjectStore> object_store;

// TODO: Use Modern Logging library (ie boost, google, etc)
// TODO: Configurable log levels (Bonus, make log levels dynamically configurable)
// TODO: Use smart pointers | RAII (make sure all resources [memory, file handles, sockets, etc are owned by an object and returned to os])
// TODO: Retrieve port, backlog, etc from command line flags OR config OR DEFAULT
// TODO: Support more than one client connection
// TODO: Add unit tests for IPv6 logic
// TODO: Make this cross platform (Windows, Linux, MacOS)
    // TODO: See if you can use the strategy pattern to abstract out the platform specific code (ie kqueue/kevent for io instead of epoll)
// TODO: Find best practices for mixing C style Errors and C++ style Exceptions
    // TODO: Use consistent error handling strategy
// TODO: Create class for synchronous TCP Server
int main(void) {
    printf("Starting ArjunorDB\n");

    object_store = std::make_shared<store::ObjectStore>();

    int server_socket_fd;
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
        if ((server_socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (fcntl(server_socket_fd, F_SETFL, O_NONBLOCK) == -1) {
            perror("fcntl");
            exit(1);
        }

        if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &socker_reuse_addr_flag, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(server_socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_socket_fd);
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

    if (listen(server_socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
   
    struct epoll_event ev, events[MAX_CLIENTS];
    int epoll_fd =  epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }

    ev.events = EPOLLIN;
    ev.data.fd = server_socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &ev) == -1) {
        perror("epoll_ctl: server_socket_fd");
        exit(1);
    }


    struct sockaddr_storage their_addr; // Type used to encapsulate address info for both IPv4 and IPv6 sockets. Inter castable pointer with sockaddr*
    socklen_t sin_size;
    char incoming_connection_details[INET6_ADDRSTRLEN]; // TODO: Rename these variables to relect function
    int nfds;
    int conn_sock;
    while(1) {
        nfds = epoll_wait(epoll_fd, events, MAX_CLIENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(1);
        }

        for(int n = 0; n < nfds; n++) {
            if (events[n].data.fd == server_socket_fd) {
                sin_size = sizeof their_addr;
                conn_sock = accept(server_socket_fd, (struct sockaddr *) &their_addr, &sin_size);
                if (conn_sock == -1) {
                    perror("accept");
                    continue;
                }
                int fcntl_return_value = fcntl(conn_sock, F_SETFL, O_NONBLOCK);
                if (fcntl_return_value == -1) {
                    perror("fcntl");
                    exit(1);
                }
                ev.events = EPOLLIN | EPOLLET; // TODO: Look up what Edge Triggered means
                ev.data.fd = conn_sock;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit(1);
                }              
            } else {
                sin_size = sizeof their_addr;

                void * their_addr_inet_addr = (struct sockaddr *)&their_addr;
                if (their_addr.ss_family == AF_INET) {
                    their_addr_inet_addr = &(((struct sockaddr_in *)&their_addr)->sin_addr);
                } else {
                    their_addr_inet_addr = &(((struct sockaddr_in6 *)&their_addr)->sin6_addr); // TODO: add unit test for IPv6 addresses
                }

                inet_ntop(their_addr.ss_family, their_addr_inet_addr, incoming_connection_details, sizeof incoming_connection_details);
                //printf("incoming IP connection details: %s\n", incoming_connection_details);

                comm::handle_client_request(events[n].data.fd, incoming_connection_details, object_store);

            }

        }
    }

    return 0;
}