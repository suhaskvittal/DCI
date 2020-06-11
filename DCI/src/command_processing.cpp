//
//  command_processing.cpp
//  DCI
//
//  Created by Suhas Vittal on 6/1/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/command.h"

#include <iostream>

#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;

extern unordered_map<string, function<command*(list<string>)>> initializer_map;

static unordered_set<string> auth_required = {  // a list of commands that require authentication
    "system",
    "file"
};
static unordered_set<SOCKET> auth_given = { client };  // a list of sockets that have provided authentication; the client's instance provides authentication to itself, naturally

int parse_and_send(char* input) {
    SOCKET target;
    command* cmd = create_command(string(input), &target);
    
    if (target == client) {
        string buf;
        if (cmd->execute(&buf)) {
            cerr << "Command execution failed." << endl;
            return 1;
        }
        char* fbuf = format_string((char*) buf.c_str());
        cout << color_client(fbuf) << endl;
        delete[] fbuf;
    } else {
        if (send_command(target, cmd, {})) {
            cerr << "Command transmit failed. Make sure all preparations, such as authentication, has been completed." << endl;
            return 1;
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
        delete[] arg_buf;
    }
    command* cmd = initializer_map[string(cmd_name_buf)](cmd_args);
    
    string buf;
    int res = cmd->execute(&buf);
    delete[] cmd_name_buf;
    if (res == EXEC_RESULT_FAILURE) {
        cerr << color_error("Command execution failed.") << endl;
        delete cmd;
        return 1;
    } else if (res == EXEC_RESULT_BOUNCE_FGET) {
        int r = send_command(from, cmd, { EXEC_RESEND_TOKEN });
        delete cmd;
        return r;
    } else {
        *buf_p = (char*) buf.c_str();
        delete cmd;
    }
    return 0;
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

void give_auth(SOCKET socket) { auth_given.insert(socket); }
void remove_auth(SOCKET socket) { auth_given.erase(socket); }

int send_command(SOCKET to, command* cmd, vector<string> tokens) {
    // check if user requires and has authentication
    if (auth_required.count(cmd->get_name()) && !auth_given.count(to)) { return 1; }
    
    // indicate that outgoing message is a command
    b_send(to, (char*) REQ_CMD_SENT);
    // send number of incoming messages to target (args.size() + tokens.size() + 1) (cmd_name is first message)
    i_send(to, (int) (cmd->get_args().size() + tokens.size() + 1));
    if (b_send(to, (char*) cmd->get_name().c_str()) < 0) { return 1; }
    for (int i = 0; i < tokens.size(); i++) {
        if (b_send(to, (char*) tokens[i].c_str()) < 0) { return 1; }
    }
    
    list<string> args(cmd->get_args());
    auto arg_iter = args.begin();
    while (arg_iter != args.end()) {
        string arg = *arg_iter;
        if (b_send(to, (char*) arg.c_str()) < 0) { return 1; }
        arg_iter++;
    }
    return 0;
}
