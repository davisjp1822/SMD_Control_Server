//
//  SMD_SocketOps.c
//  SMD_Control_Server
//
//  Created by John Davis on 10/3/15.
//  Copyright © 2015 3ML LLC. All rights reserved.
//

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "SMD_SocketOps.h"
#include "SMD_Constants.h"
#include "SMD_Utilities.h"
#include "SMD_Motor_Commands.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#define BUF_SIZE 8192

void open_server_socket() {
	
	struct sockaddr_in addr;
	struct sockaddr_storage client_addr;
	socklen_t len;
	int16_t cl,rc,fd;
	char buf[BUF_SIZE];
	
	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket Error");
		exit(-1);
	}
	
	len = sizeof(client_addr);
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);
	
	int yes = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("Error Setting Sock Options");
		exit(-1);
	}
	
	if (setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &yes, sizeof(int)) == -1) {
		perror("Error Setting Sock Options");
		exit(-1);
	}
	
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("Socket Bind Error");
		exit(-1);
	}
	
	if (listen(fd, 5) == -1) {
		perror("Socket Listen Error");
		exit(-1);
	}
	
	if ( (cl = accept(fd, (struct sockaddr*)&client_addr, &len)) == -1) {
		perror("Socket Accept Error");
		exit(-1);
	}
	
	else {
			
		char str[INET_ADDRSTRLEN];
		struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
			
		inet_ntop(AF_INET, &s->sin_addr, str, INET_ADDRSTRLEN);
		
		char message[1024];
		snprintf(message,
				 sizeof(message),
				 "New client connection from %s\n",
				 str);
		
		log_message(message);
	}
	
	//parse socket input
	while (1) {
		
		//this clears the buffer, gets it ready for more input
		memset(buf, 0, sizeof(buf));
		
		rc=read(cl,buf,sizeof(buf));
		
		if(rc > 0) {
			
			parse_smd_response_to_client_input(buf, cl);
		}
		
		else {

			close(cl);
			
			//if the client disconnects, we should close the command connection with the motor for safety
			SMD_close_command_connection();
			
			log_message("Client disconnected\n");
			
			//start a new socket connection accept
			if ( (cl = accept(fd, (struct sockaddr*)&client_addr, &len)) == -1) {
				perror("Socket Accept Error");
				exit(-1);
			}
			
			else {
				
				char str[INET_ADDRSTRLEN];
				struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
				
				inet_ntop(AF_INET, &s->sin_addr, str, INET_ADDRSTRLEN);
				
				char message[1024];
				snprintf(message,
						 sizeof(message),
						 "New client connection from %s\n",
						 str);
				
				log_message(message);
			}
		}
	}
}

int parse_socket_input(const char *input, const int cl) {
	
	if(SMD_CONNECTED == 0) {
		
		if(strncmp(input, CONNECT, strlen(CONNECT)) == 0) {
			
			char *array_of_commands[2] = {0};
			int num_tokens = 0;
			char *smd_ip = NULL;
			
			//should have 2 commands only (connect and device ip)
			if((num_tokens = number_of_tokens(input)) != 2) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			//now that we are sure that we have an ip, parse it out
			else {
				
				int i = 0;
				if(tokenize_client_input(array_of_commands, input, num_tokens, ARRAYSIZE(array_of_commands)) != 0) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//loop through the input and convert to int
				for(i=0; i<num_tokens; i++) {
					
					//skip the actual command, "connect"
					//just get the ip address
					if(i == 1) {
						smd_ip = malloc(sizeof(char) * (strlen(array_of_commands[i]))+1);
						
						if(smd_ip) {
							strncpy(smd_ip, array_of_commands[i], strlen(array_of_commands[i]));
						}
					}
				}
				
				//now try and connect
				if(smd_ip) {
					
					DEVICE_IP = malloc(sizeof(char) * strlen(smd_ip));
					strncpy(DEVICE_IP, smd_ip, strlen(smd_ip)-1);
					
					if(strstr(DEVICE_IP, "\n") != NULL) {
						//log_message("New line in DEVICE_IP! Fixing...\n");
						DEVICE_IP[strcspn(DEVICE_IP, "\n")] = '\0';
					}
					
					free(smd_ip);
					return SMD_open_command_connection();
				}
				
				//should never get here - smd_ip would have to NULL
				else {
					log_message("Fatal error - we should NOT be here\n");
					free(smd_ip);
					return SMD_RETURN_NO_ROUTE_TO_HOST;
				}
			}
		}
		
		//if not connected, and the command is not 'connect', return 'invalid_input'
		else {
	
			return SMD_RETURN_INVALID_INPUT;
		}
	}
	
	/*
		JOG_CW, JOG_CCW, FIND_HOME_CW and FIND_HOME_CCW all use strstr() because of string similarity when doing the comparison
	 
		Also, all of these commands require a connection to the SMD. If there is no connection, return SMD_RETURN_NO_ROUTE_TO_HOST
	 
		Several scenarios exist, depending on what state the drive is currently in, and what we want to do.
		
		*** Scenario 1
		Standard motion - drive is connected, and we are listening for commands over the socket interface. There is no assembled move in progress,
		and we are not accepting commands from the manual controls. All of the different mode states are set in this scenario by the client.
	 
		*** Scenario 2
		We are connected to the motor, and are in manual mode. The *only* command that will be accepted from the client is stopManualMode, resetErrors,
		or readInputRegisters.
		Everything else is invalid input.
	 
		*** Scenario 3
		We are connected to the motor, but in the process of loading an assembled move. The only valid input at this point is a JSON object
		representing an assembled move.
	 
		*** Scenario 4
		This is the fallthrough state. Something borked.
	 
	 */
	
	/* Scenario 1 */
	if(SMD_CONNECTED == 1 && STATUS_WAITING_FOR_ASSEMBLED_MOVE == 0 && STATUS_MANUAL_MODE_ENABLE == 0) {
		
		if(strstr(input, JOG_CW) !=NULL || strstr(input, JOG_CCW) !=NULL) {
			
			//search for the jog substring - if it is found, then parse out:
			// accel
			// decel
			// steps/s
			
			char *array_of_commands[5] = {0};
			
			int num_tokens = 0;
			int direction = 0;			//0=CW, 1=CCW
			int16_t accel = 0;
			int16_t decel = 0;
			int16_t jerk = 0;
			int32_t speed = 0;
			
			//should have 5 commands only
			if((num_tokens = number_of_tokens(input)) != 5) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				int i = 0;
				
				if(tokenize_client_input(array_of_commands, input, num_tokens, ARRAYSIZE(array_of_commands)) != 0) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//loop through the input and convert to int
				for(i=0; i<5; i++) {
					
					//skip the actual command, "jogCW"
					//accel
					if(i == 1) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						accel = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//decel
					if(i == 2) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						decel = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//jerk
					if(i == 3) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						jerk = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//speed
					if(i == 4) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						speed = (uint32_t)convert_string_to_long_int(array_of_commands[i]);
					}
				}
				
				//figure out the direction
				if(strstr(input, JOG_CCW) !=NULL)
					direction = 1;
				
				//tell the motor to jog
				return SMD_jog(direction, accel, decel, jerk, speed);
			}
		}
		
		else if(strstr(input, FIND_HOME_CW) !=NULL || strstr(input, FIND_HOME_CCW) !=NULL) {
			
			//search for the home substring - if it is found, then parse out:
			// accel
			// decel
			// steps/s
			
			char *array_of_commands[5] = {0};
			
			int num_tokens = 0;
			int direction = 0;			//0=CW, 1=CCW
			int16_t accel = 0;
			int16_t decel = 0;
			int16_t jerk = 0;
			int32_t speed = 0;
			

			//should have 5 commands only
			if((num_tokens = number_of_tokens(input))  != 5) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				int i = 0;
				
				if(tokenize_client_input(array_of_commands, input, num_tokens, ARRAYSIZE(array_of_commands)) != 0) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//loop through the input and convert to int
				for(i=0; i<5; i++) {
					
					//skip the actual command
					//speed
					if(i == 1) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						speed = (uint32_t)convert_string_to_long_int(array_of_commands[i]);
					}
					
					//accel
					if(i == 2) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						accel = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//decel
					if(i == 3) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						decel = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//jerk
					if(i == 4) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						jerk = convert_string_to_long_int(array_of_commands[i]);
					}
				}
				
				//figure out the direction
				if(strstr(input, FIND_HOME_CCW) !=NULL)
					direction = 1;
				
				return SMD_find_home(direction, speed, accel, decel, jerk);
		
			}
		}
		
		else if(strncmp(input, DISCONNECT, strlen(DISCONNECT)) == 0) {
			
			SMD_close_command_connection();
			return SMD_RETURN_DISCONNECT_SUCCESS;
		}
		
		else if(strncmp(input, SAVE_CONFIG_TO_DRIVE, strlen(SAVE_CONFIG_TO_DRIVE)) == 0) {
			
			char *array_of_commands[8] = {0};
			int num_tokens = 0;
			
			int32_t control_word = 0;
			int32_t config_word = 0;
			int32_t starting_speed = 0;
			int16_t steps_per_turn = 0;
			int16_t enc_pulses_per_turn = 0;
			int16_t idle_current_percentage = 0;
			int16_t motor_current = 0;
			
			if((num_tokens = number_of_tokens(input)) != 8) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				int i = 0;
				
				if(tokenize_client_input(array_of_commands, input, num_tokens, ARRAYSIZE(array_of_commands)) != 0) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				for(i=0; i<num_tokens; i++) {
					
					//control word
					if(i == 1) {
						control_word = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//config word
					if(i == 2) {
						config_word = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//starting speed
					if(i == 3) {
						starting_speed = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//motor steps/turn
					if(i == 4) {
						steps_per_turn = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//encoder pulses/turn
					if(i == 5) {
						enc_pulses_per_turn = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//idle current percentage
					if(i == 6) {
						idle_current_percentage = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//motor current
					if(i == 7) {
						motor_current = convert_string_to_long_int(array_of_commands[i]);
					}
				}
				
				return SMD_set_configuration(control_word, config_word, starting_speed, steps_per_turn, enc_pulses_per_turn, idle_current_percentage, motor_current);
			}
		}
		
		else if(strncmp(input, RELATIVE_MOVE, strlen(RELATIVE_MOVE)) == 0) {
			
			char *array_of_commands[6] = {0};
			
			int num_tokens = 0;
			int32_t rel_pos = 0;
			int32_t speed = 0;
			int16_t accel = 0;
			int16_t decel = 0;
			int16_t jerk = 0;

			//loop through the input and convert to int
			if((num_tokens = number_of_tokens(input)) != 6) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				int i = 0;
				
				if(tokenize_client_input(array_of_commands, input, num_tokens, ARRAYSIZE(array_of_commands)) != 0) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				for(i=0; i<num_tokens; i++) {
					
					//relative position
					if(i==1) {
						rel_pos = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//accel
					if(i==2) {
						accel = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//decel
					if(i==3) {
						decel = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//jerk
					if(i==4) {
						jerk = convert_string_to_long_int(array_of_commands[i]);
					}
					
					//speed
					if(i==5) {
						speed = convert_string_to_long_int(array_of_commands[i]);
					}
				}
				
				return SMD_relative_move(rel_pos, accel, decel, jerk, speed);
			}
		}
		
		else if(strncmp(input, DRIVE_ENABLE, strlen(DRIVE_ENABLE)) == 0) {
			
			return SMD_drive_enable();
			
		}
		
		else if(strncmp(input, DRIVE_DISABLE, strlen(DRIVE_DISABLE)) == 0) {
			
			return SMD_drive_disable();
		}
		
		else if(strncmp(input, HOLD_MOVE, strlen(HOLD_MOVE)) == 0) {
			
			return SMD_hold_move();
			
		}
		
		else if(strncmp(input, IMMED_STOP, strlen(IMMED_STOP)) == 0) {
			
			return SMD_immed_stop();
		}
		
		else if(strncmp(input, RESET_ERRORS, strlen(RESET_ERRORS)) == 0) {
			
			return SMD_reset_errors();
		}
		
		else if(strncmp(input, READ_INPUT_REGISTERS, strlen(READ_INPUT_REGISTERS)) == 0) {
			
			return SMD_read_input_registers(cl);
		}
		
		else if(strncmp(input, LOAD_CURRENT_CONFIGURATION, strlen(LOAD_CURRENT_CONFIGURATION)) == 0) {
			
			return SMD_load_current_configuration(cl);
	
		}
		
		else if(strncmp(input, READ_CURRENT_CONFIGURATION, strlen(READ_CURRENT_CONFIGURATION)) == 0) {
			
			return SMD_read_current_configuration(cl);

		}
		
		else if(strncmp(input, PRESET_MOTOR_POSITION, strlen(PRESET_MOTOR_POSITION)) == 0) {
			
			char *array_of_commands[2] = {0};
			
			int num_tokens = 0;
			int32_t pos = 0;

			//loop through the input and convert to int
			if((num_tokens = number_of_tokens(input)) != 2) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				if(tokenize_client_input(array_of_commands, input, num_tokens, ARRAYSIZE(array_of_commands)) != 0) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				pos = (int32_t)convert_string_to_long_int(array_of_commands[1]);
				
				return SMD_preset_motor_position(pos);
			}
		}
		
		else if(strncmp(input, PRESET_ENCODER_POSITION, strlen(PRESET_ENCODER_POSITION)) == 0) {
			
			char *array_of_commands[2] = {0};
			
			int num_tokens = 0;
			int32_t pos = 0;
			
			//loop through the input and convert to int
			if((num_tokens = number_of_tokens(input)) != 2) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {

				if(tokenize_client_input(array_of_commands, input, num_tokens, ARRAYSIZE(array_of_commands)) != 0) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				pos = (int32_t)convert_string_to_long_int(array_of_commands[1]);
				
				return SMD_preset_encoder_position(pos);

			}
		}
		
		else if(strncmp(input, PROGRAM_ASSEMBLED_MOVE, strlen(PROGRAM_ASSEMBLED_MOVE)) == 0) {
			
			return SMD_program_assembled_move(cl);
		}
		
		else if(strncmp(input, RUN_ASSEMBLED_DWELL_MOVE, strlen(RUN_ASSEMBLED_DWELL_MOVE)) == 0) {
			
			char *array_of_commands[2];
			
			int num_tokens = 0;
			int32_t dwell_time = 0;
			
			//loop through the input and convert to int
			if((num_tokens = number_of_tokens(input)) != 2) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				if(tokenize_client_input(array_of_commands, input, num_tokens, ARRAYSIZE(array_of_commands)) != 0) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				dwell_time = (int32_t)convert_string_to_long_int(array_of_commands[1]);
				
				if(dwell_time < 0 || dwell_time > 65535)
					return SMD_RETURN_INVALID_PARAMETER;
				
				return SMD_run_assembled_move(0, dwell_time, SMD_ASSEMBLED_MOVE_TYPE_DWELL);
			}
			
		}
		
		else if(strncmp(input, RUN_ASSEMBLED_BLEND_MOVE, strlen(RUN_ASSEMBLED_BLEND_MOVE)) == 0) {
			
			char *array_of_commands[2];
			
			int num_tokens = 0;
			int16_t direction = 0; /* 0 = CW, 1 = CCW */
			
			//loop through the input and convert to int
			if((num_tokens = number_of_tokens(input)) != 2) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				if(tokenize_client_input(array_of_commands, input, num_tokens, ARRAYSIZE(array_of_commands)) != 0) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				direction = (int16_t)convert_string_to_long_int(array_of_commands[1]);
				
				if(direction < 0 || direction > 1) {
					return SMD_RETURN_INVALID_PARAMETER;
				}
				
				return SMD_run_assembled_move(direction, 0, SMD_ASSEMBLED_MOVE_TYPE_BLEND);
			}
		}
		
		else if(strncmp(input, START_MANUAL_MODE, strlen(START_MANUAL_MODE)) == 0) {
			
			/* Set STATUS_MANUAL_MODE_ENABLE and call the function that spins off a thread to handle listening to the manual inputs */
			STATUS_MANUAL_MODE_ENABLE = 1;
			return SMD_set_manual_mode();
		}
		
		//the client submitted a command that was not recognized
		else {
			return SMD_RETURN_INVALID_INPUT;
		}
	}
	
	/* Scenario 2 */
	if(SMD_CONNECTED == 1 && STATUS_WAITING_FOR_ASSEMBLED_MOVE == 0 && STATUS_MANUAL_MODE_ENABLE == 1) {
		
		if(strncmp(input, STOP_MANUAL_MODE, strlen(STOP_MANUAL_MODE)) == 0) {
			
			/* set bit to stop manual mode and then issue an immediate stop command to the motor */
			STATUS_MANUAL_MODE_ENABLE = 0;
			
			SMD_RESPONSE_CODES response = SMD_immed_stop();
			
			/* if the command fails, disconnect and bail! */
			if(response == SMD_RETURN_COMMAND_FAILED) {
				
				SMD_close_command_connection();
				return SMD_RETURN_COMMAND_FAILED;
			}
			
			else {
				return SMD_RETURN_COMMAND_SUCCESS;
			}
		}
		
		else if(strncmp(input, RESET_ERRORS, strlen(RESET_ERRORS)) == 0) {
			
			return SMD_reset_errors();
		}
		
		else if(strncmp(input, READ_INPUT_REGISTERS, strlen(READ_INPUT_REGISTERS)) == 0) {
			
			return SMD_read_input_registers(cl);
		}
		
		else {
			return SMD_RETURN_INVALID_INPUT;
		}
	}
	
	/* Scenario 3 */
	else if(SMD_CONNECTED == 1 && STATUS_WAITING_FOR_ASSEMBLED_MOVE == 1) {
		
		//if valid JSON, proceed with programming
		if(SMD_parse_and_upload_assembled_move(input, cl) == SMD_RETURN_COMMAND_SUCCESS) {
			
			STATUS_WAITING_FOR_ASSEMBLED_MOVE = 0;
			return SMD_RETURN_ASSEMBLED_MOVE_ACCEPTED;
		}
		
		//if not valid JSON, return failure, reset STATUS_WAITING_FOR_ASSEMBLED_MOVE, and reset errors
		else {
			
			STATUS_WAITING_FOR_ASSEMBLED_MOVE = 0;
			SMD_reset_errors();
			
			return SMD_RETURN_COMMAND_FAILED;
		}
	}
	
	/* Scenario 4 */
	else {
		return SMD_RETURN_NO_ROUTE_TO_HOST;
	}
	
}

void parse_smd_response_to_client_input(const char *input, const int cl) {
	
	int smd_response = parse_socket_input(input,cl);
	
	//successful run - tell the client
	if(smd_response == SMD_RETURN_COMMAND_SUCCESS ) {
		write_to_client(cl, COMMAND_SUCCESS);
	}
	
	//succesful connect to SMD
	if(smd_response == SMD_RETURN_CONNECT_SUCCESS) {
		write_to_client(cl, SMD_CONNECT_SUCCESS);
	}
	
	//outputting string for parsing by client - don't do anything - the helper function handles the client write
	if(smd_response == SMD_RETURN_HANDLED_BY_CLIENT) {
		//do nothing
	}
	
	//successful SMD enable
	if(smd_response == SMD_RETURN_ENABLE_SUCCESS) {
		write_to_client(cl, ENABLE_SUCCESS);
	}
	
	//successful SMD disable
	if(smd_response == SMD_RETURN_DISABLE_SUCCESS) {
		write_to_client(cl, DISABLE_SUCCESS);
	}
	
	//command borked
	if(smd_response == SMD_RETURN_COMMAND_FAILED){
		write_to_client(cl, COMMAND_ERROR);
	}
	
	//invalid input
	if(smd_response == SMD_RETURN_INVALID_INPUT) {
		
		if(write_to_client(cl, INVALID_INPUT) == 0) {
			close(cl);
		}
	}
	
	//no route to SMD - can't connect
	if(smd_response == SMD_RETURN_NO_ROUTE_TO_HOST){
		write_to_client(cl, NO_ROUTE_TO_SMD);
	}
	
	//invalid parameter, such as in a jog function
	if(smd_response == SMD_RETURN_INVALID_PARAMETER){
		write_to_client(cl, INVALID_PARAMETER);
	}
	
	//preset encoder success
	if(smd_response == SMD_RETURN_PRESET_ENC_SUCCESS) {
		write_to_client(cl, PRESET_ENCODER_SUCCESS);
	}
	
	//preset encoder fail
	if(smd_response == SMD_RETURN_PRESET_ENC_FAIL) {
		write_to_client(cl, PRESET_ENCODER_FAIL);
	}
	
	//preset motor success
	if(smd_response == SMD_RETURN_PRESET_POS_SUCCESS) {
		write_to_client(cl, PRESET_POSITION_SUCCESS);
	}
	
	//preset motor fail
	if(smd_response == SMD_RETURN_PRESET_POS_FAIL) {
		write_to_client(cl, PRESET_POSITION_FAIL);
	}
	
	//save config to drive success
	if(smd_response == SMD_RETURN_SAVE_CONFIG_SUCCESS) {
		write_to_client(cl, CONFIG_SAVE_SUCCESS);
	}
	
	//save config to drive fail
	if(smd_response == SMD_RETURN_SAVE_CONFIG_FAIL) {
		write_to_client(cl, CONFIG_SAVE_FAIL);
	}
	
	//tell the client that we are ready to read the config from the input registers
	if(smd_response == SMD_RETURN_READY_TO_READ_CONFIG) {
		write_to_client(cl, READY_TO_READ_CONFIG);
	}
	
	//read current config fail
	if(smd_response == SMD_RETURN_READ_CURRENT_CONFIG_FAIL) {
		write_to_client(cl, GET_CURRENT_CONFIG_FAIL);
	}
	
	//reset errors success
	if(smd_response == SMD_RETURN_RESET_ERRORS_SUCCESS) {
		write_to_client(cl, RESET_ERRORS_SUCCESS);
	}
	
	//ready for an assembled move profile
	if(smd_response == SMD_RETURN_READY_FOR_ASSEMBLED_MOVE) {
		
		STATUS_WAITING_FOR_ASSEMBLED_MOVE = 1;
		write_to_client(cl, SEND_ASSEMBLED_MOVE_PARAMS);
	}
	
	//the assembled move was accepted!
	if(smd_response == SMD_RETURN_ASSEMBLED_MOVE_ACCEPTED) {
		write_to_client(cl, ASSEMBLED_MOVE_ACCEPTED);
	}
	
}

int write_to_client(const int cl, const char *message) {
	
	if(VERBOSE == 1 && strncmp(message, ",,", 2) != 0) {
		fprintf(stderr, "Client write: %s", message);
	}
	
	//Send the message back to client
	if(write(cl, message , strlen(message)) == -1) {
		log_message("Error writing to client\n");
		return -1;
	}
	else {
		return 0;
	}
}

