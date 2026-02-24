/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetUtils.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 20:59:43 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/08 13:08:10 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/HttpMethods.hpp"
#include "../../include/Utils.hpp"

#include <sstream>
#include <dirent.h>

std::string makeJsonUploadId(const string &uploadId)
{
    std::ostringstream ss;
    ss << "{\"uploadId\":\"" << uploadId << "\"}";
    return ss.str();
}

Response ResUploadId(string uploadId) 
{
	Response res;

	res.statusCode = 200;
	res.statusText = "OK";
	res.body = makeJsonUploadId(uploadId);
	std::ostringstream len;
	len << res.body.size();
	res.headers["Content-Length"] = len.str();
	res.headers["Content-Type"] = "application/json";
	return res;
}

Response GetJsonList(const Request &req, const ServerBlock &cfg, string uploadId) 
{
	Response res;
	
	const Location *loc = getMatchedLocation(req.path, cfg);
	string LocRoot = loc->root;
	string uploadDir = LocRoot + "/" + uploadId;
	string filesDir = uploadDir + "/files";
	DIR *dir = opendir(filesDir.c_str());
	if (!dir)
	{
		res.statusCode = 200;
		res.statusText = "OK";
		res.body = "[]";
	}
	else
	{
		vector<string> files;
		struct dirent *entry;
		while ((entry = readdir(dir)))
		{
			string name = entry->d_name;
			if (name != "." && name != "..")
				files.push_back(name);
		}
		closedir(dir);
		std::ostringstream oss;
		oss << "[";
		for (size_t i = 0; i < files.size(); ++i)
		{
			oss << "\"" << files[i] << "\"";
			if (i + 1 < files.size()) oss << ",";
		}
		oss << "]";
		res.statusCode = 200;
		res.statusText = "OK";
		res.body = oss.str();
	}
	std::ostringstream len;
	len << res.body.size();
	res.headers["Content-Length"] = len.str();
	res.headers["Content-Type"] = "application/json";
	return res;	
}
