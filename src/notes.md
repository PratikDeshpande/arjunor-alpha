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