/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpMethods.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 13:21:32 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/08 13:08:14 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPMETHODS_HPP
#define HTTPMETHODS_HPP

#include "ConfigParser.hpp"
#include "HttpStructs.hpp"

using std::string;
using std::cout;
using std::endl;

enum DirResult {
	DIR_CONTINUE,
	DIR_DONE
};

Request		parseRequest(const string& buffer);

Response	handleGet(const Request &req, const ServerBlock &cfg, string uploadId);
Response	handlePost(const Request &req, const ServerBlock &cfg, string uploadDir);
Response	handleDelete(const Request &req, const ServerBlock &cfg, string uploadDir);
Response	handleCGI(const Request &req, const ServerBlock &cfg);
Response	sendErrorPage(const ServerBlock &cfg, int errorCode);
Response	doRedirect(const RedirectRule &rd);

Response	ResUploadId(string uploadId);
Response	GetJsonList(const Request &req, const ServerBlock &cfg, string uploadId);

#endif