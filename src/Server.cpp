#include "../include/Server.hpp"
#include "../include/HttpMethods.hpp"
#include "../include/HttpStructs.hpp"
#include "../include/UploadDirectories.hpp"
#include "../include/Utils.hpp"
#include "../include/Colors.hpp"
#include "../include/RequestHandler.hpp"

#include <cstdio>
#include <csignal>
#include <arpa/inet.h>
#include <cerrno>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

Server* Server::signalInstance = NULL;

Server::Server()
{}

Server::Server(const Server &other) {
    *this = other;
}

Server &Server::operator=(const Server &other)
{
    if (this != &other)
    {
         serverBlocks = other.serverBlocks;
        listeningSockets = other.listeningSockets;
        isRunning = other.isRunning;
        clients = other.clients;
    }
    return *this;
}

Server::~Server()
{
	for (size_t i = 0; i < listeningSockets.size(); ++i)
    {
        for (size_t j = 0; j < listeningSockets[i].size(); ++j)
        {
            close(listeningSockets[i][j].getSocketFd());
        }
    }
    listeningSockets.clear();

    pollFds.clear();
    vector<string> processedDirs;

    for (size_t i = 0; i < serverBlocks.size(); ++i) {
        const ServerBlock &sb = serverBlocks[i];
        string root = sb.root;
        if (!root.empty() && root[root.size() - 1] == '/')
            root.resize(root.size() - 1);
        if (root.empty()) {
            continue;
        }
        for (size_t li = 0; li < sb.listens.size(); ++li)
        {
            std::ostringstream oss;
            oss << sb.listens[li].port;
            string portStr = oss.str();
			const Location *loc = getMatchedLocation("/uploads", sb);
			string LocRoot = loc->root;
			string uploadDir = LocRoot + "/" + createUploadDir(i, li);
            bool already = false;
            for (size_t p = 0; p < processedDirs.size(); ++p)
            {
                if (processedDirs[p] == uploadDir)
                {
                    already = true;
                    break;
                }
            }
            if (already)
                continue;
            processedDirs.push_back(uploadDir);
            string filesDir = uploadDir + "/files";
            DIR *dir = opendir(filesDir.c_str());
            if (dir)
            {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL)
                {
                    string name = entry->d_name;
                    if (name == "." || name == "..")
                        continue;
                    string fullPath = filesDir + "/" + name;
                    struct stat st;
                    if (stat(fullPath.c_str(), &st) == 0)
                    {
                        if (S_ISREG(st.st_mode))
                        {
                            if (remove(fullPath.c_str()) != 0)
                                cerr << RED << "Destructor: failed to remove file " << fullPath << ": " << strerror(errno) << RESET << endl;
                        }
                        else
                            cerr << RED << "Destructor: skipping non-file entry " << fullPath << RESET << endl;
                    }
                    else
                        cerr << RED << "Destructor: stat failed for " << fullPath << ": " << strerror(errno) << RESET << endl;
                }
                closedir(dir);
            }
			rmdir(filesDir.c_str());
			DIR *udir = opendir(uploadDir.c_str());
            if (udir) {
				struct dirent *entry;
				while ((entry = readdir(udir)) != NULL)
				{
					string name = entry->d_name;
					if (name == "." || name == "..")
						continue;
					string fullPath = uploadDir + "/" + name;
					struct stat st;
					if (stat(fullPath.c_str(), &st) == 0)
					{
						if (S_ISREG(st.st_mode))
						{
							if (remove(fullPath.c_str()) != 0)
								cerr << RED << "Destructor: failed to remove file " << fullPath << ": " << strerror(errno) << RESET << endl;
						}
						else
							cerr << RED << "Destructor: skipping non-file entry " << fullPath << RESET << endl;
					}
					else
						cerr << RED << "Destructor: stat failed for " << fullPath << ": " << strerror(errno) << RESET << endl;
				}
				closedir(udir);
			}
			removeEmptyDirsUpwards(uploadDir);
        }
		const Location *loc = getMatchedLocation("/uploads", sb);
		string LocRoot = loc->root;
		rmdir(LocRoot.c_str());
    } 
    cout << "\nServer destructor finished cleanup (upload files removed where possible)" << endl;
}

std::ostream &operator<<(std::ostream &out, const Server &obj)
{
    out << obj.getIsRunning();
    return out;
}

Server::Server(vector<ServerBlock>& servers) : serverBlocks(servers), isRunning(false)
{
	buildUploadRoot(servers);
}

void Server::init()
{
    listeningSockets.clear();
    pollFds.clear();
    for (size_t i = 0; i < serverBlocks.size(); ++i) {
        vector<Socket> socketsForBlock;
		for (size_t li = 0; li < serverBlocks[i].listens.size(); ++li) {
			const ListenAdress &la = serverBlocks[i].listens[li];
			cout << "trying to create listening socket for server " << i << " on " << la.ip << ":" << la.port << endl;
			in_addr_t addr = INADDR_ANY; 
			if (!la.ip.empty() && la.ip != "0.0.0.0") {
				addr = inet_addr(la.ip.c_str());
				if (addr == INADDR_NONE) {
					cerr << RED << "invalid listen IP '" << la.ip << "' for server " << i << ", skipping this listen entry." << RESET << endl;
					continue ;
				}
			}
			Socket sock(AF_INET, SOCK_STREAM, 0, la.port, addr);
			if (!sock.setNonBlocking()) {
				cerr << RED << "Error: setting socket non-blocking!" << RESET << endl;
				sock.closeSocket();
				continue;
			}
			if (sock.bindSocket() < 0) {
				cerr << RED << "Error: unable to bind socket!" << RESET << endl;
				sock.closeSocket();
				continue;
			}
			if (sock.listenSocket(SOMAXCONN) < 0) {
				cerr << RED << "Error: unable to set socket on listen!" << RESET << endl;
				sock.closeSocket();
				continue;
			}
			socketsForBlock.push_back(sock);
			struct pollfd pfd;
			pfd.fd = sock.getSocketFd();
			pfd.events = POLLIN | POLLHUP | POLLERR | POLLNVAL;
			pfd.revents = 0;
			pollFds.push_back(pfd);
		}
		if (!socketsForBlock.empty())
			listeningSockets.push_back(socketsForBlock);
		else
			cerr << RED << "Warning: No listening sockets created for server block index " << i << RESET << endl;
    }
	if (listeningSockets.empty()) {
		throw std::runtime_error("No socket could be bound. Shutting down server...");
	}
	signalInstance = this;
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);	
}

string Server::createUploadDir(size_t listenIndex, size_t socketIndex)
{
	std::stringstream oss, xss, qss;
	oss << listenIndex;
	xss << socketIndex;
	qss << serverBlocks[listenIndex].listens[socketIndex].port;
	string res = oss.str() + "_" + xss.str() + "_" + qss.str();
	return res;
}

void Server::run()
{
    cout << BOLD << YELLOW << "\nServer run called\n" << RESET << endl;
    isRunning = true;
	struct pollfd stdinPoll;
	stdinPoll.fd = 0;
	stdinPoll.events = POLLIN;
	pollFds.push_back(stdinPoll);
    while (isRunning)
    {
        int ret = poll(&pollFds[0], pollFds.size(), 1000);
        if (ret < 0)
        {
            cerr << RED << "poll-error!" << RESET << endl;
            break;
        }
        for (size_t i = 0; i < pollFds.size(); ++i)
        {
            if (pollFds[i].revents & POLLHUP || pollFds[i].revents & POLLERR || pollFds[i].revents & POLLNVAL)
            {
                int fd = pollFds[i].fd;
                cout << "closed connection on Client FD " << RED << fd << RESET << endl << endl;
                pollFds.erase(pollFds.begin() + i);
                --i;
                for ( size_t c = 0; c < clients.size(); ++c)
                {
                    if (clients[c]->getFd() == fd)
                    {
                        delete clients[c];
                        clients.erase(clients.begin() + c);
                        break;
                    }
                }
                continue;
            }
            if (pollFds[i].revents & POLLIN)
            {
                int fd = pollFds[i].fd;
				if (fd == 0)
                {
					char buf[128];
					ssize_t n = read(0, buf, sizeof(buf)-1);
					if (n == 0)
                    {
						cout << "EOF received." << endl;
						isRunning = false;
						continue;
					}
					if (n > 0)
                    {
						buf[n] = '\0';
						string input(buf);
						if (!input.empty() && input[input.size()-1] == '\n')
							input.erase(input.size()-1);
						if (input == "EXIT") {
							cout << MAGENTA << BOLD << "EXIT " << RESET << "received, shutting down server..." << endl;
							isRunning = false;
						}
					}
					continue;
				}
                bool isListening = false;
                size_t listeningIndex = 0;
				size_t socketIndexInBlock = 0;
				for (size_t j = 0; j < listeningSockets.size(); ++j)
                {
					for (size_t k = 0; k < listeningSockets[j].size(); ++k)
                    {
						if (listeningSockets[j][k].getSocketFd() == fd)
                        {
							isListening = true;
							listeningIndex = j;
							socketIndexInBlock = k;
							uploadDir = createUploadDir(listeningIndex, socketIndexInBlock);
							
							break;
						}
					}
					if (isListening)
                        break;
				}
				if (isListening)
                {
					cout << "listening socket event on Server FD " << GREEN << fd << RESET << endl;
					int clientFd = listeningSockets[listeningIndex][socketIndexInBlock].acceptSocket();

					if (clientFd >= 0)
                    {
						cout << "connection accepted on Client FD " << BLUE << clientFd << RESET << endl << endl;
						fcntl(clientFd, F_SETFL, O_NONBLOCK);
						Client* client = new Client(clientFd, listeningIndex);
						clients.push_back(client);
						struct pollfd clientPfd;
						clientPfd.fd = clientFd;
						clientPfd.events = POLLIN | POLLHUP | POLLERR | POLLNVAL;
						clientPfd.revents = 0;
						pollFds.push_back(clientPfd);
					}
                    else
						cerr << RED << "acceptSocket() failed!" << RESET << endl;
				}
				else
                {
					for (size_t c = 0; c < clients.size(); ++c)
                    {
						if (clients[c]->getFd() == fd)
                        {
							handleRequest(clients[c]->getServerIndex(), fd, uploadDir);
							break;
						}
					}
				}
            }
        }
    }
}

void Server::handleCorrectClient(Client *client, size_t serverIndex, int clientFd, string uploadDir)
{
    ssize_t bytes = client->recvData();
    if (bytes == 0)
    {
        cout << "Client " << clientFd << " shut down connection." << endl;
        close(clientFd);
        return;
    }
    else if (bytes < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            cout << "no further data to read (EAGAIN/EWOULDBLOCK)." << endl;
            return;
        }
        else
        {
            cerr << RED << "error receiving data " << clientFd << ": " << strerror(errno) << RESET << endl;
            close(clientFd);
            return;
        }
    }
    const string &buffer = client->getRecvBuffer();
    if (!headerComplete(buffer))
    {
        cout << "header incomplete." << endl;
        return;
    }
	ServerBlock &cfg = serverBlocks[serverIndex];
	long long contentLen = getContentLength(buffer);
    if (checkRequestValid(&buffer, cfg, contentLen, clientFd))
    {
        return;
    }
    Request req;
    try
    {
        req = parseRequest(buffer);
    }
    catch (const std::exception &e)
    {
        cerr << RED << "error parsing request: " << e.what() << RESET << endl;
        Response errRes = sendErrorPage(cfg, 400);
		string responseStr = buildResponseString(req.version, errRes);
		send(clientFd, responseStr.c_str(), responseStr.size(), 0);
        close(clientFd);
        return;
    }
    Response res;
    int errorCode = requestErrorHandling(req, cfg, clientFd);
    if (errorCode == 1)
        res = sendErrorPage(cfg, 413);
    else if (errorCode == 2)
    {
        return;
    }
	if (isCgiRequest(req.path, cfg))
    {
        res = handleCGI(req, cfg);
    }
	else if (req.method == "GET")
        res = handleGet(req, cfg, uploadDir);
    else if (req.method == "POST")
        res = handlePost(req, cfg, uploadDir);
    else if (req.method == "DELETE")
        res = handleDelete(req, cfg, uploadDir);
    else
		res = sendErrorPage(cfg, 501);
	string responseStr = buildResponseString(req.version, res);
	cout << CYAN << BOLD << "\n\n[SERVER]\n" << RESET << req.method << " " << req.path
		<< "\n" << req.version << " " << res.statusCode << " " << res.statusText << "\n" 
		"Content-Length: " << res.headers["Content-Length"] << "\n" << "Content-Type: " 
		<< res.headers["Content-Type"] << endl << endl;
	send(clientFd, responseStr.c_str(), responseStr.size(), 0);
    close(clientFd);
    return;
}

void Server::handleRequest(size_t serverIndex, int clientFd, string uploadDir)
{
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getFd() == clientFd)
        {
            handleCorrectClient(clients[i], serverIndex, clientFd, uploadDir);
        }
    }
}

void Server::closeClient(size_t serverIndex, int clientFd)
{
    cout << "close client " << clientFd << " on server block " << serverIndex << endl;
}

void Server::shutdown()
{
    cout << "Server shutdown called" << endl;
}

void Server::stop()
{
	isRunning = false;
	cout << "stop called." << endl;
}


string intToString(int number)
{
    std::ostringstream oss;
    oss << number;
    return oss.str();
}

std::string	getStatusText(int errorCode)
{
	switch (errorCode) {
		case 400: return "Bad Request";
		case 404: return "Not Found";
		case 403: return "Forbidden";
		case 405: return "Method Not Allowed"; 
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 505: return "HTTP Version Not Supported";
		default:  return "Error";
	}
}

static void replaceAll(string &data, const string &from, const string &to)
{
    size_t pos = 0;
    while ((pos = data.find(from, pos)) != string::npos)
    {
        data.replace(pos, from.length(), to);
        pos += to.length();
    }
}

Response sendErrorPage(const ServerBlock &cfg, int errorCode)
{
    Response res;
    res.statusCode = errorCode;
	
    map<int, string>::const_iterator i = cfg.errorPages.find(errorCode);
    if (i != cfg.errorPages.end())
    {
		const string filePath = i->second;
        std::ifstream file(filePath.c_str());
        if (file)
        {
            std::stringstream ss;
            ss << file.rdbuf();
            res.body = ss.str();
			replaceAll(res.body, "{{INDEX}}", cfg.indexFile);
        }
    }
    else
    {
        cout << "no error page configured for this error code: " << errorCode << endl;
        std::ostringstream oss;
        oss << "<html><head><title>" << errorCode << "</title></head>"
            << "<body><h1>" << errorCode << " Error</h1></body></html>";
        res.body = oss.str();
    }
    res.statusText = getStatusText(errorCode);
    std::ostringstream len;
    len << res.body.size();
	res.headers["Content-Type"] = "text/html";
    res.headers["Content-Length"] = len.str();
    return res;
}

bool Server::getIsRunning() const
{
    return isRunning;
}


ListenAdress Server::getListenAdressFromSocket(int clientFd)
{
    for (size_t i = 0; i < listeningSockets.size(); ++i)
    {
        for (size_t j = 0; j < listeningSockets[i].size(); ++j)
        {
            if (listeningSockets[i][j].getSocketFd() == clientFd)
                return serverBlocks[i].listens[j];
        }
    }
    if (!serverBlocks.empty() && !serverBlocks[0].listens.empty())
        return serverBlocks[0].listens[0];
    ListenAdress l; l.ip = "0.0.0.0"; l.port = 0;
    return l;
}