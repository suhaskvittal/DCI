//
//  alias_command.h
//  DCI
//
//  Created by Suhas Vittal on 6/16/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef alias_command_h
#define alias_command_h

#include "command.h"

#include <unordered_map>

extern std::unordered_map<std::string, std::string> aliases;

class alias_command : public command {
public:
    alias_command(std::list<std::string> args) :command("alias", args) {}
    ~alias_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() == 2; }
};

class unalias_command : public command {
public:
    unalias_command(std::list<std::string> args) :command("unalias", args) {}
    ~unalias_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() == 1; }
};

#endif /* alias_command_h */
