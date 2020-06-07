//
//  system_command.cpp
//  DCI
//
//  Created by Suhas Vittal on 6/1/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/command.h"

#include <iostream>
#include <unordered_map>

using namespace std;

unordered_map<SOCKET, string> dir;

int system_command::execute(string* buf_p) {
    /*
     system command should have 1 argument: the command itself.
     */
    list<string> args(get_args());
    auto arg_iter = args.begin();
    SOCKET target = (SOCKET) stoi(*arg_iter++);
    string cpfx;
    if (dir.count(target)) { cpfx = "cd " + dir[target] + ";"; }
#if defined(_WIN32)
    string cext = ";cd";  // a command extension that prints the working directory after a command is executed
#else
    string cext = ";pwd";
#endif

    string cmd = cpfx + (*arg_iter++) + cext;
    if (call_system(cmd.c_str(), buf_p)) { return 1; }
    // get working directory after command call
    string d;
    char b;
    while (buf_p->length() > 0 && ((b=buf_p->back()) != '\n' || d.length() == 0)) {
        buf_p->pop_back();
        if (b == ' ' || b == '\n' || b == ' ') { continue; }
        d.insert(d.begin(), b);
    }
    dir[target] = d;
    
    return 0;
}

int call_system(string cmd, string* buf_p) {
    FILE* pipe = POPEN(cmd.c_str(), "r");
    if (pipe == nullptr) { return 1; }
    
    try {
        char c;
        while ((c=fgetc(pipe)) != EOF) {
            buf_p->push_back(c);
        }
    } catch (...) {
        PCLOSE(pipe);
        return 1;
    }
    buf_p->push_back('\n');
    PCLOSE(pipe);
    
    return 0;
}
