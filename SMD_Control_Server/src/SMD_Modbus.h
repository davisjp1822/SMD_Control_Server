//
//  SMD_Modbus.h
//  SMD_Control_Server
//
//  Created by John Davis on 10/11/15.
//  Copyright © 2015 3ML LLC. All rights reserved.
//

#ifndef SMD_Modbus_h
#define SMD_Modbus_h

#include "SMD_Constants.h"

#include <stdio.h>

SMD_RESPONSE_CODES send_modbus_command(const int *registers, const int *values, const int num_registers, const char *command_name);

#endif /* SMD_Modbus_h */
