/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Redirect.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 16:26:25 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/08 14:41:46 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/HttpMethods.hpp"
#include "../../include/ConfigParser.hpp"
#include "../../include/Server.hpp"

Response doRedirect(const RedirectRule &rd)
{
	Response res;
	res.statusCode = rd.statusCode;
	res.statusText = "See Other";
	res.headers["Location"] = rd.target;
	res.body = "";
	return res;
}