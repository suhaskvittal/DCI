//
//  command.h
//  DCI
//
//  Created by Suhas Vittal on 5/24/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef command_h
#define command_h

#include "node.h"

#include <string>
#include <forward_list>
#include <list>
#include <vector>

extern SOCKET client;
extern std::forward_list<struct node*> network_sll;
extern int network_size;

#define IS_DELIMITER(c) (c == ' ' || c == '\n')

/* COMMAND CLASSES */
#define EXEC_RESULT_SUCCESS 0
#define EXEC_RESULT_FAILURE 1
#define EXEC_RESULT_BOUNCE_FGET 2

#define EXEC_RESEND_TOKEN "0"

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

class invalid_command : public command {
public:
    invalid_command() :command("invalid", std::list<std::string>()) {}
    ~invalid_command() {}
    int execute(std::string*) override;
    int is_valid() override { return 0; }
};

int parse_and_send(char* input);
int recv_and_parse(SOCKET from, char** buf_p);
void process_input(std::string input, std::list<std::string>* args);

command* create_command(std::string input, SOCKET* socket_p);
command* build_command(std::list<std::string>, SOCKET* target_p);

int send_command(SOCKET to, command* cmd, std::vector<std::string> tokens);

#endif /* command_h */
