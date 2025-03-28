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

class SingleThreadedTcpServer {
    public:
        void start() {
            std::cout << "Starting ArjunorDB" << std::endl;
            struct sockaddr_storage their_addr;
            socklen_t sin_size;
            char incoming_connection_details[INET6_ADDRSTRLEN];
            int nfds;
            int conn_sock; // add conn sockets to close list

            while(1) {
                nfds = epoll_wait(epoll_fd, events, MAX_CLIENTS, -1);
                if (nfds == -1) {
                    throw std::runtime_error("epoll_wait failed");
                }

                for(int n = 0; n < nfds; n++) {
                    if (events[n].data.fd == server_socket_fd) {
                        sin_size = sizeof their_addr;
                        conn_sock = accept(server_socket_fd, (struct sockaddr *) &their_addr, &sin_size);
                        if (conn_sock == -1) {
                            std::cout << "accept failed" << std::endl;
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

                        // This is where the magic happens. Pretend the while(1) is the NodeJS event loop and this method is executing callbacks
                        comm::handle_client_request(events[n].data.fd, incoming_connection_details, object_store);

                    }

                }
            }

        }

        // make this private
        void initialize() {

            memset(&hints, 0, sizeof hints); // TODO: Use smart pointer to allocate and release
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;

            int socker_reuse_addr_flag=1;
            int addrinfo_return_value;
            // TODO: use port from config
            if ((addrinfo_return_value = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
                std::cout << "getaddrinfo: " << gai_strerror(addrinfo_return_value) << std::endl;
                throw std::runtime_error("getaddrinfo failed");
            }

            for(p = servinfo; p != NULL; p = p->ai_next) {  // TODO: use modern iterator
                if ((server_socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                    throw std::runtime_error("socket failed");
                }
                if (fcntl(server_socket_fd, F_SETFL, O_NONBLOCK) == -1) {
                    //perror("fcntl");
                    //exit(1);
                    throw std::runtime_error("fcntl failed");
                }
        
                if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &socker_reuse_addr_flag, sizeof(int)) == -1) {
                    //perror("setsockopt");
                    //exit(1);
                    throw std::runtime_error("setsockopt failed");
                }
        
                if (bind(server_socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
                    close(server_socket_fd);
                    //perror("server: bind");
                    //continue;
                    throw std::runtime_error("bind failed");
                }
        
                break;
            }

            if (p == NULL) {
                throw std::runtime_error("bind failed");
            }
        
            if (listen(server_socket_fd, BACKLOG) == -1) {
                throw std::runtime_error("listen failed");
            }

            epoll_fd =  epoll_create1(0);
            if (epoll_fd == -1) {
                throw std::runtime_error("epoll_create1 failed");
            }

            ev.events = EPOLLIN;
            ev.data.fd = server_socket_fd;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &ev) == -1) {
                throw std::runtime_error("epoll_ctl: server_socket_fd failed");
            }



        }

        void flush();

        void stop() {

            std::cout << "Stopping ArjunorDB" << std::endl;
            freeaddrinfo(servinfo);

            close(server_socket_fd);
            close(epoll_fd);


        }

        SingleThreadedTcpServer()
        {
            // Extract port from config or params
            port = 7379;
            object_store = std::make_shared<store::ObjectStore>();



        }

        ~SingleThreadedTcpServer()
        {
            stop();

            // close all file descriptors
        }

        

    private:
        std::shared_ptr<store::ObjectStore> object_store;
        int port;

        int server_socket_fd;
        struct addrinfo hints, *servinfo, *p;

        struct epoll_event ev, events[MAX_CLIENTS];
        int epoll_fd; 


};

// TODO: This should be a singleton available from anywhere in the program. Find best practices on how to do this
std::shared_ptr<store::ObjectStore> object_store;

// TODO: Use Modern Logging library (ie boost, google, etc)
// TODO: Configurable log levels (Bonus, make log levels dynamically configurable)
// TODO: Use smart pointers | RAII (make sure all resources [memory, file handles, sockets, etc are owned by an object and returned to os])
// TODO: Retrieve port, backlog, etc from command line flags OR config OR DEFAULT
// TODO: Add unit tests for IPv6 logic
// TODO: Make this cross platform (Windows, Linux, MacOS)
    // TODO: See if you can use the strategy pattern to abstract out the platform specific code (ie kqueue/kevent for io instead of epoll)
// TODO: Find best practices for mixing C style Errors and C++ style Exceptions
    // TODO: Use consistent error handling strategy
int main(void) {

    auto server = std::make_shared<SingleThreadedTcpServer>();
    server->initialize();
    server->start();









    return 0;
}