/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserDetails.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 15:19:22 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/09 13:08:16 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigParser.hpp"
#include "../include/Colors.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <arpa/inet.h>

bool	parseLocationDirective(Location &currentLoc, ServerBlock &sb, string &key, std::istringstream &iss) {
	if (key == "}")
	{
		if (currentLoc.allowedMethods.empty())
			currentLoc.allowedMethods.push_back("GET");
		if (currentLoc.root.empty())
			throw std::runtime_error("Location has no root.");
		sb.locations.push_back(currentLoc);
		return true;
	}
	else if (key == "root")
	{
		string val;
		iss >> val;
		if (!currentLoc.root.empty())
			throw std::runtime_error ("duplicate 'root' detected in current location");
		currentLoc.root = clean(val);
		if (currentLoc.path == "/uploads") {
			checkSubDir(currentLoc.root);
		}
	}
	else if (key == "allow_methods")
	{
		string method;
		while (iss >> method)
		{
			if (clean(method) != "GET" && clean(method) != "POST" && clean(method) != "DELETE")
				throw std::runtime_error("Invalid method detected");
			else
				currentLoc.allowedMethods.push_back(clean(method));
		}
	}
	else if (key == "redirect")
	{
		RedirectRule rd;
		string method, target;
		int status = 303;
		iss >> method >> target;
		if (clean(method) != "GET" && clean(method) != "POST" && clean(method) != "DELETE")
			throw std::runtime_error("Invalid method detected");
		int tmp;
		if (iss >> tmp)
			status = tmp;
		rd.method = clean(method);
		rd.target = clean(target);
		rd.statusCode = status;
		currentLoc.redirects.push_back(rd);				
	}
	else if (key == "autoindex")
	{
		string val;
		iss >> val;
		val = clean(val);
		currentLoc.autoindex = (val == "on");
	}
	else if (key == "cgi_extension")
	{
		string val;
		while (iss >> val)
			currentLoc.cgiExtensions.push_back(clean(val));
	}
	else if (key == "cgi_interpreter")
	{
		string ext, path;
		iss >> ext >> path;
		if (!ext.empty() && !path.empty())
			currentLoc.cgiInterpreterMap[clean(ext)] = clean(path);
	} 
	else
		cerr << RED << "Warning: Unknown directive inside location: " << key << RESET << endl;
	return false;
}

ServerParseResult	parseServerDirective(ServerBlock &sb, string &key, std::istringstream &iss) {
	if (key == "listen")
	{
		string token;
		iss >> token;
		if (token.empty())
			throw std::runtime_error("'listen' must have a value");
		vector<string> tokens;
		tokens.push_back(token);
		string more;
		while (iss >> more)
			tokens.push_back(more);
		for (size_t i = 0; i < tokens.size(); ++i) {
			ListenAdress la = parseListenToken(clean(tokens[i]));
			for (size_t k = 0; k < sb.listens.size(); ++k) {
				if (sb.listens[k].ip == la.ip && sb.listens[k].port == la.port)
					throw std::runtime_error("Duplicate in listen detected.");
			}
			sb.listens.push_back(la);
			sb.hasListen = true;
		}
		return SERVER_HANDLED;
	}
	else if (key == "root")
	{
		string val;
		iss >> val;
		sb.root = clean(val);
		if (sb.root == "")
			throw std::runtime_error("'root' must have a value.");
		if (sb.hasRoot == true)
			throw std::runtime_error("Duplicate of 'root' detected.");
		sb.hasRoot = true;
		return SERVER_HANDLED;
	}
	else if (key == "index")
	{
		string val;
		iss >> val;
		if (!sb.indexFile.empty())
			throw std::runtime_error("Duplicate 'index' detected");
		sb.indexFile = clean(val);
		return SERVER_HANDLED;
	}
	else if (key == "client_max_body_size")
	{
		string val;
		iss >> val;
		if (sb.maxBodySize != -1)
			throw std::runtime_error("Duplicate 'client_max_body_size' detected");
		sb.maxBodySize = std::atoi(clean(val).c_str());
		return SERVER_HANDLED;
	}
	else if (key == "server_name")
	{
		string val;
		iss >> val;
		if (!sb.name.empty())
			throw std::runtime_error("Duplicate 'server_name' detected.");
		sb.name = clean(val);
		return SERVER_HANDLED;
	}
	else if (key == "error_page")
	{
		string codeStr, path;
		iss >> codeStr >> path;
		int code = std::atoi(clean(codeStr).c_str());
		sb.errorPages[code] = clean(path);
		return SERVER_HANDLED;
	}
	else if (key == "}")
		return SERVER_END;
	return SERVER_UNKNOWN;
}

void finalizeServerBlock(ServerBlock sb) {
	    if (!sb.hasListen || sb.listens.empty())
        throw std::runtime_error("Missing 'listen' directive in server block");
    if (!sb.hasRoot)
	{
        throw std::runtime_error("Missing 'root' directive in server block");
	}
	bool hasUpload = false;
	for (size_t i = 0; i < sb.locations.size(); i++)
	{
		if (sb.locations[i].path.find("uploads"))
		{
			if (hasUpload)
				throw std::runtime_error("Duplicate 'uploads' directive detected");
			hasUpload = true;
			break;
		}
	}
	if (!hasUpload)
		throw std::runtime_error("Missing 'upload' directive in server block");
    if (sb.indexFile.empty())
		sb.indexFile = "index.html";
    if (sb.maxBodySize <= 0)
		sb.maxBodySize = 1000000;
    if (sb.name.empty())
		sb.name = "localhost";
}