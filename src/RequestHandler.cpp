/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 12:22:11 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/08 14:38:45 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/RequestHandler.hpp"
#include "../include/HttpMethods.hpp"
#include "../include/Utils.hpp"

#include <csignal>
#include <arpa/inet.h>
#include <sstream>

bool headerComplete(const string& buffer)
{
    return buffer.find("\r\n\r\n") != string::npos || buffer.find("\n\n") != string::npos;
}

bool requestLineValid(const string& buffer)
{
    size_t lineEnd = buffer.find("\r\n");
    if (lineEnd == string::npos)
        lineEnd = buffer.find("\n");
    if (lineEnd == string::npos)
    {
        return false;
    }
	return true;
}

bool bodyComplete(const string& buffer)
{
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == string::npos)
        headerEnd = buffer.find("\n\n");
    if (headerEnd == string::npos)
        return false;
    size_t bodyStart = headerEnd + ((buffer[headerEnd] == '\r') ? 4 : 2);
    size_t contentLength = getContentLength(buffer);
    size_t bodyReceived = buffer.size() - bodyStart;
    return bodyReceived >= contentLength;
}

size_t getContentLength(const string& buffer)
{
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == string::npos)
        headerEnd = buffer.find("\n\n");
    if (headerEnd == string::npos)
        return 0;
    string headers = buffer.substr(0, headerEnd);
    size_t pos = headers.find("Content-Length:");
    if (pos != string::npos)
    {
        pos += 15;
        while (pos < headers.size() && (headers[pos] == ' ' || headers[pos] == '\t'))
            ++pos;
        istringstream iss(headers.substr(pos));
        size_t contentLength = 0;
        iss >> contentLength;
        return contentLength;
    }
    return 0;
}

int    checkRequestValid(const string* buffer, const ServerBlock& cfg, long long contentLen, int clientFd)
{
	if (contentLen > 0)
	{
		if ((size_t)contentLen > (unsigned long)cfg.maxBodySize)
		{
			Response err = sendErrorPage(cfg, 413);
			string responseStr = buildResponseString("", err);
			send(clientFd, responseStr.c_str(), responseStr.size(), 0);
			close(clientFd);
			return 1;
		}
	}
	bool isChunked = false;
	if (buffer->find("Transfer-Encoding: chunked") != string::npos ||
		buffer->find("transfer-encoding: chunked") != string::npos)
	{
		isChunked = true;
	}
	if (isChunked)
	{
		size_t headerEnd = buffer->find("\r\n\r\n");
		if (headerEnd == string::npos)
			headerEnd = buffer->find("\n\n");
		if (headerEnd != string::npos)
		{
			size_t bodyStart = headerEnd + 4;
			size_t rawBodyBytes = 0;
			if (buffer->size() > bodyStart)
				rawBodyBytes = buffer->size() - bodyStart;
			if (rawBodyBytes > (unsigned long)cfg.maxBodySize)
			{
				Response err = sendErrorPage(cfg, 413);
				string responseStr = buildResponseString("", err);
				send(clientFd, responseStr.c_str(), responseStr.size(), 0);
				close(clientFd);
				return 1;
			}
		}
	}
    if (!requestLineValid(*buffer) || (isChunked && contentLen > 0))// HIER GEÄNDERT DASS CONTENTLENGTH BEI CHUNKED NICHT ERLAUBT IST
    {
        cout << "invalid request line!" << endl;
        Response errRes = sendErrorPage(cfg, 400);
		string responseStr = buildResponseString("", errRes);
		send(clientFd, responseStr.c_str(), responseStr.size(), 0);
        close(clientFd);
        return 1;
    }
	if (contentLen > 0 && !bodyComplete(*buffer))
	{
		cout << "header completed, body incomplete..." << endl;
		return 1;
	}
    cout << "received full and valid HTTP request!" << endl;
    return 0;
}