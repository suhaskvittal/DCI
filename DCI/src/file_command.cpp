//
//  file_command.cpp
//  DCI
//
//  Created by Suhas Vittal on 5/27/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//


#include "../include/command.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

using namespace std;

static int file_access_state = FILE_ACCESS_OPEN;

int file_command::execute(string* buf_p) {
    /*
     check the cmd type and call the respective function
     */
    cout << "cmd type: " << cmd_type << endl;
    if (cmd_type == "send") { return file_send(buf_p); }
    else if (cmd_type == "get") { return file_get(buf_p); }
    else if (cmd_type == "access") { return file_access(buf_p); }
    else { return 1; }
}

/*
    When examining these commands, it is important to note:
        1) The "client" was actually the TARGET SOCKET in the original command. We call the target socket the client because the target socket is calling these functions.
        2) Some of these commands may seem intrusive, so we give the user the option to not allow them.
 */

int file_command::file_send(string* buf_p) {
    // check if operation is allowed
    if (file_access_state >= FILE_ACCESS_CLOSED) { return 1; }
    
    list<string> args(get_args());
    
    /* args: token(?) target src dst */
    
    /* We also have a indicator argument. Since the file get command bounces from
     the peer to the client, we need to indicate the second call of file send. We do
     this by appending a token to the beginning of the arg list. */
    
    
    
    auto iter = args.begin();
    string token(*iter++);
    
    if (token == EXEC_RESEND_TOKEN) {
        // save file data to indicated file
        iter++;  // skip socket target
        string file_data(move(*iter++));
        string dest(move(*iter++));
        ofstream os(dest);
        if (!os) { *buf_p = FILE_SEND_FAILURE; return 1; }
        os << file_data << "\n";
        os.flush();
        os.close();
        *buf_p = FILE_SEND_SUCCESS;
    } else {
        // get file data from indicated file
        SOCKET target = (SOCKET) stoi(token);
        ifstream is(*iter);
        string data;
        while (is) {
            string content;
            getline(is, content);
            data += content + "\n";
        }
        *iter = data;
        
        command* cpy_cmd = new file_command(cmd_type, args);  // create copy of the command
        if (send_command(target, cpy_cmd, { EXEC_RESEND_TOKEN })) {
            return 1;
        }
        delete cpy_cmd;
    }
    return 0;
}

int file_command::file_get(string* buf_p) {
    // check if the client has allowed this operation
    if (file_access_state >= FILE_ACCESS_SAFE) { return 1; }
    
    list<string> args(get_args());
    
    /*
     First argument should be be source path on client, and second argument should
     be target path on the peer. We will simply send the data back to the peer; as
     buf is sent back to the peer, we will put the file data in there.
     */
    
    /* We also have a indicator argument. Since the file get command bounces from
     the client to the peer, we need to indicate the second call of file get. We do
     this by appending a token to the beginning of the arg list. */
    
    auto iter = args.begin();
    string token(move(*iter++));
    
    if (token == EXEC_RESEND_TOKEN) {
        // then second call on peer side
        string file_data(move(*iter++));
        string dest(move(*iter++));
        
        ofstream os(dest);
        if (!os) { return 1; }
        os << file_data << "\n";
        os.flush(); os.close();
        return 0;
    } else {
        ifstream is(token);
        string data;
        while (is) {  // there is content coming from the input stream
            string content;
            getline(is, content);
            data += content + "\n";
        }
        is.close();
        if (data.empty()) { return 1; }
        return EXEC_RESULT_BOUNCE_FGET;  // 2=call again
    }
}

/* The file access function is called on the callee.*/
int file_command::file_access(string* buf_p) {
    string access_level = get_args().front();  // access level should be only argument

    if (access_level == "open") { file_access_state = FILE_ACCESS_OPEN; }
    else if (access_level == "safe") { file_access_state = FILE_ACCESS_SAFE; }
    else if (access_level == "closed") { file_access_state = FILE_ACCESS_CLOSED; }
    else { return 1; }
    
    *buf_p = "File access was set to " + access_level + ".\n";
    
    return 0;
}
