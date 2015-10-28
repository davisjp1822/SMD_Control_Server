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
#include "SMD_Motor_Commands.h"

#include <modbus.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

typedef struct send_modbus_command_args {
	
	const int *registers;
	const int *values;
	int num_registers;
	const char *command_name;
	SMD_RESPONSE_CODES result;
	
} send_modbus_command_args;

typedef struct read_modbus_command_args {
	
	const uint16_t *registers;
	SMD_REGISTER_READ_TYPE reg_read_type;
	int cl;
	SMD_RESPONSE_CODES result;
	char *registers_string;
	
} read_modbus_command_args;

int LOCK_COMMS;
pthread_mutex_t lock;

/* Local function declarations */

static void *_send_modbus_command(void *args);
static void *_read_modbus_registers(void *args);

/* Public function bodies */

SMD_RESPONSE_CODES send_modbus_command(const int *registers, const int *values, const int num_registers, const char *command_name) {
	
	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("Cannot get mutex\n");
		return 0;
	}
	
	else if(LOCK_COMMS == 1) {
		
		printf("Comms are locked when trying to send modbus command\n");
	}
	
	else {
		
		pthread_mutex_lock(&lock);
		
		LOCK_COMMS = 1;
		
		pthread_t tid;
		send_modbus_command_args cmd_args;
		
		cmd_args.registers = registers;
		cmd_args.values = values;
		cmd_args.num_registers = num_registers;
		cmd_args.command_name = command_name;
		
		if(pthread_create(&tid, NULL, _send_modbus_command, &cmd_args) != 0) {
			
			log_message("Could not create Modbus connection thread!\n");
		}
		
		else {
			
			LOCK_COMMS = 0;
			pthread_mutex_unlock(&lock);
			
			pthread_join(tid, NULL);
			return cmd_args.result;
		}
	}
	
	log_message("FATAL: Modbus threading should never have made it here!!!\n");
	return 0;
}

SMD_RESPONSE_CODES read_modbus_registers(const uint16_t *registers, const SMD_REGISTER_READ_TYPE reg_read_type, const int cl, char *registers_string) {
	
	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("Cannot get mutex\n");
		return 0;
	}
	
	else if(LOCK_COMMS == 1) {
		
		printf("Comms are locked when trying to send modbus command\n");
	}
	
	else {
		
		pthread_mutex_lock(&lock);
		
		LOCK_COMMS = 1;
		
		pthread_t tid;
		read_modbus_command_args cmd_args;
		
		cmd_args.registers = registers;
		cmd_args.reg_read_type = reg_read_type;
		cmd_args.cl = cl;
		cmd_args.registers_string = malloc(sizeof(char) * 128);
		
		if(pthread_create(&tid, NULL, _read_modbus_registers, &cmd_args) != 0) {
			
			log_message("Could not create Modbus read registers thread!\n");
		}
		
		else {
			
			LOCK_COMMS = 0;
			
			pthread_mutex_unlock(&lock);
			
			pthread_join(tid, NULL);
			
			//registers_string = &cmd_args.registers_string;
			strncpy(registers_string, cmd_args.registers_string, strlen(cmd_args.registers_string));
			
			fprintf(stderr, "should print registers: %s\n", registers_string);
			
			free(cmd_args.registers_string);
			return cmd_args.result;
		}
		
	}
	
	log_message("FATAL: Modbus read registers threading should never have made it here!!!\n");
	return 0;
}

/* Local function bodies */

static void *_send_modbus_command(void *args) {
	
	send_modbus_command_args *cmd_args = (send_modbus_command_args *)args;
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		
		char message[1024];
		
		snprintf(message,
				 sizeof(message),
				 "Connection failed when trying to connect to motor (%s) to send command %s with error: %s\n",
				 DEVICE_IP,
				 cmd_args->command_name,
				 modbus_strerror(errno));
		
		log_message(message);
		
		modbus_close(ctx);
		modbus_free(ctx);
		
		if(errno == 9) {
			
			log_message("Sleeping due to connection overflow\n");
			nanosleep((const struct timespec[]){{0, 50000000L}}, NULL);
			
			//if the previous command was a hold move or immediate stop, resend it so that we can stop!!!!!!
			if(strncmp(cmd_args->command_name, "hold_move", strlen(cmd_args->command_name)) == 0) {
				
				log_message("!!! Caught a failed hold_move command!\n");
				SMD_hold_move();
			}
			
			if(strncmp(cmd_args->command_name, "immediate_stop", strlen(cmd_args->command_name)) == 0) {
				
				log_message("!!! Caught a failed immediate_stop command!\n");
				SMD_immed_stop();
			}
		}
		
		//TODO - this should fail more gracefully - just tell it connection numbers exceeded?
		//return SMD_RETURN_COMMAND_FAILED;
		cmd_args->result = SMD_RETURN_COMMAND_FAILED;
	}
	
	else {
		
		int rc, i;
		
		for(i=0; i<cmd_args->num_registers; i++) {
			rc = modbus_write_register(ctx, cmd_args->registers[i], cmd_args->values[i]);
			
			if( rc == -1 ) {
				
				char message[1024];
				snprintf(message,
						 sizeof(message),
						 "Error in calling modbus_write_register for command %s with error: %s\n",
						 cmd_args->command_name,
						 modbus_strerror(errno));
				
				log_message(message);
				
				//return SMD_RETURN_COMMAND_FAILED;
				cmd_args->result = SMD_RETURN_COMMAND_FAILED;
			}
			
			char message[1024];
			snprintf(message,
					 sizeof(message),
					 "Successfully wrote value %d to Modbus register %d for command %s\n",
					 cmd_args->values[i],
					 cmd_args->registers[i],
					 cmd_args->command_name);
			
			log_message(message);
			
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		
		//return SMD_RETURN_COMMAND_SUCCESS;
		cmd_args->result = SMD_RETURN_COMMAND_SUCCESS;
	}
	
	pthread_exit(NULL);
	return 0;
}

static void *_read_modbus_registers(void *args) {
	
	read_modbus_command_args *cmd_args = (read_modbus_command_args *)args;
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		
		modbus_close(ctx);
		modbus_free(ctx);
		
		char errorString[1024];
		snprintf(errorString, sizeof(errorString), "Could not connect to SMD when trying to read input registers with error: %s\n", modbus_strerror(errno));
		log_message(errorString);
		
		//return SMD_RETURN_NO_ROUTE_TO_HOST;
		cmd_args->result = SMD_RETURN_NO_ROUTE_TO_HOST;
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
			
			//return SMD_RETURN_COMMAND_FAILED;
			cmd_args->result = SMD_RETURN_COMMAND_FAILED;
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
					
					if(cmd_args->reg_read_type == SMD_READ_INPUT_REGISTERS) {
						strncpy(idString, ",,\0", strlen(",,\0"));
					}
					
					if(cmd_args->reg_read_type == SMD_READ_CONFIG_REGISTERS) {
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
			
			//write the registers to the client and save the string to the return struct
			if(cmd_args->cl > 0) {
				write_to_client(cmd_args->cl, client_write_string);
			}
			
			if(cmd_args->registers_string != NULL) {
				cmd_args->registers_string = strdup(client_write_string);
				fprintf(stderr, "first registers_string: %s\n", cmd_args->registers_string);
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		
		cmd_args->result = SMD_RETURN_COMMAND_SUCCESS;
	}
	
	pthread_exit(NULL);
	return 0;
}

