/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 20:29:27 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/09 13:07:56 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigParser.hpp"
#include "../include/Utils.hpp"
#include "../include/Colors.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <arpa/inet.h>

ListenAdress parseListenToken(const string &token)
{
	ListenAdress la;

	la.ip = "";
	la.port = -1;
	string t = token;
	if (!t.empty() && t[t.size()-1] == ';')
		t = t.substr(0, t.size() - 1);
	size_t colon = t.find(':');
	string ipPart;
	string portPart;
	if (colon != string::npos)
	{
		ipPart = t.substr(0, colon);
		portPart = t.substr(colon + 1);
	}
	else
		portPart = t;
	if (portPart.empty())
		throw std::runtime_error("listen directive missing port number");
	long p = std::strtol(portPart.c_str(), NULL, 10);
	if (p <= 0 || p > 65535)
		throw std::runtime_error("Invalid port in listen directive");
	la.port = static_cast<int>(p);
	if (!ipPart.empty())
	{
		if (ipPart == "*")
			la.ip = "0.0.0.0";
		else
		{
			struct in_addr addr4;
			if (inet_pton(AF_INET, ipPart.c_str(), &addr4) == 1)
				la.ip = ipPart;
			else
				throw std::runtime_error(string("Invalid IP address in listen directive: ") + ipPart);
		}
	}
	else
		la.ip = "0.0.0.0";
	return la;	
}

void parseServerBlock(std::ifstream &file, ServerBlock &sb)
{
    string line;
    bool inLocation = false;
    Location currentLoc;
	sb.hasListen = false;
	sb.hasRoot = false;
    while (getline(file, line))
	{
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream iss(line);
        string key;
        iss >> key;
        if (!inLocation && isServerBlockStart(line))
		{
            file.seekg(-((std::streamoff)line.size() + 1), std::ios_base::cur);
            break;
        }
        if (key == "location")
		{
            inLocation = true;
            currentLoc = Location();
            string path;
            iss >> path;
            currentLoc.path = clean(path);
            currentLoc.autoindex = false;
            currentLoc.allowedMethods.clear();
            continue;
        }
        if (inLocation) {
			bool finished = parseLocationDirective(currentLoc, sb, key, iss);
			if (finished)
				inLocation = false;
			continue ;
        }
		else {
			ServerParseResult res = parseServerDirective(sb, key, iss);
			if (res == SERVER_END)
				break ;
			if (res == SERVER_UNKNOWN)
				cerr << RED << "Warning: Unknown directive: " << key << RESET << std::endl;
		}
    }
	finalizeServerBlock(sb);
}

vector<ServerBlock> parse(const string &filename)
{
	vector<ServerBlock> servers;

	std::ifstream file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Cannot open configuration file.");
	string line;
	while (getline(file, line))
	{
		if (isServerBlockStart(line))
		{
			ServerBlock sb;
			sb.hasListen = false;
			sb.hasRoot = false;
			sb.maxBodySize = -1;
			parseServerBlock(file, sb);
			if (!sb.hasListen)
				throw std::runtime_error("Missing 'listen' in server block!");
			if (!sb.hasRoot)
				throw std::runtime_error("Missing 'root' in server block!");
			servers.push_back(sb);
		}
		else
			continue ;
	}
	setUploadIds(servers);
	checkDuplicateIpPorts(servers);
	return servers;
}
