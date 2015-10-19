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
#include "SMD_SocketOps.h"

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
		char formattedIP[strlen(DEVICE_IP)-1];
		
		strncpy(formattedIP, DEVICE_IP, sizeof(formattedIP));
		
		snprintf(message,
				 sizeof(message),
				 "Connection failed when trying to connect to motor (%s) to send command %s with error: %s\n",
				 formattedIP,
				 command_name,
				 modbus_strerror(errno));
		
		log_message(message);
		
		modbus_close(ctx);
		modbus_free(ctx);
		
		//TODO - this should fail more gracefully - just tell it connection numbers exceeded?
		return SMD_RETURN_COMMAND_FAILED;
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

SMD_RESPONSE_CODES read_modbus_registers(const uint16_t *registers, const SMD_REGISTER_READ_TYPE reg_read_type, const int cl) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		
		modbus_close(ctx);
		modbus_free(ctx);
		
		char errorString[1024];
		snprintf(errorString, sizeof(errorString), "Could not connect to SMD when trying to read input registers with error: %s\n", modbus_strerror(errno));
		log_message(errorString);
		
		return SMD_RETURN_NO_ROUTE_TO_HOST;
		
	}
	
	else {
		
		int rc = 0, i = 0;
		uint16_t tab_status_words_reg[10];
		char client_write_string[128];
		
		memset(&tab_status_words_reg, 0, sizeof(tab_status_words_reg));
		memset(&client_write_string, 0, sizeof(client_write_string));
		
		if( (rc=modbus_read_registers(ctx, 0, 10, tab_status_words_reg)) == -1 ) {
			
			char errorString[1024];
			snprintf(errorString, sizeof(errorString), "Could not read input registers with error: %s\n", modbus_strerror(errno));
			log_message(errorString);
			
			return SMD_RETURN_COMMAND_FAILED;
			
		}
		
		else {
			for( i=0; i < rc; i++ ) {
				
				char temp[16];
				
				memset(&temp, 0, sizeof(temp));
				
				//only add leading 0x for items that are legitimately in hex
				if(i == 0 || i == 1)
					snprintf(temp, 16, "0x%X", tab_status_words_reg[i]);
				
				else
					snprintf(temp, 16, "%d", (int16_t)tab_status_words_reg[i]);
				
				//add the leading character
				if(i==0) {
					
					char idString[4];
					
					if(reg_read_type == SMD_READ_INPUT_REGISTERS) {
						strncpy(idString, ",,\0", strlen(",,\0"));
					}
					
					if(reg_read_type == SMD_READ_CONFIG_REGISTERS) {
						strncpy(idString, "###\0", strlen("###\0"));
					}
					
					snprintf(client_write_string, sizeof(client_write_string), "%s%s", idString, temp);
				}
				
				else {
					snprintf(client_write_string, sizeof(client_write_string), "%s,%s", client_write_string, temp);
				}
			}
			
			//close the string with a linebreak so that the data is sent
			snprintf(client_write_string, sizeof(client_write_string), "%s%s", client_write_string, "\n");
			
			//write the registers to the client
			write_to_client(cl, client_write_string);
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		
		return SMD_RETURN_COMMAND_SUCCESS;
	}

}