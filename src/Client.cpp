#include "../include/Client.hpp"

Client::Client() : fd(-1), hasError(false) {}

Client::Client(const Client &other)
    : fd(other.fd),
      recvBuffer(other.recvBuffer),
      sendBuffer(other.sendBuffer),
      requestStatus(other.requestStatus),
      hasError(other.hasError) {}

Client &Client::operator=(const Client &other)
{
    if (this != &other) {
        fd = other.fd;
        recvBuffer = other.recvBuffer;
        sendBuffer = other.sendBuffer;
        requestStatus = other.requestStatus;
        hasError = other.hasError;
    }
    return *this;
}

Client::~Client() {}

Client::Client(int clientFd, size_t serverIndex) : fd(clientFd), serverIndex(serverIndex), hasError(false)
{
    recvBuffer.clear();
    sendBuffer.clear();
    requestStatus.headersComplete = false;
    requestStatus.bodyComplete = false;
    requestStatus.parseError = false;
    requestStatus.contentLengthExpected = 0;
    requestStatus.contentReceived = 0;
}

int Client::getFd()
{
    return (fd);
}

const string& Client::getRecvBuffer() const
{
    return recvBuffer;
}

ssize_t Client::recvData()
{
    ssize_t bytes = 0;

    char buffer[4096];
    bytes = recv(fd, buffer, sizeof(buffer), 0);
    if (bytes > 0) {
        recvBuffer.append(buffer, bytes);
    }
    return bytes;
}