//
//  command.cpp
//  DCI
//
//  Created by Suhas Vittal on 5/24/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/command.h"

#include <iostream>
#include <vector>

using namespace std;

static vector<string> help_cmd_list = {
    HELP_CMD_HELP,
    AUTH_CMD_HELP,
    FILE_CMD_HELP,
    MSG_CMD_HELP,
    SYSTEM_CMD_HELP
};


int help_command::execute(string* buf_p) {
    if (get_args().size() == 0) {
        for (int i = 0; i < help_cmd_list.size(); i++) { cout << "\t" << help_cmd_list[i] << endl; }
    } else {
        string cmd_request = get_args().front();
        
        if (cmd_request == "network") {
            /* Print out sockets in network_sll */
            auto nsll_iter = network_sll.begin();
            while (nsll_iter != network_sll.end()) {
                struct node network_node = *nsll_iter;
                
                if (network_node.socket == client) {
                    cout << "\tSocket " << network_node.socket << " = YOU" << endl;
                } else {
                    cout << "\tSocket " << network_node.socket << " = " << network_node.ipv4_host << ":" << network_node.ipv4_port << " (IPv4:Port)" << endl;
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
    *buf_p = "\tSocket " + to_string(auth_socket) + " has been authenticated.\n";
    return b_send(auth_socket, (char*) REQ_AUTH_YES_SENT) < 0;
}

int block_command::execute(string* buf_p) {
    SOCKET block_socket = (SOCKET) stoi(get_args().front());
    *buf_p = "\tSocket " + to_string(block_socket) + " no longer has authentication.\n";
    return b_send(block_socket, (char*) REQ_AUTH_NO_SENT) < 0;
}

int msg_command::execute(string* buf_p) {
    /*
     msg command should have first argument as the target socket, and the 2..n arguments as the messages to send.
     */
    
    list<string> args(get_args());
    auto arg_i = args.begin();
    SOCKET target = (SOCKET) stoi(*arg_i); arg_i++;
    string concat_msg;
    if (b_send(target, (char*) REQ_TXT_SENT) < 0) { // indicate to the target, we are sending text
        *buf_p = MSG_SENT_FAILURE;
        return 1;
    }
    while (arg_i != args.end()) {
        string msg = *arg_i;
        concat_msg += msg + " ";
        arg_i++;
    }
    b_send(target, (char*) concat_msg.c_str());
    
    *buf_p = "[YOU]: " + concat_msg;
    return 0;
}

int invalid_command::execute(string* buf_p) {
    *buf_p = "\tInvalid command. Please try again.";
    return 0;
}

command* create_command(string input, SOCKET* socket_p) {
    /* all command inputs come in format: <cmd_name> <socket> <arg1> <arg2> ... <argn> */
    
    list<string> cmd_args;
    process_input(input, &cmd_args);
    command* cmd = build_command(cmd_args, socket_p);
    if (!cmd->is_valid()) {
        delete cmd;  // make room for a new instance
        cmd = new invalid_command();
        *socket_p = client;  // execute the output of cmd on the client's instance
    }
    
    return cmd;
}
