//
//  command_factory.cpp
//  DCI
//
//  Created by Suhas Vittal on 5/28/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/command.h"

using namespace std;

#include <fstream>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>

extern unordered_map<SOCKET, string> dir;

typedef function<command*(list<string>)> init_f;
unordered_map<string, init_f> initializer_map = {
    { "system", [](list<string> args) { return new system_command(args); } },
    { "help", [](list<string> args) { return new help_command(args); } },
    { "auth", [](list<string> args) { return new auth_command(args); } },
    { "block", [](list<string> args) { return new block_command(args); } },
    { "msg", [](list<string> args) { return new msg_command(args); } },
    { "file", [](list<string> args) -> command* {
        string fcmd_t = args.back();  // don't remove, this will make cmd sending easier
        string client_dir;
        if (dir.count(client)) {
            client_dir = string(dir[client]);
        }
        
        string peer_dir;
        SOCKET peer;
        
        try {
            if (fcmd_t == "send") {
                peer = (SOCKET) stoi(args.front());
            } else {
                peer = (SOCKET) stoi(args.front());
                args.pop_front();
            }
            if (dir.count(peer)) {
                peer_dir = dir[peer];
            }
            return new file_command(fcmd_t, args, client_dir, peer_dir);
        } catch (...) {
            // return invalid command due to bad cast
            return new invalid_command();
        }
    } }
};

static unordered_set<string> target_is_argument = {
    "system",  // highest type level that requires the target socket to be the argument as well
    "get"
};

static unordered_set<string> client_only = {
    // highest type level that indicates client only
    "help",
    "auth",
    "block",
    "msg",
    "send",     // file sub type
    "access"    // file sub type
};

static unordered_set<string> has_sub_type = {
    "file"
};

command* build_command(list<string> args, SOCKET* target_p) {
    /* We want to build a flexible system that can build complex commands. As
      a result, we will use the most recent sub type to parse commands. */
    queue<string> type_q;
    int is_client_only = 0;
    int is_target_is_argument = 0;
    do {
        string typ = args.front(); args.pop_front();
        is_client_only = is_client_only | (client_only.count(typ) > 0);
        is_target_is_argument = is_target_is_argument | (target_is_argument.count(typ) > 0);
        type_q.push(typ);  // keep pushing back the subtypes to the type queue
    } while (has_sub_type.count(type_q.back()));
    
    /* Reformat the args. Declare the first element of the queue as the cmd_name, and push the rest of the elements to the back of args. */
    string cmd_name(move(type_q.front())); type_q.pop();
    while (type_q.size() > 0) {
        args.push_back(type_q.front()); type_q.pop();
    }

    /* args now looks like:
        <arg1> <arg2> ... <argN> <typK> <typ(K-1)> ... <typ1>
       arg1 may or may not be the target socket, depending on the command.
     */
    
    if (!initializer_map.count(cmd_name)) {
        // return invalid command, set target socket to client
        *target_p = client;
        return new invalid_command();
    } else if (is_client_only) {  // really not much else to do
        *target_p = client;
        return initializer_map[cmd_name](args);
    } else {
        try {
            *target_p = (SOCKET) stoi(args.front());
            if (!is_target_is_argument) {
                args.pop_front();
            }
            return initializer_map[cmd_name](args);
        } catch (...) {
            // return invalid argument due to invalid cast
            return new invalid_command();
        }
    }
}
