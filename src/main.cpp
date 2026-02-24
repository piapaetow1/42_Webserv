#include "../include/Server.hpp"
#include "../include/Colors.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <iostream>

string check_extension(const string filename)
{
	size_t pos = filename.rfind('.');
	if (pos == string::npos)
		return ("");
	return filename.substr(pos);
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 2) {
        cerr << BOLD << RED << "Usage: " << argv[0] << " <config file>" << RESET << endl;
        return 1;
    }
	string ext = check_extension(argv[1]);
	if (ext != ".conf") {
		cerr << BOLD << RED << "Invalid file extension. Required: " << GREEN << ".conf" << RESET << endl;
		return 1;
	}
    vector<ServerBlock> servers;
    try {
        servers = parse(argv[1]);
        if (servers.empty())
		{
            cerr << BOLD << RED << "Invalid config file: no server blocks found." << RESET << endl;
            return 1;
        }
    } catch (const std::exception &e) {
        cerr << RED << "Error parsing config file: " << e.what() << RESET << endl;
        return 1;
    }
	try {
		Server server(servers);
		server.init();
		server.run();
	} catch (const std::exception &e) {
		cerr << RED << "Fatal: " << e.what() << endl; 
		return 1;
	}
	return 0;
}
