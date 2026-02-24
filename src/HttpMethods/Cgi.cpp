/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hreusing <hreusing@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 16:57:18 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/12 12:40:50 by hreusing         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/HttpMethods.hpp"
#include "../../include/Utils.hpp"

#include <cstdio>
#include <sstream>
#include <cstdlib> 
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <string.h>

#define CGI_TIMEOUT 5

string getCgiInterpreter(const Location *loc, const string &ext)
{    
	return loc->cgiInterpreterMap.find(ext) != loc->cgiInterpreterMap.end()
        ? loc->cgiInterpreterMap.find(ext)->second
        : "";
}
static void freeStuff(char *envp[10])
{
	for (int i = 0; i < 10; i++)
	{
		if (envp[i])
			free(envp[i]);
	}
	//free (envp);
}


Response handleCGI(const Request &req, const ServerBlock &cfg)
{    
	Response res;
	string fullscriptPath = cfg.root + req.path;

	const Location *loc = getMatchedLocation(req.path, cfg);
	size_t dot = req.path.rfind('.');
	string extension = (dot != string::npos) ? req.path.substr(dot) : "";
	string interpreter = getCgiInterpreter(loc, extension);
	char *envp[10];
	std::stringstream cl;
	cl << req.body.length();

	envp[0] = strdup((string("REQUEST_METHOD=") + req.method).c_str());
	envp[1] = strdup((string("CONTENT_LENGTH=") + cl.str()).c_str());
	envp[2] = strdup("CONTENT_TYPE=application/x-www-form-urlencoded");
	envp[3] = strdup((string("SCRIPT_NAME=") + req.path).c_str());
	envp[4] = strdup((string("SCRIPT_FILENAME=") + fullscriptPath).c_str());
	envp[5] = strdup("REDIRECT_STATUS=200");
	envp[6] = strdup("GATEWAY_INTERFACE=CGI/1.1");
	envp[7] = strdup("SERVER_PROTOCOL=HTTP/1.1");
	envp[8] = strdup(("PATH=/usr/bin:/bin:/usr/local/bin"));
	envp[9] = NULL;

	int inPipe[2];
    int outPipe[2];

    if (pipe(inPipe) == -1 || pipe(outPipe) == -1)
    {
        Response errRes = sendErrorPage(cfg, 500);
		freeStuff(envp);
		return errRes;
    }
	if (interpreter.empty() || access(interpreter.c_str(), X_OK) != 0)
   	{	
		freeStuff(envp);
		return sendErrorPage(cfg, 500);
	}
    pid_t pid = fork();
    if (pid < 0)
    {
		Response errRes = sendErrorPage(cfg, 500);
		freeStuff(envp);
        return errRes;
    }
    if (pid == 0)
    {
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);

        close(inPipe[1]);
        close(outPipe[0]);

        char *argv[3];
		argv[0] = const_cast<char *>(interpreter.c_str());
		argv[1] = const_cast<char *>(fullscriptPath.c_str());
		argv[2] = NULL;

		if (access(interpreter.c_str(), X_OK) != 0)
			perror("Interpreter not executable");
		if (access(fullscriptPath.c_str(), R_OK) != 0)
			perror("Script not readable");
		execve(interpreter.c_str(), argv, envp);
		exit(1); 
    }
    else
    {
        close(inPipe[0]);
		close(outPipe[1]);
		write(inPipe[1], req.body.c_str(), req.body.size());
		close(inPipe[1]);
		fcntl(outPipe[0], F_SETFL, O_NONBLOCK);

		std::stringstream output;
		char buffer[4096];
		time_t start = time(NULL);
		int status;

		while (true)
		{
			ssize_t bytesRead = read(outPipe[0], buffer, sizeof(buffer));
			if (bytesRead > 0)
				output.write(buffer, bytesRead);
			pid_t result = waitpid(pid, &status, WNOHANG);
			if (result == pid)
				break;
			if (difftime(time(NULL), start) > CGI_TIMEOUT)
			{
				kill(pid, SIGKILL);
				waitpid(pid, &status, 0);
				close(outPipe[0]);
				freeStuff(envp);
				return sendErrorPage(cfg, 504);
			}
			usleep(10000);
		}
		close(outPipe[0]);

        string raw = output.str();
        size_t headerEnd = raw.find("\r\n\r\n");
        if (headerEnd == string::npos)
            headerEnd = raw.find("\n\n");
        string headerPart, bodyPart;
        if (headerEnd != string::npos)
        {
            headerPart = raw.substr(0, headerEnd);
            bodyPart   = raw.substr(headerEnd + 4);
        }
        else
            bodyPart = raw;
        string contentType = "text/html";
        istringstream hs(headerPart);
        string line;
        while (std::getline(hs, line))
        {
            if (line.find("Content-Type:") == 0)
            {
                contentType = line.substr(13);
                while (!contentType.empty() && isspace(contentType[0]))
                    contentType.erase(0, 1);
                break;
            }
        }
        res.statusCode = 200;
        res.statusText = "OK";
        res.body = bodyPart;
        res.headers["Content-Type"] = contentType;
        std::ostringstream len;
        len << res.body.size();
        res.headers["Content-Length"] = len.str();
    }
	freeStuff(envp);
    return res;
}
