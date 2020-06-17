//
//  auth_command.h
//  DCI
//
//  Created by Suhas Vittal on 6/12/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef auth_command_h
#define auth_command_h

#include "command.h"

class auth_command : public command {
public:
    auth_command(std::list<std::string> args) :command("auth", args) {}
    ~auth_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() == 1; }
};

class block_command : public command {
public:
    block_command(std::list<std::string> args) :command("block", args) {}
    ~block_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() == 1; }
};

void give_auth(SOCKET socket);
void remove_auth(SOCKET socket);

#endif /* auth_command_h */
