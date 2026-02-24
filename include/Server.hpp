#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <poll.h>
#include <map>
#include <algorithm>
#include <sstream>
#include "Socket.hpp"
#include "ConfigParser.hpp"
#include "Client.hpp"
#include "HttpStructs.hpp"

#include "UseNamespace.hpp"
#include "Colors.hpp"

class Server {
    private:
        vector<ServerBlock> serverBlocks; 
        vector<vector<Socket> > listeningSockets;
        vector<Client*> clients;
        vector<struct pollfd> pollFds;
        bool isRunning;
		static Server* signalInstance;

		string uploadDir;

		static void handleSignal(int signum) {
			cout << BOLD << ORANGE << "\nreceived signal " << signum << ", shutting down...\n" << RESET << endl;
			if (signalInstance) {
				signalInstance->stop();
			}
		};

    public:
        Server();
        Server(const Server &other);
        Server &operator=(const Server &other);
        ~Server();
        Server(vector<ServerBlock>& servers);

        void init();
        void run();

        void handleRequest(size_t serverIndex, int clientFd, string uploadDir);
        void handleCorrectClient(Client *client, size_t serverIndex, int clientFd, string uploadDir);
        void closeClient(size_t serverIndex, int clientFd);
        void shutdown();
		void stop();

        bool getIsRunning() const;

		ListenAdress getListenAdressFromSocket(int clientFd);

		string createUploadDir(size_t listenIndex, size_t socketIndex);
};

std::ostream &operator<<(std::ostream &out, const Server &obj);

bool	isListeningFd(int fd, const vector<int>& listeningFds);

void	removeEmptyDirsUpwards(const std::string &dirPath, const std::string &stopDir = "");
int   	requestErrorHandling(const Request& req, const ServerBlock& cfg, int clientFd);


#endif
