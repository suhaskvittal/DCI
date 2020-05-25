//
//  parser.hpp
//  DCI
//
//  Created by Suhas Vittal on 5/24/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef command_h
#define command_h

#include "network.h"
#include "node.h"

#include <string>
#include <forward_list>
#include <list>

extern SOCKET client;
extern std::forward_list<struct node> network_sll;
extern int network_size;

#define IS_DELIMITER(c) (c == ' ' || c == '\n')

/* HELP MACROS */
#define SYSTEM_CMD_HELP "system command usage (sends a command to run to a target socket; REQUIRES AUTH): system <socket> <cmd>\n"
#define HELP_CMD_HELP "show sockets: help network\n"
#define AUTH_CMD_HELP "auth command usage (authenticates a given socket): auth <socket>\n"
#define MSG_CMD_HELP "msg command usage (sends a list of messages to a target socket): msg <socket> <msg_1> <msg_2> ... <msg_n>\n"

#define HELP_CMD_LIST { AUTH_CMD_HELP, MSG_CMD_HELP, SYSTEM_CMD_HELP, HELP_CMD_HELP }

/* COMMAND CLASSES */
class command {
public:
    command() :cmd_name("command"), cmd_args() {}
    command(std::string name, std::list<std::string> args) :cmd_name(name), cmd_args(args) {}
    virtual ~command() {}
    virtual int execute(std::string*) =0;  // the string pointer is a pointer to an output buffer
    virtual int is_valid() =0;
    std::string get_name() { return cmd_name; }
    std::list<std::string> get_args() { return cmd_args; }
private:
    std::string cmd_name;
    std::list<std::string> cmd_args;
};

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

class auth_command : public command {
public:
    auth_command(std::list<std::string> args) :command("auth", args) {}
    ~auth_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() == 1; }
};

#define MSG_SENT_SUCCESS "Messages were sent successfully.\n"
#define MSG_SENT_FAILURE "Messages could not be sent.\n"
class msg_command : public command {
public:
    msg_command(std::list<std::string> args) :command("msg", args) {}
    ~msg_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() >= 1; }
};

/* Executes a system command on the computer connected to the target socket. */
class system_command : public command {
public:
    system_command(std::list<std::string> args) :command("system", args) {}
    ~system_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() == 1; }
};

class invalid_command : public command {
public:
    invalid_command() :command("invalid", std::list<std::string>()) {}
    ~invalid_command() {}
    int execute(std::string*) override;
    int is_valid() override { return 0; }
};

int parse_and_send(char* input);
int recv_and_parse(SOCKET from, char** buf_p);
command* to_command(std::string input, SOCKET* socket_p);
void process_input(std::string input, std::list<std::string>* args);
void auth_provided(SOCKET socket);

#endif /* command_h */
