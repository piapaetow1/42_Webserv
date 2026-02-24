/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserUtils.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 21:20:14 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/07 16:22:13 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigParser.hpp"
#include "../include/Utils.hpp"

#include <sstream>

void checkSubDir(const string path) {
	int subCount = 0;
	string Dir = path;
	for (size_t i = 0; i < Dir.size(); i++) {
		if (Dir[i] == '.' && Dir[i + 1] == '/') {
			i += 2;
		}
		if (Dir[i] == '/') {
			subCount++;
		}
	}
	if (subCount > 2) {
		throw std::runtime_error("Too many subdirectories in location '/uploads'");
	}
}

void checkDuplicateIpPorts(const vector<ServerBlock> servers)
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		const ServerBlock &sb = servers[i];
		for (size_t k = 0; k < sb.listens.size(); k++)
		{
			const ListenAdress &li = sb.listens[k];
			for (size_t l = i; l < servers.size(); l++)
			{
				const ServerBlock &sb2 = servers[l];
				size_t m_start = (l == i) ? (k + 1) : 0;
				for (size_t m = m_start; m < sb2.listens.size(); m++)
				{
					const ListenAdress &b = sb2.listens[m];
					if (li.port != b.port)
						continue ;
					if (li.ip == b.ip || li.ip == "0.0.0.0" || b.ip == "0.0.0.0")
					{
						std::ostringstream oss;
						oss << "Duplicate Listen address detected: " << li.ip << ":" << li.port << " conflicts with " << b.ip << ":" << b.port << "!";
						throw std::runtime_error(oss.str());
					}
				}
			}
		}
	}
}

void setUploadIds(vector<ServerBlock>& sb) {
	for (size_t i = 0; i < sb.size(); i++) {
		for (size_t j = 0; j < sb[i].listens.size(); j++) {
			sb[i].listens[j].uploadID = buildUploadID(i, j, sb[i].listens[j].port);
		} 
	}
}

bool isServerBlockStart(string line)
{
	size_t start = line.find_first_not_of(" \t");
	if (start == string::npos)
		return false ;
	string clean = line.substr(start);
	if (clean.compare(0, 6, "server") == 0)
	{
		size_t pos = clean.find("{");
		return pos != string::npos;
	}
	return false;
}

string clean(const string &s)
{
    if (s.empty())
        return "";
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n;");
    if (start == string::npos)
        return "";
    string tmp = s.substr(start, end - start + 1);
    if (!tmp.empty() && tmp[tmp.size() - 1] == ';')
        tmp.erase(tmp.size() - 1);
    size_t s2 = tmp.find_first_not_of(" \t\r\n");
    size_t e2 = tmp.find_last_not_of(" \t\r\n");
    if (s2 == string::npos)
        return "";
    return tmp.substr(s2, e2 - s2 + 1);
}

string buildUploadID(size_t listenIndex, size_t socketIndex, int port)
{
	std::stringstream oss, xss, qss;
	oss << listenIndex;
	xss << socketIndex;
	qss << port;
	string res = oss.str() + "_" + xss.str() + "_" + qss.str();
	return res;
}
