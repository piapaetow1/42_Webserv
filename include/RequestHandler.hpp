/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 12:22:26 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/07 12:51:36 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "UseNamespace.hpp"
#include "ConfigParser.hpp"

bool	headerComplete(const string& buffer);
bool	requestLineValid(const string& buffer);
bool	bodyComplete(const string& buffer);
size_t	getContentLength(const string& buffer);
int		checkRequestValid(const string* buffer, const ServerBlock& cfg, long long contentLen, int clientFd);

#endif