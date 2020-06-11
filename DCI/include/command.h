//
//  command.h
//  DCI
//
//  Created by Suhas Vittal on 5/24/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef command_h
#define command_h

#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "node.h"

#include <string>
#include <forward_list>
#include <list>
#include <vector>

extern SOCKET client;
extern std::forward_list<struct node*> network_sll;
extern int network_size;

#define IS_DELIMITER(c) (c == ' ' || c == '\n')

/* HELP MACROS */
#define SYSTEM_CMD_HELP "system command usage (sends a command to run to a target socket; REQUIRES AUTH): system <socket> <cmd>\n"
#define HELP_CMD_HELP "show sockets: help network\n"
#define AUTH_CMD_HELP "auth command usage (authenticates a given socket): auth <socket>\n"
#define MSG_CMD_HELP "msg command usage (sends a list of messages to a target socket): msg <socket> <msg_1> <msg_2> ... <msg_n>\n"
#define FILE_CMD_HELP "file command usage (send/get a file from a peer, or change file access level; REQUIRES AUTH): (file send) file send <target> <src> <dst>, (file get) file get <target> <src> <dst>, (file access; levels are open, safe (only sending), closed (nothing allowed)) file access <level>\n"

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

class block_command : public command {
public:
    block_command(std::list<std::string> args) :command("block", args) {}
    ~block_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() == 1; }
};

#define MSG_SENT_SUCCESS ""
#define MSG_SENT_FAILURE "Messages could not be sent.\n"
class msg_command : public command {
public:
    msg_command(std::list<std::string> args) :command("msg", args) {}
    ~msg_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() >= 1; }
};

#define FILE_ACCESS_OPEN 0  // higher number means higher security
#define FILE_ACCESS_SAFE 1
#define FILE_ACCESS_CLOSED 2
#define FILE_SEND_FAILURE "An output stream could not be established.\n"
#define FILE_SEND_SUCCESS "File send occurred successfully.\n"
class file_command : public command {
public:
    file_command(std::string typ, std::list<std::string> args) :file_command(typ, args, "", "") {}
    file_command(std::string typ, std::list<std::string> args, std::string c_dir, std::string p_dir) :command("file", args), cmd_type(typ), client_dir(c_dir), peer_dir(p_dir) {}
    ~file_command() {}
    int execute(std::string*) override;
    int is_valid() override { return cmd_type == "access" ? get_args().size() == 2 : get_args().size() >= 3; }
    
    std::string get_client_dir() { return client_dir; }
    std::string get_peer_dir() { return peer_dir; }
private:
    std::string cmd_type;
    std::string client_dir;
    std::string peer_dir;
    
    int file_send(std::string*);    // sends a copy of a file
    int file_get(std::string*);     // gets a copy of a file
    int file_access(std::string*);  // sets file access level
};

/* Executes a system command on the computer connected to the target socket. */
#if defined(_WIN32)
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif
class system_command : public command {
public:
    system_command(std::list<std::string> args) :command("system", args) {}
    ~system_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() == 2; }
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
command* create_command(std::string input, SOCKET* socket_p);
void process_input(std::string input, std::list<std::string>* args);
void give_auth(SOCKET socket);
void remove_auth(SOCKET socket);
int send_command(SOCKET to, command* cmd, std::vector<std::string> tokens);

command* build_command(std::list<std::string>, SOCKET* target_p);
int call_system(std::string c, std::string* buf_p);
#endif /* command_h */
