/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStructs.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 17:13:50 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/06 20:38:09 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSTRUCTS_HPP
#define HTTPSTRUCTS_HPP

#include "UseNamespace.hpp"

struct Request {
	string method;
	string path;
	string version;
	map<string, string> headers;
	string body;
};

struct Response {
	int statusCode;
	string statusText;
	map<string, string> headers;
	string body;
};

#endif