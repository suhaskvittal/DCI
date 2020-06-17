//
//  dci_utils.cpp
//  DCI
//
//  Created by Suhas Vittal on 6/16/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/dci_utils.h"

#include <vector>

using namespace std;

char* format_string(char* orig) {
    vector<char> f_stack;
    
    vector<char> whitespace_stack;
    int k = 0;
    while (orig[k] != '\0') {
        char c = orig[k++];
        
        if (c == '\n' || c == ' ' || c == '\t') {
            whitespace_stack.push_back(c);
        } else {
            // dump the whitespace stack
            while (whitespace_stack.size() > 0) {
                char w = whitespace_stack.back();
                f_stack.push_back(w);
                
                whitespace_stack.pop_back();
            }
            f_stack.push_back(c);
        }
    }
    
    char* s = new char[f_stack.size() + 1];
    int length = 0;
    while (length < f_stack.size()) { s[length] = f_stack[length]; length++;}
    s[length] = '\0';
    f_stack.clear();
    whitespace_stack.clear();
    
    return s;
}

string color_system(string s) {
    return COLOR(SYSTEM_MSG_COLOR, s);
}

string color_client(string s) {
    return COLOR(CLIENT_MSG_COLOR, s);
}

string color_peer(string s) {
    return COLOR(PEER_MSG_COLOR, s);
}

string color_error(string s) {
    return COLOR(ERROR_MSG_COLOR, s);
}

void trim(string* s_p) {
    while (isspace(s_p->front())) {
        s_p->erase(s_p->begin());
    }
    while (isspace(s_p->back())) {
        s_p->pop_back();
    }
}

