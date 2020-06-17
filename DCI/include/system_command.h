//
//  system_command.h
//  DCI
//
//  Created by Suhas Vittal on 6/12/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef system_command_h
#define system_command_h

#include "command.h"

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


int call_system(std::string c, std::string* buf_p);

#endif /* system_command_h */
