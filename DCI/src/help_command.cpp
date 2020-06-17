//
//  help_command.cpp
//  DCI
//
//  Created by Suhas Vittal on 6/13/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/help_command.h"

#include <string>
#include <vector>

using namespace std;

static vector<string> help_cmd_list = {
    HELP_CMD_HELP,
    AUTH_CMD_HELP,
    ALIAS_CMD_HELP,
    BLOCK_CMD_HELP,
    FILE_CMD_HELP,
    MSG_CMD_HELP,
    SYSTEM_CMD_HELP,
    UNALIAS_CMD_HELP
};


int help_command::execute(string* buf_p) {
    if (get_args().size() == 0) {
        for (int i = 0; i < help_cmd_list.size(); i++) {
            buf_p->append(help_cmd_list[i]);
        }
    } else {
        string cmd_request = get_args().front();
        
        if (cmd_request == "network") {
            /* Print out sockets in network_sll */
            auto nsll_iter = network_sll.begin();
            while (nsll_iter != network_sll.end()) {
                struct node* network_node = *nsll_iter;
                
                if (nsll_iter != network_sll.begin()) { buf_p->push_back('\n'); }
                buf_p->append("Socket " + to_string(network_node->socket) + " = ");
                if (network_node->socket == client) {
                    buf_p->append("YOU");
                } else {
                    buf_p->append(network_node->ipv4_host + ":" + network_node->ipv4_port + " (IPv4:Port)");
                }
                nsll_iter++;
            }
        }
    }
    
    return 0;
}
