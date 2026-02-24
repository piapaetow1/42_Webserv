/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 17:00:50 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/08 14:42:21 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Utils.hpp"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sstream>
#include <algorithm>

string getMimeType(const string &path)
{
    size_t dot = path.find_last_of(".");
    if (dot != string::npos)
    {
        string ext = path.substr(dot + 1);
		for (int i = 0; ext[i]; i++)
			ext[i] = (char)std::tolower(ext[i]);
        if (ext == "html")
            return "text/html";
        if (ext == "css")
            return "text/css";
        if (ext == "js")
            return "application/javascript";
        if (ext == "png")
            return "image/png";
        if (ext == "jpg" || ext == "jpeg")
            return "image/jpeg";
        if (ext == "gif")
            return "image/gif";
		if (ext == "ico")
            return "image/x-icon";
		if (ext == "svg")
            return "image/svg+xml";
		if (ext == "woff" || ext == "woff2")
            return "font/woff2";
	}
    return "text/plain";
}

string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == string::npos)
        return "";
    return s.substr(start, end - start + 1);
}

string urlDecode(const string &s)
{
	string result;

	for (size_t i = 0; i < s.size(); i++)
    {
		if (s[i] == '%' && i + 2 < s.size() && std::isxdigit(s[i + 1]) && std::isxdigit(s[i + 2]))
        {
			string hex = s.substr(i + 1, 2);
			char decodChar = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
			result += decodChar;
			i+= 2;
		}
        else if (s[i] == '+')
			result += ' ';
		else
			result += s[i];
	} 
	return result;
}

bool isCgiRequest(const string& uri, const ServerBlock& server)
{
    const Location* loc = getMatchedLocation(uri, server);
	if (!loc)
        return false;
	size_t dot = uri.rfind('.');
	if (dot == string::npos)
		return false;
	string ext = uri.substr(dot);
	for (size_t i = 0; i < loc->cgiExtensions.size(); ++i)
    {
        if (loc->cgiExtensions[i] == ext)
            return true;
    }
    return false;
}

string buildResponseString(string reqVersion, const Response &res, const string &filePath)
{
	string version;
	if (reqVersion == "") 
		version = "HTTP/1.1";
	else 
		version = reqVersion;
		
    std::ostringstream out;

    out << version << " " << res.statusCode << " " << res.statusText << "\r\n";
    map<string, string>::const_iterator it;
    for (it = res.headers.begin(); it != res.headers.end(); ++it)
        out << it->first << ": " << it->second << "\r\n";
    if (!filePath.empty())
        out << "Content-Type: " << getMimeType(filePath) << "\r\n";
    out << "\r\n";
    out << res.body;
    return out.str();
}

bool isMethodAllowed(const string &method, const string &requestPath, const ServerBlock &cfg)
{    
	const vector<Location> &locations = cfg.locations;
    for (size_t i = 0; i < locations.size(); ++i)
    {
        if (requestPath.find(locations[i].path) == 0)
        {
            const vector<string> &methods = locations[i].allowedMethods;
            for (size_t j = 0; j < methods.size(); ++j)
            {
                if (methods[j] == method)
                return true;
            }
            return false;
        }
    }
    return false;
}

bool dirExists(const string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

bool fileExists(const string &path)
{
    struct stat s;
    return (stat(path.c_str(), &s) == 0);
}

bool isDirectory(const string &p)
{
    struct stat st;
    if (stat(p.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

bool isRegularFile(const string &p)
{
    struct stat st;
    if (stat(p.c_str(), &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

const Location* getMatchedLocation(const string& uri, const ServerBlock& server)
{
    const Location* bestMatch = NULL;
	size_t bestMatchLen = 0;

	for (vector<Location>::const_iterator it = server.locations.begin(); it != server.locations.end(); ++it)
    {
		const string& path = it->path;
		if (uri.find(path) == 0 && path.length() > bestMatchLen)
        {
			bestMatch = &(*it);
			bestMatchLen = path.length();
		}
	}
	return bestMatch;
}

ListenAdress getListenAddrForClientFd(int clientFd)
{
	struct sockaddr_in addr;
	ListenAdress la;

	la.ip = "0.0.0.0";
	la.port = 0;
	socklen_t len = sizeof(addr);
	if (getsockname(clientFd, (struct sockaddr*)&addr, &len) < 0)
		return la;
	char buf[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf)) == NULL)
		la.ip = "0.0.0.0";
	else
		la.ip = string(buf);
	la.port = ntohs(addr.sin_port);
	return la;
}
