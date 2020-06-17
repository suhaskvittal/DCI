//
//  alias_command.cpp
//  DCI
//
//  Created by Suhas Vittal on 6/16/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/alias_command.h"

using namespace std;

unordered_map<std::string, std::string> aliases;

int alias_command::execute(string* buf_p) {
    /* Alias command have only two args:
        (1) The alias to create.
        (2) What it equals.
     */
    list<string> args(get_args());
    auto arg_i = args.begin();
    string alias = *arg_i++;
    string raw = *arg_i++;
    aliases[alias] = raw;
    *buf_p = "\"" + alias + "\" now is an alias for \"" + raw + "\".";
    return 0;
}

int unalias_command::execute(string* buf_p) {
    list<string> args(get_args());
    auto arg_i = args.begin();
    string alias = *arg_i++;
    if (aliases.count(alias) <= 0) {
        return 1;
    } else {
        aliases.erase(alias);
        *buf_p = "The alias \""  + alias + "\" has been removed.";
        return 0;
    }
}
