//
//  msg_command.h
//  DCI
//
//  Created by Suhas Vittal on 6/12/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#ifndef msg_command_h
#define msg_command_h

#include "command.h"

#define MSG_SENT_SUCCESS ""
#define MSG_SENT_FAILURE "Messages could not be sent.\n"
class msg_command : public command {
public:
    msg_command(std::list<std::string> args) :command("msg", args) {}
    ~msg_command() {}
    int execute(std::string*) override;
    int is_valid() override { return get_args().size() >= 1; }
};

#endif /* msg_command_h */
