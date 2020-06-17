//
//  msg_command.cpp
//  DCI
//
//  Created by Suhas Vittal on 6/13/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/msg_command.h"

#include <list>
#include <string>

using namespace std;

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
    
    *buf_p = "TO [" + to_string(target) + "]: " + concat_msg;
    return 0;
}
