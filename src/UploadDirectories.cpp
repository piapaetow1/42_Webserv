/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadDirectories.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 13:27:03 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/09 13:13:11 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/UploadDirectories.hpp"
#include "../include/HttpMethods.hpp"
#include "../include/Utils.hpp"
#include "../include/Colors.hpp"

#include <sys/stat.h>
#include <sstream>
#include <cstring>
#include <fstream>


void buildUploadDir(string uploadRoot, string id) {
	string uploadDir = uploadRoot + "/" + id;
	if (!(mkdir(uploadDir.c_str(), 0755) == 0)) {
		if (errno == EEXIST) {
		} else {
			std::string err = std::string("mkdir failed for '") + uploadDir + "': " + strerror(errno);
			throw std::runtime_error(err);
		}
	}
	string filesDir = uploadDir + "/files";
	if (!(mkdir(filesDir.c_str(), 0755) == 0)) {
		if (errno == EEXIST) {
		} else {
			std::string err = std::string("mkdir failed for '") + filesDir + "': " + strerror(errno);
			throw std::runtime_error(err);
		}
	}
	string filePath = uploadDir + "/" + id + ".txt";
	std::ofstream out(filePath.c_str(), std::ios::app);
    if (!out.is_open())
    {
		throw std::runtime_error("Failed to create .txt file");
    }
	out.close();
}

static void splitPathTokens(const string& path, std::vector<string>& outTokens) {
    outTokens.clear();
    string token;
    std::istringstream ss(path);
    while (std::getline(ss, token, '/')) {
        if (token.empty() || token == ".") continue;
        outTokens.push_back(token);
    }
}

static void ensurePathExists(const std::string& rawPath) {
    if (rawPath.empty()) return;

    std::string path = rawPath;
    if (path.size() > 1 && path[path.size() - 1] == '/')
        path.erase(path.size() - 1);

    if (path.size() >= 2 && path[0] == '.' && path[1] == '/')
        path = path.substr(2);

    bool isAbsolute = false;
    if (!path.empty() && path[0] == '/') {
        isAbsolute = true;
        if (path.size() > 1)
            path = path.substr(1);
        else
            path = "";
    }

    std::vector<std::string> tokens;
    splitPathTokens(path, tokens);

    std::string cur;
    if (isAbsolute) cur = "/";

    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& tok = tokens[i];
        if (tok == "..") {
            throw std::runtime_error("Parent-directory (..) not allowed in upload path");
        }
        if (!cur.empty() && cur[cur.size()-1] != '/')
            cur += "/";
        cur += tok;
        if (!(mkdir(cur.c_str(), 0755) == 0)) {
            if (errno == EEXIST) {
            } else {
                std::string err = std::string("mkdir failed for '") + cur + "': " + strerror(errno);
                throw std::runtime_error(err);
            }
        }
    }
}

void buildUploadRoot(const std::vector<ServerBlock>& serverBlocks) {
    for (size_t i = 0; i < serverBlocks.size(); ++i) {
        const ServerBlock& sb = serverBlocks[i];

        const Location* loc = getMatchedLocation("/uploads", sb);
        if (!loc) {
            continue;
        }

        const string uploadRoot = loc->root;
        try {
            ensurePathExists(uploadRoot);
			
			for (size_t j = 0; j < sb.listens.size(); j++) {
				string uploadDir = sb.listens[j].uploadID;
				buildUploadDir(uploadRoot, uploadDir);
			}
        } catch (const std::exception& e) {
            cerr << RED << "[buildUploadDir] error for serverBlock " << i << ": " << e.what() << RESET << endl;
            throw;
        }
    }
}
