//
//  SMD_Modbus.c
//  SMD_Control_Server
//
//  Created by John Davis on 10/11/15.
//  Copyright © 2015 3ML LLC. All rights reserved.
//

#include "SMD_Modbus.h"
#include "SMD_Constants.h"
#include "SMD_Utilities.h"

#include <modbus.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

SMD_RESPONSE_CODES send_modbus_command(const int *registers, const int *values, const int num_registers, const char *command_name) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		
		char message[1024];
		snprintf(message,
				 sizeof(message),
				 "Connection failed when trying to connect to motor to send command %s with error: %s\n",
				 command_name,
				 modbus_strerror(errno));
		
		log_message(message);
		
		modbus_close(ctx);
		modbus_free(ctx);
		return SMD_RETURN_NO_ROUTE_TO_HOST;
	}
	
	else {
		
		int rc, i;
		
		for(i=0; i<num_registers; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
				
			if( rc == -1 ) {
				
				char message[1024];
				snprintf(message,
						 sizeof(message),
						 "Error in calling modbus_write_register for command %s with error: %s\n",
						 command_name,
						 modbus_strerror(errno));
				
				log_message(message);

				return SMD_RETURN_COMMAND_FAILED;
			}
			
			char message[1024];
			snprintf(message,
					 sizeof(message),
					 "Successfully wrote value %d to Modbus register %d for command %s\n",
					 values[i],
					 registers[i],
					 command_name);
			
			log_message(message);

		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		
		return SMD_RETURN_COMMAND_SUCCESS;
	}
}