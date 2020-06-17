//
//  auth_command.cpp
//  DCI
//
//  Created by Suhas Vittal on 6/13/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/auth_command.h"

#include <string>
#include <unordered_set>

using namespace std;

unordered_set<string> auth_required = {  // a list of commands that require authentication
    "system",
    "file"
};
unordered_set<SOCKET> auth_given = { client };  // a list of sockets that have provided authentication; the client's instance provides authentication to itself, naturally

int auth_command::execute(string* buf_p) {
    /*
    auth command should have 1 argument: the socket to authenticate
     */
    SOCKET auth_socket = (SOCKET) stoi(get_args().front());
    *buf_p = "Socket " + to_string(auth_socket) + " has been authenticated.\n";
    return b_send(auth_socket, (char*) REQ_AUTH_YES_SENT) < 0;
}

int block_command::execute(string* buf_p) {
    SOCKET block_socket = (SOCKET) stoi(get_args().front());
    *buf_p = "Socket " + to_string(block_socket) + " no longer has authentication.\n";
    return b_send(block_socket, (char*) REQ_AUTH_NO_SENT) < 0;
}


void give_auth(SOCKET socket) { auth_given.insert(socket); }
void remove_auth(SOCKET socket) { auth_given.erase(socket); }
