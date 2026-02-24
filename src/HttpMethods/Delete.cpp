/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Delete.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 16:56:12 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/06 20:54:39 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/HttpMethods.hpp"
#include "../../include/Utils.hpp"

#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

static string shortenPath(const string &fullPath, const string &locPath)
{
	if (fullPath.find(locPath) == 0)
		return fullPath.substr(locPath.length());
	return fullPath;
}

Response deleteTextItem(const Request &req, const ServerBlock &cfg, string uploadId)
{
    Response res;
    const Location *loc = getMatchedLocation(req.path, cfg);
    string root = loc->root;
    string uploadDir = root + "/" + uploadId;
	string reqPath = shortenPath(req.path, loc->path);
    string requestedFilePath = root + reqPath;

    if (fileExists(requestedFilePath) && req.body.empty())
    {
        if (unlink(requestedFilePath.c_str()) != 0)
            return sendErrorPage(cfg, 500);
        res.statusCode = 204;
        res.statusText = "No Content";
        res.headers["Content-Length"] = "0";
        return res;
    }

    string txtFilePath = uploadDir + "/" + uploadId + ".txt";
    if (!fileExists(txtFilePath))
        return sendErrorPage(cfg, 500);

    string itemToDelete = req.body;
    if (!itemToDelete.empty() && itemToDelete[itemToDelete.size() - 1] == '\n')
        itemToDelete.erase(itemToDelete.size() - 1);

    std::ifstream inFile(txtFilePath.c_str());
    if (!inFile.is_open())
        return sendErrorPage(cfg, 500);

    vector<string> lines;
    string line;
    while (std::getline(inFile, line))
    {
        if (line != itemToDelete)
            lines.push_back(line);
    }
    inFile.close();

    std::ofstream outFile(txtFilePath.c_str(), std::ios::trunc);
    for (size_t i = 0; i < lines.size(); ++i)
        outFile << lines[i] << "\n";
    outFile.close();

    res.statusCode = 204;
    res.statusText = "No Content";
    res.headers["Content-Length"] = "0";
    return res;
}

Response deleteFile(const Request &req, const ServerBlock &cfg, string uploadId)
{
	Response res;
	
	const Location *loc = getMatchedLocation(req.path, cfg);
	string root = loc->root;
	string filesDir = root + "/" + uploadId + "/files";
	if (req.path.find("/files") != string::npos)
	{
		struct stat st;
		if (stat(filesDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
			return (sendErrorPage(cfg, 404));
		DIR *dir = opendir(filesDir.c_str());
		if (!dir)
			return (sendErrorPage(cfg, 404));
		struct dirent *entry;
		bool anyFailed = false;
		vector<string> removed;
		
		while ((entry = readdir(dir)) != NULL)
		{
			string name = entry->d_name;
			if (name == "." || name == "..")
				continue ;
			string fullPath = filesDir + "/" + name;
			struct stat entryStat;
			if (stat(fullPath.c_str(), &entryStat) == 0)
			{
				if (S_ISDIR(entryStat.st_mode))
				{
					if (rmdir(fullPath.c_str()) != 0)
					{
						closedir(dir);
						anyFailed = true;
						break ;
					}
					else
						removed.push_back(fullPath);
				}
				else
				{
					if (unlink(fullPath.c_str()) != 0)
						anyFailed = true;
					else
						removed.push_back(fullPath);
				}
			}
			else
				cout << "stat failed for " << fullPath << endl;
		}
		closedir(dir);
		if (anyFailed)
			return sendErrorPage(cfg, 500);
	}
	res.statusCode = 204;
	res.statusText = "No Content";
	res.body = "";
	res.headers["Content-Length"] = "0";
	return res;
}

Response handleDelete(const Request &req, const ServerBlock &cfg, string uploadId)
{
	if (!isMethodAllowed("DELETE", req.path, cfg))
	{
		Response errRes = sendErrorPage(cfg, 405);
		return errRes;
	}
	Response res;
	if (req.path.find("/files") != string::npos)
		res = deleteFile(req, cfg, uploadId);
	else 
		res = deleteTextItem(req, cfg, uploadId);
	const Location *loc = getMatchedLocation(req.path, cfg);
    if (loc)
	{
        for (size_t i = 0; i < loc->redirects.size(); ++i)
		{
            const RedirectRule &rd = loc->redirects[i];
            if (req.method == rd.method)
                return doRedirect(rd);
        }
    }
    return res;
}