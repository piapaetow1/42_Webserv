/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 20:13:00 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/08 13:41:34 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "UseNamespace.hpp"
#include "ConfigParser.hpp"
#include "HttpStructs.hpp"


string	getMimeType(const string &path);
string	trim(const string &s);
string	buildResponseString(string reqVersion, const Response &res, const string &filePath = "");
string	urlDecode(const string &s);

bool	isCgiRequest(const string& uri, const ServerBlock& server);
bool	isMethodAllowed(const string &method, const string &requestPath, const ServerBlock &cfg);
bool	dirExists(const string &path);
bool	fileExists(const string &path);
bool	isDirectory(const string &p);
bool	isRegularFile(const string &p);

const Location*	getMatchedLocation(const string& uri, const ServerBlock& server);

ListenAdress	getListenAddrForClientFd(int clientFd);

#endif