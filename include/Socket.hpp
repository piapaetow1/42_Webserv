#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <fcntl.h>

#include "UseNamespace.hpp"

class Socket {
    private:
        struct sockaddr_in address;
        int socketFd;
        int connection;
    public:
        Socket();
        Socket(const Socket &other);
        Socket &operator=(const Socket &other);
        ~Socket();

        Socket(int domain, int type, int protocol, int port, u_long interface);
        
		int bindSocket();
		int listenSocket(int backlog);
		int acceptSocket();
		bool setNonBlocking();
		void closeSocket();
		void testConnection(int test_item);

		int getSocketFd() const;
};

#endif
