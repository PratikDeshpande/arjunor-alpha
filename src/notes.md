structs used for TCP server socket operations


```
struct addrinfo
{
  int ai_flags;			/* Input flags.  */
  int ai_family;		/* Protocol family for socket.  */
  int ai_socktype;		/* Socket type.  */
  int ai_protocol;		/* Protocol for socket.  */
  socklen_t ai_addrlen;		/* Length of socket address.  */
  struct sockaddr *ai_addr;	/* Socket address for socket.  */
  char *ai_canonname;		/* Canonical name for service location.  */
  struct addrinfo *ai_next;	/* Pointer to next in list.  */
};
```

* Used for host name lookups, service name lookups, etc
* Used as a parameter/criteria specification field for `getaddrinfo()` (`hints` argument)
* Value returned by getaddrinfo contains a linked list (ai_next) of all the socket addresses ()

ie

```
hints.ai_family = AF_UNSPEC; // AF_INET is IPv4, AF_INET6 is IPv6, AF_UNSPEC means IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM; // SOCK_STREAM or SOCK_DGRAM
hints.ai_flags = AI_PASSIVE;
```

Source:
https://man7.org/linux/man-pages/man3/getaddrinfo.3.html


https://beej.us/guide/bgnet/html/split/ip-addresses-structs-and-data-munging.html#structs


######

```
struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
}; 

```
* Holds the socket address information for many types of sockets
* Parellel structs created: `sockaddr_in` and `sockaddr_in6`



```
// (IPv4 only--see struct sockaddr_in6 for IPv6)

struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};

// (IPv4 only--see struct sockaddr_in6 for IPv6)

struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};

// (IPv6 only--see struct sockaddr_in and struct in_addr for IPv4)

struct sockaddr_in6 {
    u_int16_t       sin6_family;   // address family, AF_INET6
    u_int16_t       sin6_port;     // port number, Network Byte Order
    u_int32_t       sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr;     // IPv6 address
    u_int32_t       sin6_scope_id; // Scope ID
};

struct in6_addr {
    unsigned char   s6_addr[16];   // IPv6 address
};
```

* `struct sockaddr_in` used with IPv4
* IMPORTANT: pointer for sockaddr_in can be cast to a pointer for sockaddr and vice versa
* In cases when you don't know if you need to full `struct sockaddr` with IPv4 or IPv6 addresses, use `struct sockaddr_storage`



```
struct sockaddr_storage
  {
    __SOCKADDR_COMMON (ss_);	/* Address family, etc.  */
  };

```

* Designed to be large enough to hold both IPv4 and IPv6 structures.
* When filling out `struct sockaddr`, cast sockaddr_stroage to type that is needed


## Memory Leaks

This was the valgrind output after receiving a ping message from nc:


```
Valgrind output (after accepting one nc ping message, then CNTRL+C):

==57895== 
==57895== Process terminating with default action of signal 2 (SIGINT)
==57895==    at 0x4994427: accept (accept.c:26)
==57895==    by 0x1096B0: main (server.cpp:84)
==57895== 
==57895== HEAP SUMMARY:
==57895==     in use at exit: 2,584 bytes in 2 blocks
==57895==   total heap usage: 7 allocs, 5 frees, 7,412 bytes allocated
==57895== 
==57895== LEAK SUMMARY:
==57895==    definitely lost: 0 bytes in 0 blocks
==57895==    indirectly lost: 0 bytes in 0 blocks
==57895==      possibly lost: 0 bytes in 0 blocks
==57895==    still reachable: 2,584 bytes in 2 blocks
==57895==         suppressed: 0 bytes in 0 blocks
==57895== Reachable blocks (those to which a pointer was found) are not shown.
==57895== To see them, rerun with: --leak-check=full --show-leak-kinds=all
==57895== 
==57895== For lists of detected and suppressed errors, rerun with: -s
==57895== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)

```

Will likely need to have logic to shut down server, free memory, close sockets, and create crash dump when server is killed
Look up signals API

```
#include <signal.h>

void sig_handler(int s) {
    if (s == SIGINT) {
        printf("\nTerminating Server...\n");
    }
    exit(s);
}

int main() {

  // Register signal and signal handler
  signal(SIGINT, sig_handler);

  return 0;
}
```


#### Dependencies

Need to keep track of dependency libraries installed for this project

Currently Google test is the only one: libgtest-dev

`sudo apt-get install libgtest-dev`

Alternative approach used: Building Google Test from source:

https://github.com/google/googletest

Built as a standalone CMake project:

```
git clone https://github.com/google/googletest.git -b v1.15.2
cd googletest        # Main directory of the cloned repository.
mkdir build          # Create a directory to hold the build output.
cd build
cmake ..             # Generate native build scripts for GoogleTest.
```
Then once the build scripts are generated:

```
make
sudo make install    # Install in /usr/local/ by default
```

This installs the google test libraries in /usr/local (gtest.pc is also added)

Now you can import GTEST. This is the sample code (from AI):

```
#include <gtest/gtest.h>
#include <stdexcept>

void divide(int a, int b, int& result) {
    if (b==0) {
        throw std::runtime_error("Division by zero");
    }
    result = a/b;
}

TEST(DivisionTest, TestDivisionByZero) {
    int result;
    EXPECT_THROW(divide(10, 0, result), std::runtime_error);
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
```

To compile the code and link it with Google Test library:

https://stackoverflow.com/questions/15813723/how-do-i-set-up-google-test-with-a-gnu-make-project

```
g++ -I h -pthread -o test_response tests/test_response.cpp /usr/local/lib/libgtest.a 
```

TODO: Learn what all those other options mean
TODO: See if you can incorporate a build system where you can build the files from source and add the libraries (ie libgtest.a, libgtest_main.a, libgmock.a, libgmock_main.a in the compiler options using must Makefile)


######  Common Response Interface for parsed data

Problem 1:

functions that parse byte data (read_*) can return data in many different types (stirng, int_64, array, etc). Decoding a large stream of bytes representing arrays containing different data types will require calling these functions compositionally.

What C++ language feature can we use for this?

Ideas on top of my head:

1. Union 

https://learn.microsoft.com/en-us/cpp/cpp/unions?view=msvc-170

The MSFT C++ doc recommends that std::variant is a type-safe alternative


2. Modern typesafe version: std::variant

Solution:

Struct that represents protocol message will have a data field that is a variant of the types of data used and an enum type that represents the type of data in the message (which can later be used for type narrowing when reading the variant):

```
enum ProtocolMessageType { SimpleString, SimpleError, Integer, BulkString, Array, Null, Boolean, Double, BigNumber, BulkError, VerbatimString, Map, Attribute, Set, Push };
struct ProtocolMessage {
    ProtocolMessageType protocol_message_type;
    std::variant<std::string, int64_t, std::vector<std::shared_ptr<ProtocolMessage>>> data;
};

```

std::variant can support vectors of ProtocolMessage as well. 

Problem 2: When you instantiate a struct or object in a function that is supposed to be composible and used by other functions, you need to manage the pointer and memory. You can't instantiate an object and then pass it outside scope (dangling?)

Solution: Smart pointers

3 types:

1. unique pointer

- only one function (or rather one scope) owns the object referenced by this pointer. Will be cleared when out of scope


2. shared pointer

- used when object needs to be shared between different scopes (ie functions that perform some composable operation that need to hand off the object to another function without worrying about memory allocation).

- memory freed when all functions using it go out of scope. requires another byte to keep track of references


3. weak pointer

- in conjunction with shared ptr. provides access to oject owned by shared ptr instances, but does not contribute to reference counting.

TODO: read more on weak pointers later


EPOLL Notes:

https://man7.org/linux/man-pages/man7/epoll.7.html

https://en.wikipedia.org/wiki/Epoll

epoll is a linux kernal call used to monitor multiple file descriptors (ie sockets)

1. Use epoll_create1 to create an epoll object

2. use epoll_ctl to add file descripters to be "watched" in the epoll object (register)

3. use epoll_wait (blocking call) to wait for events on the file descriptors registered with the epoll object created in [1]


Triggering modes: Edge triggered and Level Triggered

Sample Code (form Linux Man Pages):
```
#define MAX_EVENTS 10
           struct epoll_event ev, events[MAX_EVENTS];
           int listen_sock, conn_sock, nfds, epollfd;

           /* Code to set up listening socket, 'listen_sock',
              (socket(), bind(), listen()) omitted. */

            // Step 1: Create epoll object
           epollfd = epoll_create1(0);
           if (epollfd == -1) {
               perror("epoll_create1");
               exit(EXIT_FAILURE);
           }

            // Step 2: Register your server's socket fd with epoll object to listen for events
           ev.events = EPOLLIN;
           ev.data.fd = listen_sock;
           if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
               perror("epoll_ctl: listen_sock");
               exit(EXIT_FAILURE);
           }

           for (;;) {
                // Step 3: blocking epoll_wait call for events on any of the registered file descriptors in epoll object
               nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
               if (nfds == -1) {
                   perror("epoll_wait");
                   exit(EXIT_FAILURE);
               }

                // Process all the events returned by epoll_wait
               for (n = 0; n < nfds; ++n) {
                    // If the events are on your server's socket (ie new connection request)
                    // accept connection, set new connection's socket file descriptor to non blocking, and register it 
                    // with your epoll object form step 1 
                   if (events[n].data.fd == listen_sock) {
                       conn_sock = accept(listen_sock,
                                          (struct sockaddr *) &addr, &addrlen);
                       if (conn_sock == -1) {
                           perror("accept");
                           exit(EXIT_FAILURE);
                       }
                       setnonblocking(conn_sock);
                       ev.events = EPOLLIN | EPOLLET;
                       ev.data.fd = conn_sock;
                       if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                                   &ev) == -1) {
                           perror("epoll_ctl: conn_sock");
                           exit(EXIT_FAILURE);
                       }
                   } else {
                        // if the events are on one of the other client's socket file descriptors (ie they have sent data),
                        // process that data
                       do_use_fd(events[n].data.fd);
                   }
               }
           }

```
