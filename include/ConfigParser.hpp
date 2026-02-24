#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "UseNamespace.hpp"

struct RedirectRule {
	string method;
	int statusCode;
	string target;
};

struct ListenAdress {
	std::string ip;
	int port;
	std::string uploadID;
};

struct Location{
	string path;
	string root;
	vector<string> allowedMethods;
	bool autoindex;
	vector<string> cgiExtensions;
	map<std::string, string> cgiInterpreterMap;
	vector<RedirectRule> redirects;
};

struct ServerBlock {
	std::vector <ListenAdress> listens;
	string name;
	string root;
	string indexFile;
	string uploadDir;
	map<int, string> errorPages;
	long maxBodySize;
	vector<Location> locations;
	bool hasListen;
	bool hasRoot;
};

enum ServerParseResult {
    SERVER_HANDLED,
    SERVER_END,
    SERVER_UNKNOWN
};

vector<ServerBlock>	parse(const string &filename);
void				parseServerBlock(std::ifstream &file, ServerBlock &sb);

void				checkSubDir(const string path);
void				checkDuplicateIpPorts(const vector<ServerBlock> servers);
void				setUploadIds(vector<ServerBlock>& sb);
void				finalizeServerBlock(ServerBlock sb);
bool				isServerBlockStart(string line);
bool				parseLocationDirective(Location &currentLoc, ServerBlock &sb, string &key, std::istringstream &iss);
ServerParseResult	parseServerDirective(ServerBlock &sb, string &key, std::istringstream &iss);
string				clean(const string &s);
string				buildUploadID(size_t listenIndex, size_t socketIndex, int port);
ListenAdress		parseListenToken(const string &token);

#endif