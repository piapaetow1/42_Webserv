#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#include "Socket.hpp"
#include "ConfigParser.hpp"
#include "UseNamespace.hpp"

struct RequestStatus {
    bool headersComplete;
    bool bodyComplete;
    bool parseError;
    size_t contentLengthExpected;
    size_t contentReceived;
};


class Client {
    private:
        int fd;
		size_t serverIndex;
        string recvBuffer;
        string sendBuffer;
        RequestStatus requestStatus;
        bool hasError;

    public:
        Client();
        Client(const Client &other);
        Client &operator=(const Client &other);
        ~Client();

		Client(int clientFd, size_t serverIndex);

        int getFd();
        const string& getRecvBuffer() const;
        ssize_t recvData();
		
		size_t getServerIndex() const { return serverIndex; }
};

#endif