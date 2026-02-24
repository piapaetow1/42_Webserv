#include "../include/Socket.hpp"
#include "../include/Colors.hpp"

Socket::Socket()
{}

Socket::Socket(const Socket &other)
{
    address = other.address;
    connection = other.connection;
    socketFd = other.socketFd;
}

Socket &Socket::operator=(const Socket &other)
{
    if (this != &other)
    {
        address = other.address;
        connection = other.connection;
        socketFd = other.socketFd;
    }
    return *this;
}

Socket::~Socket()
{}

Socket::Socket(int domain, int type, int protocol, int port, u_long interface)
{
    socketFd = socket(domain, type, protocol);
    testConnection(socketFd);
    address.sin_family = domain;
    address.sin_port = htons(port);
	if (interface == INADDR_ANY)
		address.sin_addr.s_addr = htonl(interface);
    else
		address.sin_addr.s_addr = interface;
}

void    Socket::testConnection(int test_item)
{
    if (test_item < 0)
    {
        cerr << RED << "Socket error on " << test_item << RESET << endl;
        exit(EXIT_FAILURE);
    }
}

int Socket::bindSocket()
{
    return bind(socketFd, (struct sockaddr *)&address, sizeof(address));
}

int Socket::listenSocket(int backlog)
{
    return listen(socketFd, backlog);
}

int Socket::acceptSocket()
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(socketFd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0)
        cerr << RED << "Accept error" << RESET << endl;
    return client_fd;
}

bool Socket::setNonBlocking()
{
    int flags = fcntl(socketFd, F_GETFL, 0);
    if (flags == -1)
        return false;
    if (fcntl(socketFd, F_SETFL, flags | O_NONBLOCK) == -1)
        return false;
    return true;
}

void Socket::closeSocket()
{
    if (socketFd >= 0)
        close(socketFd);
    socketFd = -1;
}

int Socket::getSocketFd() const
{
    return socketFd;
}
