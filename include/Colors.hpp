/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Colors.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pbohme <pbohme@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/29 17:19:53 by pbohme            #+#    #+#             */
/*   Updated: 2026/01/06 20:17:38 by pbohme           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COLORS_HPP

#pragma once

// RESET
#define RESET "\033[0m"

// Standard foreground
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

// Bright foreground
#define BRIGHT_BLACK   "\033[90m"
#define BRIGHT_RED     "\033[91m"
#define BRIGHT_GREEN   "\033[92m"
#define BRIGHT_YELLOW  "\033[93m"
#define BRIGHT_BLUE    "\033[94m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_CYAN    "\033[96m"
#define BRIGHT_WHITE   "\033[97m"

// Custom descriptive colors (foreground)
#define ORANGE    "\033[38;5;208m"
#define LIME      "\033[38;5;118m"
#define LIGREY    "\033[38;5;244m"
#define PURPLE    "\033[38;5;21m"
#define SEAWEED   "\033[38;5;22m"
#define GRASS     "\033[38;5;34m"
#define TURQUISE  "\033[38;5;51m"
#define DUSKROSE  "\033[38;5;52m"
#define OKER      "\033[38;5;58m"
#define SHREK     "\033[38;5;64m"
#define SUNSET    "\033[38;5;124m"
#define DARKORANGE "\033[38;5;166m"
#define LILAC     "\033[38;5;177m"
#define BUTTERMILK "\033[38;5;229m"
#define SALMON    "\033[38;5;210m"
#define PEACH     "\033[38;5;215m"
#define MINT      "\033[38;5;121m"
#define AQUA      "\033[38;5;51m"
#define VIOLET    "\033[38;5;135m"
#define ROSE      "\033[38;5;204m"
#define COPPER    "\033[38;5;173m"

// Standard background colors
#define BK_BLACK  "\033[40m"
#define BK_RED    "\033[41m"
#define BK_GREEN  "\033[42m"
#define BK_YELLOW "\033[43m"
#define BK_BLUE   "\033[44m"
#define BK_MAGENTA "\033[45m"
#define BK_CYAN   "\033[46m"
#define BK_WHITE  "\033[47m"

// Bright background colors
#define BK_BRIGHT_BLACK   "\033[100m"
#define BK_BRIGHT_RED     "\033[101m"
#define BK_BRIGHT_GREEN   "\033[102m"
#define BK_BRIGHT_YELLOW  "\033[103m"
#define BK_BRIGHT_BLUE    "\033[104m"
#define BK_BRIGHT_MAGENTA "\033[105m"
#define BK_BRIGHT_CYAN    "\033[106m"
#define BK_BRIGHT_WHITE   "\033[107m"

// Custom descriptive backgrounds
#define BK_ORANGE    "\033[48;5;208m"
#define BK_LIME      "\033[48;5;118m"
#define BK_LIGREY    "\033[48;5;244m"
#define BK_PURPLE    "\033[48;5;21m"
#define BK_SEAWEED   "\033[48;5;22m"
#define BK_GRASS     "\033[48;5;34m"
#define BK_TURQUISE  "\033[48;5;51m"
#define BK_DUSKROSE  "\033[48;5;52m"
#define BK_OKER      "\033[48;5;58m"
#define BK_SHREK     "\033[48;5;64m"
#define BK_SUNSET    "\033[48;5;124m"
#define BK_DARKORANGE "\033[48;5;166m"
#define BK_LILAC     "\033[48;5;177m"
#define BK_BUTTERMILK "\033[48;5;229m"
#define BK_SALMON    "\033[48;5;210m"
#define BK_PEACH     "\033[48;5;215m"
#define BK_MINT      "\033[48;5;121m"
#define BK_AQUA      "\033[48;5;51m"
#define BK_VIOLET    "\033[48;5;135m"
#define BK_ROSE      "\033[48;5;204m"
#define BK_COPPER    "\033[48;5;173m"

// Text styles
#define BOLD      "\033[1m"
#define DIM       "\033[2m"
#define ITALIC    "\033[3m"
#define UNDERLINE "\033[4m"
#define BLINK     "\033[5m"   // not supported everywhere
#define REVERSE   "\033[7m"
#define HIDDEN    "\033[8m"
#define STRIKETHROUGH "\033[9m"

// Utility macros
#define FGBG(fg,bg) fg bg
#define BOLDRED    BOLD RED
#define BOLDGREEN  BOLD GREEN
#define UNDERLINEBLUE UNDERLINE BLUE

// Quick 256-color macros
#define FG256(n) "\033[38;5;" #n "m"
#define BG256(n) "\033[48;5;" #n "m"

#endif