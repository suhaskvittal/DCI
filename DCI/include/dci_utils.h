//
//  dci_utils.h
//  DCI
//
//  Created by Suhas Vittal on 6/16/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef dci_utils_h
#define dci_utils_h

#define SYSTEM_MSG_COLOR "\x1b[93m"
#define CLIENT_MSG_COLOR "\x1b[94m"
#define PEER_MSG_COLOR "\x1b[92m"
#define ERROR_MSG_COLOR "\x1b[91m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define COLOR(c, s) c + s + ANSI_COLOR_RESET

char* format_string(char* orig);

#include <string>

std::string color_system(std::string s);
std::string color_client(std::string s);
std::string color_peer(std::string s);
std::string color_error(std::string s);

void trim(std::string* s_p);

#endif /* dci_utils_h */
