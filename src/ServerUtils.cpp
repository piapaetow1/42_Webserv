/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 12:01:21 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/09 14:54:59 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/HttpMethods.hpp"
#include "../include/HttpStructs.hpp"
#include "../include/UploadDirectories.hpp"
#include "../include/Utils.hpp"

#include <arpa/inet.h>
#include <sstream>
#include <string.h>

bool isListeningFd(int fd, const vector<int>& listeningFds)
{
    for (size_t i = 0; i < listeningFds.size(); ++i)
    {
        if (listeningFds[i] == fd)
            return true;
    }
    return false;
}

void removeEmptyDirsUpwards(const std::string &dirPath, const std::string &stopDir)
{
    std::string current = dirPath;

    while (!current.empty()) {
        if (rmdir(current.c_str()) != 0)
        {
            if (errno == ENOTEMPTY || errno == EEXIST) {
                break;
            }
            else if (errno == ENOENT) {
                break; 
            }
            else {
                break;
            }
        }
        if (!stopDir.empty() && current == stopDir) {
            break; 
        }
        size_t pos = current.find_last_of('/');
        if (pos == std::string::npos) break;
        if (pos == 0)
            current = "/"; 
        else
            current = current.substr(0, pos);
    }
}

bool isValidHttpVersionFormat(const std::string &version) {
    const std::string prefix = "HTTP/";
    if (version.size() <= prefix.size() || version.substr(0, prefix.size()) != prefix)
        return false;

    std::string numbers = version.substr(prefix.size()); // z.B. "1.1"
    size_t dotPos = numbers.find('.');
    if (dotPos == std::string::npos)
        return false;

    std::string major = numbers.substr(0, dotPos);
    std::string minor = numbers.substr(dotPos + 1);

    if (major.empty() || minor.empty())
        return false;

    for (size_t i = 0; i < major.size(); ++i)
        if (!std::isdigit(major[i]))
            return false;

    for (size_t i = 0; i < minor.size(); ++i)
        if (!std::isdigit(minor[i]))
            return false;

    return true;
}

int    requestErrorHandling(const Request& req, const ServerBlock& cfg, int clientFd)
{
	if (!isValidHttpVersionFormat(req.version)) {
		cout << "Malformed HTTP version: " << req.version << endl;
		Response errRes = sendErrorPage(cfg, 400);
		string responseStr = buildResponseString(req.version, errRes);
		send(clientFd, responseStr.c_str(), responseStr.size(), 0);
		close(clientFd);
		return 2;
	}
    if (req.version != "HTTP/1.0" && req.version != "HTTP/1.1")
    {
        cout << "unsupported HTTP version: " << req.version << endl;
        Response errRes = sendErrorPage(cfg, 505);
		string responseStr = buildResponseString(req.version, errRes);
		send(clientFd, responseStr.c_str(), responseStr.size(), 0);
        close(clientFd);
        return 2;
    }

	Response res;
	if (req.body.size() > static_cast<size_t>(cfg.maxBodySize))
    {
		cout << "Payload too large!!" << endl;
        return 1;
	}
	const Location* loc = getMatchedLocation(req.path, cfg);
	if (loc && !isMethodAllowed(req.method, req.path, cfg))
    {
		cout << "method " << req.method << " not allowed" << endl; 
		Response errRes = sendErrorPage(cfg, 405);
		std::string responseStr = buildResponseString(req.version, errRes);
		send(clientFd, responseStr.c_str(), responseStr.size(), 0);
		close(clientFd);
		return 2;
	}
    return 0;
}
