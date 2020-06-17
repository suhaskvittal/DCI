//
//  file_command.h
//  DCI
//
//  Created by Suhas Vittal on 6/12/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef file_command_h
#define file_command_h

#include "command.h"

#define FILE_ACCESS_OPEN 0  // higher number means higher security
#define FILE_ACCESS_SAFE 1
#define FILE_ACCESS_CLOSED 2
#define FILE_SEND_FAILURE "An output stream could not be established."
#define FILE_SEND_SUCCESS "File send occurred successfully."

class file_command : public command {
public:
    file_command(std::string typ, std::list<std::string> args) :file_command(typ, args, "", "") {}
    file_command(std::string typ, std::list<std::string> args, std::string c_dir, std::string p_dir) :command("file", args), cmd_type(typ), client_dir(c_dir), peer_dir(p_dir) {}
    ~file_command() {}
    int execute(std::string*) override;
    int is_valid() override { return cmd_type == "access" ? get_args().size() >= 2 : get_args().size() >= 3; }
    
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

#define MEMARRAY_SIZE 1024
int read_f(std::string path, std::string* data_p);
int write_f(std::string path, std::string data);

#endif /* file_command_h */
