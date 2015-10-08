//
//  SMD_SocketOps.c
//  SMD_Control_Server
//
//  Created by John Davis on 10/3/15.
//  Copyright © 2015 3ML LLC. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "SMD_SocketOps.h"
#include "SMD_Constants.h"
#include "SMD_Utilities.h"
#include "SMD_Motor_Commands.h"

void open_server_socket() {
	
	struct sockaddr_in addr;
	int8_t cl,rc,fd;
	char buf[1024];
	
	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket Error");
		exit(-1);
	}
	
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
	
	if ( (cl = accept(fd, NULL, NULL)) == -1) {
		perror("Socket Accept Error");
		exit(-1);
	}
	
	//parse socket input
	while (1) {
		
		//this clears the buffer, gets it ready for more input
		bzero(buf, 1024);
		
		//should fork maybe?
		
		if((rc=read(cl,buf,sizeof(buf))) > 0) {
			parse_smd_response_to_client_input(rc, buf, fd, cl);
		}
		
		else {
			close(cl);
			
			//if the client disconnects, we should close the command connection with the motor for safety
			SMD_close_command_connection();
			
			//start a new socket connection accept
			if ( (cl = accept(fd, NULL, NULL)) == -1) {
				perror("Socket Accept Error");
			}
		}
	}
}

int parse_socket_input(char *input, int cl) {
	
	if(SMD_CONNECTED == 0) {
		
		if(strncmp(input, CONNECT, strlen(CONNECT)) == 0) {
			
			char array_of_commands[2] = {0};
			int num_tokens = 0;
			char *smd_ip = NULL;
			
			//should have 2 commands only (connect and device ip)
			if((num_tokens = number_of_tokens(input)) != 2) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			//now that we are sure that we have an ip, parse it out
			else {
				
				int i = 0;
				tokenize_client_input(array_of_commands, input, num_tokens);
				
				//loop through the input and convert to int
				for(i=0; i<num_tokens; i++) {
					
					//skip the actual command, "connect"
					//just get the ip address
					if(i == 1) {
						smd_ip = malloc(sizeof(char) * (strlen(&array_of_commands[i]))+1);
						
						if(smd_ip) {
							strncpy(smd_ip, &array_of_commands[i], strlen(&array_of_commands[i]));
						}
					}
				}
				
				//now try and connect
				if(smd_ip) {
					
					strncpy(DEVICE_IP, smd_ip, strlen(smd_ip));
					
					if( SMD_open_command_connection() == -1) {
						free(smd_ip);
						return SMD_RETURN_NO_ROUTE_TO_HOST;
					}
					
					else {
						free(smd_ip);
						return SMD_RETURN_CONNECT_SUCCESS;
					}
				}
				
				//should never get here - smd_ip would have to NULL
				else {
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
	 */
	
	if(SMD_CONNECTED == 1) {
		
		if(strstr(input, JOG_CW) !=NULL || strstr(input, JOG_CCW) !=NULL) {
			
			//search for the jog substring - if it is found, then parse out:
			// accel
			// decel
			// steps/s
			
			char array_of_commands[5] = {0};
			
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
				
				tokenize_client_input(array_of_commands, input, num_tokens);
				
				//loop through the input and convert to int
				for(i=0; i<5; i++) {
					
					//skip the actual command, "jogCW"
					//accel
					if(i == 1) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						accel = convert_string_to_long_int(&array_of_commands[i]);
						
						if(accel < 0 || accel > 5001)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//decel
					if(i == 2) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						decel = convert_string_to_long_int(&array_of_commands[i]);
						
						if(decel < 0 || decel > 5001)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//jerk
					if(i == 3) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						jerk = convert_string_to_long_int(&array_of_commands[i]);
						
						if(jerk < 0 || jerk > 5001)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//speed
					if(i == 4) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						speed = (uint32_t)convert_string_to_long_int(&array_of_commands[i]);
						
						if(speed < 0 || speed > 3000000)
							return SMD_RETURN_INVALID_PARAMETER;
					}
				}
				
				//figure out the direction
				if(strstr(input, JOG_CCW) !=NULL)
					direction = 1;
				
				//tell the motor to jog
				if(SMD_jog(direction, accel, decel, jerk, speed) < 0)
					return SMD_RETURN_INVALID_PARAMETER;
				else
					return SMD_RETURN_COMMAND_SUCCESS;
				
			}
		}
		
		else if(strstr(input, FIND_HOME_CW) !=NULL || strstr(input, FIND_HOME_CCW) !=NULL) {
			
			//search for the home substring - if it is found, then parse out:
			// accel
			// decel
			// steps/s
			
			char array_of_commands[5] = {0};
			
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
				
				tokenize_client_input(array_of_commands, input, num_tokens);
				
				//loop through the input and convert to int
				for(i=0; i<5; i++) {
					
					//skip the actual command
					//speed
					if(i == 1) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						speed = (uint32_t)convert_string_to_long_int(&array_of_commands[i]);
						
						if(speed < 0 || speed > 3000000)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//accel
					if(i == 2) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						accel = convert_string_to_long_int(&array_of_commands[i]);
						
						if(accel < 0 || accel > 5001)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//decel
					if(i == 3) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						decel = convert_string_to_long_int(&array_of_commands[i]);
						
						if(decel < 0 || decel > 5001)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//jerk
					if(i == 4) {
						//fprintf(stderr, "converting %s\n", array_of_commands[i]);
						jerk = convert_string_to_long_int(&array_of_commands[i]);
						
						if(jerk < 0 || jerk > 5001)
							return SMD_RETURN_INVALID_PARAMETER;
					}
				}
				
				//figure out the direction
				if(strstr(input, FIND_HOME_CCW) !=NULL)
					direction = 1;
				
				//tell the motor to jog
				if(SMD_find_home(direction, speed, accel, decel, jerk) < 0)
					return SMD_RETURN_INVALID_PARAMETER;
				else
					return SMD_RETURN_COMMAND_SUCCESS;
				
			}
		}
		
		else if(strncmp(input, DISCONNECT, strlen(DISCONNECT)) == 0) {
			
			SMD_close_command_connection();
			return SMD_RETURN_DISCONNECT_SUCCESS;
		}
		
		else if(strncmp(input, SAVE_CONFIG_TO_DRIVE, strlen(SAVE_CONFIG_TO_DRIVE)) == 0) {
			
			char array_of_commands[8] = {0};
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
				
				tokenize_client_input(array_of_commands, input, num_tokens);
				
				for(i=0; i<num_tokens; i++) {
					
					//control word
					if(i == 1) {
						control_word = convert_string_to_long_int(&array_of_commands[i]);
					}
					
					//config word
					if(i == 2) {
						config_word = convert_string_to_long_int(&array_of_commands[i]);
					}
					
					//starting speed
					if(i == 3) {
						
						starting_speed = convert_string_to_long_int(&array_of_commands[i]);
						
						if(starting_speed < 1 || starting_speed > 1999999)
							return SMD_RETURN_SAVE_CONFIG_FAIL;
					}
					
					//motor steps/turn
					if(i == 4) {
						
						steps_per_turn = convert_string_to_long_int(&array_of_commands[i]);
						
						if(steps_per_turn < 200 || steps_per_turn > 32767)
							return SMD_RETURN_SAVE_CONFIG_FAIL;
					}
					
					//encoder pulses/turn
					if(i == 5) {
						
						enc_pulses_per_turn = convert_string_to_long_int(&array_of_commands[i]);
						
						if(enc_pulses_per_turn != 1024)
							return SMD_RETURN_SAVE_CONFIG_FAIL;
					}
					
					//idle current percentage
					if(i == 6) {
						
						idle_current_percentage = convert_string_to_long_int(&array_of_commands[i]);
						
						if(idle_current_percentage < 0 || idle_current_percentage > 100)
							return SMD_RETURN_SAVE_CONFIG_FAIL;
					}
					
					//motor current
					if(i == 7) {
						
						motor_current = convert_string_to_long_int(&array_of_commands[i]);
						
						if(motor_current < 10 || motor_current > 34)
							return SMD_RETURN_SAVE_CONFIG_FAIL;
					}
				}
				
				if(SMD_set_configuration(control_word, config_word, starting_speed, steps_per_turn, enc_pulses_per_turn, idle_current_percentage, motor_current) < 0)
					return SMD_RETURN_SAVE_CONFIG_FAIL;
				else
					return SMD_RETURN_SAVE_CONFIG_SUCCESS;
			}
		}
		
		else if(strncmp(input, RELATIVE_MOVE, strlen(RELATIVE_MOVE)) == 0) {
			
			char array_of_commands[6] = {0};
			
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
				
				tokenize_client_input(array_of_commands, input, num_tokens);
				
				for(i=0; i<num_tokens; i++) {
					
					//relative position
					if(i==1) {
						rel_pos = convert_string_to_long_int(&array_of_commands[i]);
						
						if(rel_pos < -8388607 || rel_pos > 8388607)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//accel
					if(i==2) {
						accel = convert_string_to_long_int(&array_of_commands[i]);
						
						if(accel < 0 || accel > 5001)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//decel
					if(i==3) {
						decel = convert_string_to_long_int(&array_of_commands[i]);
						
						if(decel < 0 || decel > 5001)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//jerk
					if(i==4) {
						jerk = convert_string_to_long_int(&array_of_commands[i]);
						
						if(jerk < 0 || jerk > 5001)
							return SMD_RETURN_INVALID_PARAMETER;
					}
					
					//speed
					if(i==5) {
						speed = convert_string_to_long_int(&array_of_commands[i]);
						
						if(speed < 0 || speed > 2999999)
							return SMD_RETURN_INVALID_PARAMETER;
					}
				}
				
				if(SMD_relative_move(rel_pos, accel, decel, jerk, speed) < 0)
					return SMD_RETURN_COMMAND_FAILED;
				else
					return SMD_RETURN_COMMAND_SUCCESS;
			}
		}
		
		else if(strncmp(input, DRIVE_ENABLE, strlen(DRIVE_ENABLE)) == 0) {
			
			if(SMD_drive_enable() < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else
				return SMD_RETURN_ENABLE_SUCCESS;
		}
		
		else if(strncmp(input, DRIVE_DISABLE, strlen(DRIVE_DISABLE)) == 0) {
			
			if(SMD_drive_disable() < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else
				return SMD_RETURN_DISABLE_SUCCESS;
		}
		
		else if(strncmp(input, HOLD_MOVE, strlen(HOLD_MOVE)) == 0) {
			
			if(SMD_hold_move() < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else
				return SMD_RETURN_COMMAND_SUCCESS;
		}
		
		else if(strncmp(input, IMMED_STOP, strlen(IMMED_STOP)) == 0) {
			
			if(SMD_immed_stop() < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else
				return SMD_RETURN_COMMAND_SUCCESS;
		}
		
		else if(strncmp(input, RESET_ERRORS, strlen(RESET_ERRORS)) == 0) {
			
			if(SMD_reset_errors() < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else
				return SMD_RETURN_RESET_ERRORS_SUCCESS;
		}
		
		else if(strncmp(input, READ_INPUT_REGISTERS, strlen(READ_INPUT_REGISTERS)) == 0) {
			
			if(SMD_read_input_registers(cl) < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else
				return SMD_RETURN_HANDLED_BY_CLIENT;
		}
		
		else if(strncmp(input, LOAD_CURRENT_CONFIGURATION, strlen(LOAD_CURRENT_CONFIGURATION)) == 0) {
			
			if(SMD_load_current_configuration(cl) < 0)
				return SMD_RETURN_READ_CURRENT_CONFIG_FAIL;
			else
				return SMD_RETURN_READY_TO_READ_CONFIG;
		}
		
		else if(strncmp(input, READ_CURRENT_CONFIGURATION, strlen(READ_CURRENT_CONFIGURATION)) == 0) {
			
			if(SMD_read_current_configuration(cl) < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else
				return SMD_RETURN_HANDLED_BY_CLIENT;
		}
		
		else if(strncmp(input, PRESET_MOTOR_POSITION, strlen(PRESET_MOTOR_POSITION)) == 0) {
			
			char array_of_commands[2] = {0};
			
			int num_tokens = 0;
			int32_t pos = 0;

			//loop through the input and convert to int
			if((num_tokens = number_of_tokens(input)) != 2) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				tokenize_client_input(array_of_commands, input, num_tokens);
				
				pos = (int32_t)convert_string_to_long_int(&array_of_commands[1]);
				
				if(pos < -8388607 || pos > 8388607)
					return SMD_RETURN_INVALID_PARAMETER;
				
				//preset the position
				if(SMD_preset_motor_position(pos) < 0)
					return SMD_RETURN_PRESET_POS_FAIL;
				else
					return SMD_RETURN_PRESET_POS_SUCCESS;
			}
		}
		
		else if(strncmp(input, PRESET_ENCODER_POSITION, strlen(PRESET_ENCODER_POSITION)) == 0) {
			
			char array_of_commands[2] = {0};
			
			int num_tokens = 0;
			int32_t pos = 0;
			
			//loop through the input and convert to int
			if((num_tokens = number_of_tokens(input)) != 2) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {

				tokenize_client_input(array_of_commands, input, num_tokens);
				
				pos = (int32_t)convert_string_to_long_int(&array_of_commands[1]);
				
				if(pos < -8388607 || pos > 8388607)
					return SMD_RETURN_INVALID_PARAMETER;
				
				//preset the position
				if(SMD_preset_encoder_position(pos) < 0)
					return SMD_RETURN_PRESET_ENC_FAIL;
				else
					return SMD_RETURN_PRESET_ENC_SUCCESS;
			}
		}
		
		else if(strncmp(input, PROGRAM_FIRST_BLOCK, strlen(PROGRAM_FIRST_BLOCK)) == 0) {
			
			if(program_block_first_block() < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else {
				return SMD_RETURN_READY_FOR_SEGMENTS;
			}
		}
		
		else if(strncmp(input, PREPARE_FOR_NEXT_SEGMENT, strlen(PREPARE_FOR_NEXT_SEGMENT)) == 0) {
			
			if(prepare_for_next_segment() < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else
				return SMD_RETURN_SEND_NEXT_SEGMENT;
		}
		
		else if(strncmp(input, PROGRAM_MOVE_SEGMENT, strlen(PROGRAM_MOVE_SEGMENT)) == 0) {
			
			char array_of_commands[6] = {0};
			
			int num_tokens = 0;
			
			//loop through the input and convert to int
			if((num_tokens = number_of_tokens(input)) != 6) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				int i = 0;
				int32_t target_pos = 0, speed = 0;
				int16_t accel = 0, decel = 0, jerk = 0;
				
				tokenize_client_input(array_of_commands, input, num_tokens);
				
				for(i=0; i<num_tokens; i++) {
					
					if(i==1)
						target_pos = (int32_t)convert_string_to_long_int(&array_of_commands[i]);
					
					if(i==2)
						speed = (int32_t)convert_string_to_long_int(&array_of_commands[i]);
					
					if(i==3)
						accel = (int16_t)convert_string_to_long_int(&array_of_commands[i]);
					
					if(i==4)
						decel = (int16_t)convert_string_to_long_int(&array_of_commands[i]);
					
					if(i==5)
						jerk = (int16_t)convert_string_to_long_int(&array_of_commands[i]);
				}
				
				//test inputs
				if(target_pos < -8388607 || target_pos > 8388607)
					return SMD_RETURN_INVALID_PARAMETER;
				
				if(speed < 0 || speed >  2999999)
					return SMD_RETURN_INVALID_PARAMETER;
				
				if(accel < 1 || accel > 5000)
					return SMD_RETURN_INVALID_PARAMETER;
				
				if(decel < 1 || decel > 5000)
					return SMD_RETURN_INVALID_PARAMETER;
				
				if(jerk < 0 || jerk > 5000)
					return SMD_RETURN_INVALID_PARAMETER;
				
				//preset the position
				if(program_move_segment(target_pos, speed, accel, decel, jerk) < 0)
					return SMD_RETURN_COMMAND_FAILED;
				else
					return SMD_RETURN_SEGMENT_ACCEPTED;
			}
		}
		
		else if(strncmp(input, RUN_ASSEMBLED_DWELL_MOVE, strlen(RUN_ASSEMBLED_DWELL_MOVE)) == 0) {
			
			char array_of_commands[2];
			
			int num_tokens = 0;
			int32_t dwell_time = 0;
			
			//loop through the input and convert to int
			if((num_tokens = number_of_tokens(input)) != 2) {
				return SMD_RETURN_INVALID_PARAMETER;
			}
			
			else {
				
				tokenize_client_input(array_of_commands, input, num_tokens);
				
				dwell_time = (int32_t)convert_string_to_long_int(&array_of_commands[1]);
				
				if(dwell_time < 0 || dwell_time > 65535)
					return SMD_RETURN_INVALID_PARAMETER;
				
				//preset the position
				if(run_assembled_move(-1, dwell_time) < 0)
					return SMD_RETURN_COMMAND_FAILED;
				else
					return SMD_RETURN_COMMAND_SUCCESS;
			}
			
		}
		
		//the client submitted a command that was not recognized
		else {
			return SMD_RETURN_INVALID_INPUT;
		}
	}
	
	//tells the client that the SMD is not connected
	else {
		return SMD_RETURN_NO_ROUTE_TO_HOST;
	}
	
}

void parse_smd_response_to_client_input(int smd_response, char *input, int fd, int cl) {
	
	//successful run - tell the client
	if((smd_response=parse_socket_input(input,cl)) == SMD_RETURN_COMMAND_SUCCESS ) {
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
	
	//entered assembled move mode successfully - ready for programmed segments
	if(smd_response == SMD_RETURN_READY_FOR_SEGMENTS) {
		write_to_client(cl, READY_FOR_SEGMENTS);
	}
	
	//tell the client to send the next segment
	if(smd_response == SMD_RETURN_SEND_NEXT_SEGMENT) {
		write_to_client(cl, SEND_NEXT_SEGMENT);
	}
	
	//segment programming successful
	if(smd_response == SMD_RETURN_SEGMENT_ACCEPTED) {
		write_to_client(cl, SEGMENT_ACCEPTED);
	}
}