/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parseRequest.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hreusing <hreusing@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 18:22:53 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/12 11:41:34 by hreusing         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/HttpMethods.hpp"
#include "../../include/Utils.hpp"

#include <sstream>
#include <sys/stat.h>

Request parseRequest(const string &buffer)
{
    Request req;

    size_t headerEnd = buffer.find("\r\n\r\n");
    bool crlf = true;
    if (headerEnd == string::npos) {
		headerEnd = buffer.find("\n\n");
        crlf = false;
    }
	if (headerEnd == string::npos)
        throw std::runtime_error("Malformed HTTP request (no header/body separator)");
    string headerPart = buffer.substr(0, headerEnd);
    size_t bodyStart = headerEnd + (crlf ? 4 : 2);
    size_t lineEnd = headerPart.find("\r\n");
    if (lineEnd == string::npos)
        lineEnd = headerPart.find("\n");
    if (lineEnd == string::npos)
        lineEnd = headerPart.size();
    string requestLine = headerPart.substr(0, lineEnd);
    istringstream rl(requestLine);
    rl >> req.method >> req.path >> req.version;
    if (req.method.empty() || req.path.empty() || req.version.empty())
        throw std::runtime_error("Invalid request line");
    string delim = (headerPart.find("\r\n") != string::npos) ? "\r\n" : "\n";
    size_t pos = lineEnd + delim.size();
    while (pos < headerPart.size()) {
        size_t next = headerPart.find(delim, pos);
        string line = headerPart.substr(pos, next - pos);
        pos = next + delim.size();
        if (line.empty()) {
            break;
        }
        size_t colon = line.find(':');
        if (colon == string::npos || colon == 0) {
    		throw std::runtime_error("Malformed HTTP header");
		}
        string key = trim(line.substr(0, colon));
        string value = trim(line.substr(colon + 1));
        req.headers[key] = value;
        if (next == string::npos) {
            break;
        }
    }
    if (bodyStart < buffer.size())
        req.body.assign(buffer.begin() + bodyStart, buffer.end());
    else
        req.body.clear();
    return req;
}
