/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 16:55:42 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/08 13:08:42 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/HttpMethods.hpp"
#include "../../include/Server.hpp"
#include "../../include/Utils.hpp"

#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <sstream>

Response handleGet(const Request &req, const ServerBlock &cfg, string uploadId)
{
    Response res;
    string root = cfg.root;

    if (!root.empty() && root[root.size() - 1] == '/') {
        root.resize(root.size() - 1);
	}
	if (req.path == "/getUploadId") {
		return ResUploadId(uploadId);
	}
    string filePath = root + req.path;
	if (req.path.find("/uploads") == 0 && req.path.find("/files") != string::npos && (req.path.rfind("/files") + 6 == req.path.size()))
		return GetJsonList(req, cfg, uploadId);
	if (req.path.find("/uploads") == 0 && req.path.find(".txt") != string::npos)
	{
		const Location *loc = getMatchedLocation(req.path, cfg);
		string LocRoot = loc->root;
		string uploadDir = LocRoot + "/" + uploadId;
		if (access(LocRoot.c_str(), F_OK) != 0) 
			return sendErrorPage(cfg, 500);	
		if (access(uploadDir.c_str(), F_OK) != 0) {
			if (mkdir(uploadDir.c_str(), 0755) != 0)
				return sendErrorPage(cfg, 500);
		}
		filePath = uploadDir + "/" + uploadId + ".txt";
		if (!fileExists(filePath))
			return sendErrorPage(cfg, 500);
	}
	if (req.path.find("/uploads/" + uploadId + "/files/") == 0)
	{
		const Location *loc = getMatchedLocation("/uploads", cfg);
		if (!loc)
			return sendErrorPage (cfg, 404);
		if (!loc->autoindex)
			return sendErrorPage(cfg, 405);
		string LocRoot = loc->root;
		string uploadDir = LocRoot + "/" + uploadId;
		string filesDir = uploadDir + "/files";
		string filename = req.path.substr(string("/uploads/" + uploadId + "/files/").size());
		filePath = filesDir + "/" + filename;
	}
    if (!fileExists(filePath) && !isDirectory(filePath))
		return (sendErrorPage(cfg, 404));
	if (isDirectory(filePath)) {
		const Location *loc = getMatchedLocation(req.path, cfg);
		if (loc && loc->autoindex)
		{
			DIR *dir = opendir(filePath.c_str());
			if (!dir) {
				return sendErrorPage(cfg, 500);
 			}
				
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
			res.headers["Content-Type"] = "application/json";
			res.body = oss.str();
			std::ostringstream len; len << res.body.size();
			res.headers["Content-Length"] = len.str();
			return res;	
		}
		else
		{
			string indexPath = filePath;
			if (indexPath[indexPath.size() - 1] != '/')
				indexPath += "/";
			indexPath += "index.html";
			if (isRegularFile(indexPath))
				filePath = indexPath;
			else
				return sendErrorPage(cfg, 403);
    	} 
	}
	if (!isRegularFile(filePath))
		return sendErrorPage(cfg, 404);
	if (access(filePath.c_str(), R_OK) != 0)
		return sendErrorPage(cfg, 403);
	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file)
		return sendErrorPage(cfg, 500);
	vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	res.body.assign(buffer.begin(), buffer.end());
	res.statusCode = 200;
	res.statusText = "OK";
	std::ostringstream contentLength;
	contentLength << buffer.size();
	res.headers["Content-Length"] = contentLength.str();
	res.headers["Content-Type"] = getMimeType(filePath);
	return res;
}
