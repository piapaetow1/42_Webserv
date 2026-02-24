# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/02 13:40:39 by pbohme            #+#    #+#              #
#    Updated: 2026/01/07 15:49:59 by pbohme           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

NAME = webserv

SRC = src/main.cpp \
	src/Socket.cpp \
	src/Client.cpp \
	src/Server.cpp \
	src/ServerUtils.cpp \
	src/RequestHandler.cpp \
	src/ConfigParser.cpp \
	src/ConfigParserUtils.cpp \
	src/ConfigParserDetails.cpp \
	src/UploadDirectories.cpp \
	src/HttpMethods/Delete.cpp \
	src/HttpMethods/Get.cpp \
	src/HttpMethods/GetUtils.cpp \
	src/HttpMethods/Post.cpp \
	src/HttpMethods/Utils.cpp \
	src/HttpMethods/parseRequest.cpp \
 	src/HttpMethods/Cgi.cpp \
	src/HttpMethods/Redirect.cpp \
	
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean:
	rm -f $(NAME) $(OBJ)

re: fclean all 

.PHONY: all clean fclean re
