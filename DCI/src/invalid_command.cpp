//
//  command.cpp
//  DCI
//
//  Created by Suhas Vittal on 5/24/20.
//  Copyright © 2020 Suhas Vittal. All rights reserved.
//

#include "../include/command.h"

#include <string>

using namespace std;

int invalid_command::execute(string* buf_p) {
    *buf_p = "Invalid command. Please try again.";
    return 0;
}

