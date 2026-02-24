/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hreusing <hreusing@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 16:55:53 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/12 12:24:05 by hreusing         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/HttpMethods.hpp"
#include "../../include/Utils.hpp"

#include <fstream>
#include <sys/stat.h>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <unistd.h>

Response uploadTextItem(const Request &req, const ServerBlock &cfg, string uploadId)
{
    Response res;

    const Location *loc = getMatchedLocation("/uploads", cfg);
    if (!loc) return sendErrorPage(cfg, 500);

    string root = loc->root;
    string uploadDir = root + "/" + uploadId;
    if (!dirExists(root) || !dirExists(uploadDir))
        return sendErrorPage(cfg, 500);
    string fileName;
    if (req.path.find("/add_to_list.php") != string::npos ||
        req.path == "/uploads" || req.path == "/uploads/") {
        fileName = uploadId + ".txt";
    } 
    else if (req.path.find("/uploads/") == 0) {
        size_t lastSlash = req.path.find_last_of('/');
        fileName = req.path.substr(lastSlash + 1);
    } 
    else {
        return sendErrorPage(cfg, 403);
    }

    string filePath = uploadDir + "/" + fileName;
    string item = req.body;
    size_t pos = item.find('=');
    if (pos != string::npos) {
        item = item.substr(pos + 1);
    }
    item = urlDecode(item);
    std::ofstream out(filePath.c_str(), std::ios::app | std::ios::binary);
    if (!out.is_open())
        return sendErrorPage(cfg, 500);

    out << item << "\n";
    out.close();

    for (size_t i = 0; i < loc->redirects.size(); ++i) {
        const RedirectRule &rd = loc->redirects[i];
        if (req.method == rd.method)
            return doRedirect(rd);
    }

    res.statusCode = 303;
    res.statusText = "See Other";
    res.headers["Location"] = "/index.html";
    res.body = "";
    res.headers["Content-Length"] = "0";
    res.headers["Content-Type"] = "text/html";

    return res;
}

Response uploadFile(const Request &req, const ServerBlock &cfg, string uploadId)
{
    Response res;

    string contentType = req.headers.find("Content-Type")->second;
    string boundary;
    size_t pos = contentType.find("boundary=");
    if (pos != string::npos)
        boundary = contentType.substr(pos + 9);
    else {
		Response errRes = sendErrorPage(cfg, 400);
		return errRes;
    }
    const Location *loc = getMatchedLocation(req.path, cfg);
	string root = loc->root;
    string uploadDir = root + "/" + uploadId;
    string filesDir = uploadDir + "/files";
	if (!dirExists(root))
		return sendErrorPage(cfg, 500);
    if (!dirExists(uploadDir))
        return sendErrorPage(cfg, 500);
    if (!dirExists(filesDir))
        return sendErrorPage(cfg, 500);
    size_t fileStart = req.body.find("filename=\"");
    if (fileStart == string::npos) {
		Response errRes = sendErrorPage(cfg, 400);
		return errRes;
    }
    fileStart += 10;
    size_t fileEnd = req.body.find("\"", fileStart);
    string filename = req.body.substr(fileStart, fileEnd - fileStart);
    string headerEnd = "\r\n\r\n";
    size_t contentStart = req.body.find(headerEnd, fileEnd);
    if (contentStart == string::npos) {
		Response errRes = sendErrorPage(cfg, 400);
		return errRes;
    }
    contentStart += headerEnd.size();
    string endBoundary = "\r\n--" + boundary;
    size_t contentEnd = req.body.find(endBoundary, contentStart);
    if (contentEnd == string::npos)
        contentEnd = req.body.size();
    string fileData = req.body.substr(contentStart, contentEnd - contentStart);
    string filePath = filesDir + "/" + filename;

	if (fileExists(filePath) && access(filePath.c_str(), W_OK) != 0) {
		return sendErrorPage(cfg, 403);
	}
    std::ofstream fileOut(filePath.c_str(), std::ios::binary);
    if (!fileOut.is_open()) {
		Response errRes = sendErrorPage(cfg, 500);
		return errRes;
    }
    else {
        fileOut.write(fileData.data(), fileData.size());
        fileOut.close();
		for (size_t i = 0; i < loc->redirects.size(); ++i)
        {
			const RedirectRule &rd = loc->redirects[i];
			if (req.method == rd.method)
			    return doRedirect(rd);
		}
		res.statusCode = 303;
		res.statusText = "See Other";
		res.headers["Location"] = "/index.html";
		res.body = "";
    }
    std::stringstream ss;
    ss << res.body.size();
    res.headers["Content-Length"] = ss.str();
    res.headers["Content-Type"] = "text/html";
    return res;
}

Response handlePost(const Request &req, const ServerBlock &cfg, string uploadDir)
{
    Response res;

    if (!isMethodAllowed("POST", req.path, cfg)) {
		  Response errRes = sendErrorPage(cfg, 405);
		  return errRes;
    }
    map<string,string>::const_iterator it = req.headers.find("Content-Type");
    string contentType = (it != req.headers.end()) ? it->second : "";
    if (contentType.find("multipart/form-data") != string::npos)
        return uploadFile(req, cfg, uploadDir);
    else
        return uploadTextItem(req, cfg, uploadDir);
}
