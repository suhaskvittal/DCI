//
//  help_command.h
//  DCI
//
//  Created by Suhas Vittal on 6/12/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef help_command_h
#define help_command_h

#include "command.h"

/* Help has different cases:
    - no args: list possible help commands
    - 1 arg: provide help
 */

class help_command : public command {
public:
    help_command(std::list<std::string> args) :command("help", args) {}
    ~help_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() <= 1; }
};

/* HELP MACROS */
#define HELP_CMD_HELP "show sockets: help network\nclose connection: exit\n"
#define AUTH_CMD_HELP "auth command usage (authenticates a given socket): auth <socket>\n"
#define ALIAS_CMD_HELP "alias command usage (creates/sets an alias): alias <alias> <text>"
#define BLOCK_CMD_HELP "block command usage (de-authenticates a given socket): auth <socket>\n"
#define MSG_CMD_HELP "msg command usage (sends a list of messages to a target socket): msg <socket> <msg_1> <msg_2> ... <msg_n>\n"
#define FILE_CMD_HELP "file command usage (send/get a file from a peer, or change file access level; REQUIRES AUTH):\n\t(file send) file send <target> <src> <dst>\n\t(file get) file get <target> <src> <dst>\n\t\(file access; levels are open, safe (only sending), closed (nothing allowed)) file access <level>\n"
#define SYSTEM_CMD_HELP "system command usage (sends a command to run to a target socket; REQUIRES AUTH): system <socket> <cmd>\n"
#define UNALIAS_CMD_HELP "unalias command usage (deletes an alias): unalias <alias>"

#endif /* help_command_h */
