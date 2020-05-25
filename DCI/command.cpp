//
//  parser.cpp
//  DCI
//
//  Created by Suhas Vittal on 5/24/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "command.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;

typedef function<command*(list<string>)> init_f;
static unordered_map<string, init_f> initializer_map = {
    { "system", [](list<string> args) { return new system_command(args); } },
    { "help", [](list<string> args) { return new help_command(args); } },
    { "auth", [](list<string> args) { return new auth_command(args); } },
    { "msg", [](list<string> args) { return new msg_command(args); } }
};

static unordered_set<string> auth_required = {  // a list of commands that require authentication
    "system"
};

static unordered_set<SOCKET> auth_given = { client };  // a list of sockets that have provided authentication; the client's instance provides authentication to itself, naturally


int help_command::execute(string* buf_p) {
    if (get_args().size() == 0) {
        vector<string> help_list = HELP_CMD_LIST;
        for (int i = 0; i < help_list.size(); i++) { printf("\t%s", help_list[i].c_str()); }
    } else {
        string cmd_request = get_args().front();
        
        if (cmd_request == "network") {
            /* Print out sockets in network_sll */
            auto nsll_iter = network_sll.begin();
            while (nsll_iter != network_sll.end()) {
                struct node network_node = *nsll_iter;
                
                if (network_node.socket == client) {
                    printf("\tSocket %d = YOU\n", network_node.socket);
                } else {
                    printf("\tSocket %d = %s:%s (IPv4:Port)\n", network_node.socket,
                           network_node.ipv4_host.c_str(), network_node.ipv4_port.c_str());
                }
                nsll_iter++;
            }
        }
    }
    
    return 0;
}

int auth_command::execute(string* buf_p) {
    /*
    auth command should have 1 argument: the socket to authenticate
     */
    SOCKET auth_socket = (SOCKET) stoi(get_args().front());
    *buf_p = "Socket " + to_string(auth_socket) + " has been given authenticated.\n";
    return b_send(auth_socket, (char*) REQ_AUTH_SENT) < 0;
}

int msg_command::execute(string* buf_p) {
    /*
     msg command should have first argument as the target socket, and the 2..n arguments as the messages to send.
     */
    list<string> args(get_args());
    auto arg_i = args.begin();
    SOCKET target = (SOCKET) stoi(*arg_i); arg_i++;
    string concat_msg;
    b_send(target, (char*) REQ_TXT_SENT);  // indicate to the target, we are sending text
    while (arg_i != args.end()) {
        string msg = *arg_i;
        concat_msg += msg + "\n";
        arg_i++;
    }
    b_send(target, (char*) concat_msg.c_str());
    
    *buf_p = MSG_SENT_SUCCESS;
    return 0;
}

int system_command::execute(string* buf_p) {
    /*
     system command should have 1 argument: the command itself.
     */
    string sys_c = get_args().front();
    FILE* pipe = popen(sys_c.c_str(), "r");
    if (pipe == nullptr) { return 1; }
    
    try {
        char c;
        while ((c=fgetc(pipe)) != EOF) {
            buf_p->push_back(c);
        }
    } catch (...) {
        pclose(pipe);
        return 1;
    }
    buf_p->push_back('\n');
    pclose(pipe);
    return 0;
}

int invalid_command::execute(string* buf_p) {
    printf("Invalid command. Please try again.\n");
    return 0;
}

int parse_and_send(char* input) {
    SOCKET target;
    command* cmd = to_command(string(input), &target);
    
    if (target == client) {
        string buf;
        if (cmd->execute(&buf)) {
            fprintf(stderr, "Command execution failed.");
            return 1;
        }
        printf("\t%s", buf.c_str());
    } else {
        // indicate that outgoing message is a command
        b_send(target, (char*) REQ_CMD_SENT);
        // send number of incoming messages to target (args.size() + 1) (cmd_name is first message)
        i_send(target, (int) (cmd->get_args().size() + 1));
        if (b_send(target, (char*) cmd->get_name().c_str()) < 0) { return 1; }
        
        auto arg_iter = cmd->get_args().begin();
        while (arg_iter != cmd->get_args().end()) {
            string arg = *arg_iter;
            if (b_send(target, (char*) arg.c_str()) < 0) { return 1; }
            arg_iter++;
        }
    }
    
    delete cmd;
    return 0;
}

int recv_and_parse(SOCKET from, char** buf_p) {
    int n_args;
    i_recv(from, &n_args);
    char* cmd_name_buf;
    if (b_recv(from, &cmd_name_buf) < 0) { return 1; }
    n_args--;
    
    list<string> cmd_args;
    while (n_args > 0) {
        char* arg_buf;
        if (b_recv(from, &arg_buf) < 0) { return 1; }
        
        cmd_args.push_back(string(arg_buf));
        n_args--;
        free(arg_buf);
    }
    command* cmd = initializer_map[string(cmd_name_buf)](cmd_args);
    
    string buf;
    if (cmd->execute(&buf)) {
        fprintf(stderr, "Command execution failed.");
        return 1;
    }
    *buf_p = (char*) buf.c_str();
    
    delete cmd;
    free(cmd_name_buf);
    return 0;
}

command* to_command(string input, SOCKET* socket_p) {
    /* all command inputs come in format: <cmd_name> <socket> <arg1> <arg2> ... <argn> */
    
    list<string> cmd_args;
    process_input(input, &cmd_args);
    // get cmd name and the socket number
    string cmd_name = cmd_args.front(); cmd_args.pop_front();
    
    if (cmd_name == "help" || cmd_name == "auth" || cmd_name == "msg") {  // can only be used on client
        *socket_p = client;
    } else {
        *socket_p = (SOCKET) stoi(cmd_args.front()); cmd_args.pop_front();
    }
    
    command* cmd;
    if (initializer_map.find(cmd_name) != initializer_map.end() && (!auth_required.count(cmd_name) || auth_given.count(*socket_p))) {
        cmd = initializer_map[cmd_name](cmd_args);
    } else {
        cmd = new invalid_command();
    }
    
    if (!cmd->is_valid()) {
        cmd = new invalid_command();
        *socket_p = client;  // execute the output of cmd on the client's instance
    }
    
    return cmd;
}

void process_input(string input, list<string>* args) {
    string buf;
    
    int imd_raw = 0;  // escapes the next character
    int in_quote = 0;  // if inquote, ignore delimiter
    for (int i = 0; i < input.length(); i++) {
        char c = input[i];
        // check the character
        if (imd_raw) { buf.push_back(c); imd_raw = 0; }
        else if (c == '\"') { in_quote = !in_quote; }
        else if (c == '\\') { imd_raw = 1; }
        else if (IS_DELIMITER(c) && !in_quote) {
            // dump buf to sll
            string arg(std::move(buf));
            args->push_back(arg);
            buf.clear();
        } else {
            buf.push_back(c);
        }
    }
    
    if (buf.length() > 0) { args->push_back(buf); }  // dump the remaining buf
}

void auth_provided(SOCKET socket) { auth_given.insert(socket); }
