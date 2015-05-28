//
//  SMDControlDaemon.c
//  AMCI_SMD
//
//  Created by John Davis on 4/16/15.
//
//

//Compile with:
//gcc -I/usr/local/include/modbus -L/usr/local/lib -Wl,-rpath=/usr/local/lib -Wall -lmodbus

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <modbus.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <syslog.h>
#include <sys/stat.h>
#include "assert.h"

#include "SMD_Constants.h"
#include "SMD_Utilities.h"

char DEVICE_IP[32];

modbus_t *smd_command_connection = NULL;
int configMode = 0;

//function declarations - server setup
void daemonize();
void open_server_socket();

//motor connection functions
int parse_socket_input(char *input, int cl);
void parse_smd_response(int response, char *input, int fd, int cl);
int open_smd_command_connection();
void close_smd_command_connection();

//function declarations - motor commands
int read_input_registers(int cl);
int read_current_configuration(int cl);
int jog(int direction, int16_t accel, int16_t decel, int16_t jerk, int32_t speed);
int relative_move(int32_t rel_pos, int16_t accel, int16_t decel, int16_t jerk, int32_t speed);
int drive_enable();
int drive_disable();
int reset_errors();
int immed_stop();
int hold_move();
int preset_encoder_position(int32_t pos);
int preset_motor_position(int32_t pos);
int set_configuration_mode();
int set_command_mode();
int set_configuration(int32_t control_word, int32_t config_word, int32_t starting_speed, int16_t steps_per_turn, int16_t enc_pulses_per_turn, int16_t idle_current_percentage, int16_t motor_current);

int main(int argc, char *argv[]) {
	
	daemonize();
	
	while(1) {
		//open the server socket
		open_server_socket();
	}
	
	return 0;
}


/* Functions - Socket Control and Command */

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
	addr.sin_port = htons(7000);
	
	int yes = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
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
		
		if((rc=read(cl,buf,sizeof(buf))) > 0) {
			parse_smd_response(rc, buf, fd, cl);
		}
		
		//close the socket after each client disconnects
		if(rc == 0) {
			configMode = 0;
			close(cl);
			close_smd_command_connection();
			
			//start a new socket connection accept
			if ( (cl = accept(fd, NULL, NULL)) == -1) {
				perror("Socket Accept Error");
			}
		}
	}
}

int parse_socket_input(char *input, int cl) {
	
	if(strncmp(input, CONNECT, strlen(CONNECT)-1) == 0) {
		
		//we should only have one argument - an IP of the SMD itself
		char *token, *string, *tofree;
		char *array_of_commands[2];
		
		int num_tokens = 0;			//how many tokens do we have?
		char *smd_ip = NULL;
		
		//reset our values
		tofree = string = strdup(input);
		
		//check bounds first - we should only have 5 tokens
		while ((token = strsep(&string, ",")) != NULL) {
			num_tokens++;
		}
		
		free(tofree);
		
		//should have 2 commands only (connect and device ip)
		if(num_tokens != 2) {
			return SMD_RETURN_INVALID_PARAMETER;
		}
		
		//now that we are sure that we have an ip, parse it out
		else {
			
			int i;
			num_tokens = 0;
			tofree = string = strdup(input);
			
			//re-tokenize the input
			while ((token = strsep(&string, ",")) != NULL) {
				array_of_commands[num_tokens] = (char*)malloc(sizeof(char) * (strlen(token) + 1 ) );
				strcpy(array_of_commands[num_tokens], token);
				num_tokens++;
			}
			
			free(tofree);
			
			//loop through the input and convert to int
			for(i=0; i<2; i++) {
				
				//skip the actual command, "connect"
				//just get the ip address
				if(i == 1) {
					smd_ip = malloc(sizeof(char) * (strlen(array_of_commands[i]))+1);
					
					if(smd_ip)
						strncpy(smd_ip, array_of_commands[i], strlen(array_of_commands[i]));
				}
			}
			
			for(i=0; i<num_tokens; i++) {
				free(array_of_commands[i]);
			}
			
			//now try and connect
			if(smd_ip) {
				
				strncpy(DEVICE_IP, smd_ip, strlen(smd_ip));
				
				if( open_smd_command_connection() == -1) {
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
	
	else if(strncmp(input, DISCONNECT, strlen(DISCONNECT)-1) == 0) {
		
		close_smd_command_connection();
		return SMD_RETURN_DISCONNECT_SUCCESS;
	}
	
	else if(strncmp(input, SAVE_CONFIG_TO_DRIVE, strlen(SAVE_CONFIG_TO_DRIVE)-1) == 0) {
		
		char *token, *string, *tofree;
		char *array_of_commands[8];
		
		int num_tokens = 0;
		int32_t control_word = 0;
		int32_t config_word = 0;
		int32_t starting_speed = 0;
		int16_t steps_per_turn = 0;
		int16_t enc_pulses_per_turn = 0;
		int16_t idle_current_percentage = 0;
		int16_t motor_current = 0;
		tofree = string = strdup(input);
		
		//re-tokenize the input
		while ((token = strsep(&string, ",")) != NULL) {
			array_of_commands[num_tokens] = (char*)malloc(sizeof(char) * (strlen(token) + 1 ) );
			strcpy(array_of_commands[num_tokens], token);
			num_tokens++;
		}
		
		free(tofree);
		
		if(num_tokens == 8) {
			
			int i;
			
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
					
					if(starting_speed < 1 || starting_speed > 1999999)
						return SMD_RETURN_SAVE_CONFIG_FAIL;
				}
				
				//motor steps/turn
				if(i == 4) {
					
					steps_per_turn = convert_string_to_long_int(array_of_commands[i]);
					
					if(steps_per_turn < 200 || steps_per_turn > 32767)
						return SMD_RETURN_SAVE_CONFIG_FAIL;
				}
				
				//encoder pulses/turn
				if(i == 5) {
					
					enc_pulses_per_turn = convert_string_to_long_int(array_of_commands[i]);
					
					if(enc_pulses_per_turn != 1024)
						return SMD_RETURN_SAVE_CONFIG_FAIL;
				}
				
				//idle current percentage
				if(i == 6) {
					
					idle_current_percentage = convert_string_to_long_int(array_of_commands[i]);
					
					if(idle_current_percentage < 0 || idle_current_percentage > 100)
						return SMD_RETURN_SAVE_CONFIG_FAIL;
				}
				
				//motor current
				if(i == 7) {
					
					motor_current = convert_string_to_long_int(array_of_commands[i]);
					
					if(motor_current < 10 || motor_current > 34)
						return SMD_RETURN_SAVE_CONFIG_FAIL;
				}
			}
			
			//clean-up
			int j;
			
			for(j=0; j<num_tokens; j++) {
				free(array_of_commands[j]);
			}
			
			if(set_configuration(control_word, config_word, starting_speed, steps_per_turn, enc_pulses_per_turn, idle_current_percentage, motor_current) < 0)
				return SMD_RETURN_SAVE_CONFIG_FAIL;
			else
				return SMD_RETURN_SAVE_CONFIG_SUCCESS;
		}
		
		else {
			
			//clean-up
			int j;
			
			for(j=0; j<num_tokens; j++) {
				free(array_of_commands[j]);
			}
			
			return SMD_RETURN_SAVE_CONFIG_FAIL;
		}
	}
	
	else if(strstr(input, JOG_CW) !=NULL || strstr(input, JOG_CCW) !=NULL) {
		
		//search for the jog substring - if it is found, then parse out:
		// accel
		// decel
		// steps/s
		
		char *token, *string, *tofree;
		char *array_of_commands[5];
		
		int num_tokens = 0;			//how many tokens do we have?
		int direction = 0;			//0=CW, 1=CCW
		int16_t accel = 0;
		int16_t decel = 0;
		int16_t jerk = 0;
		int32_t speed = 0;
		
		tofree = string = strdup(input);
		
		//check bounds first - we should only have 5 tokens
		while ((token = strsep(&string, ",")) != NULL) {
			num_tokens++;
		}
		
		free(tofree);
		
		//should have 5 commands only
		if(num_tokens == 5) {
			
			int i;
			num_tokens = 0;
			string = strdup(input);
			
			//re-tokenize the input
			while ((token = strsep(&string, ",")) != NULL) {
				array_of_commands[num_tokens] = (char*)malloc(sizeof(char) * (strlen(token) + 1 ) );
				strcpy(array_of_commands[num_tokens], token);
				num_tokens++;
			}
			
			//loop through the input and convert to int
			for(i=0; i<5; i++) {
				
				//skip the actual command, "jogCW"
				//accel
				if(i == 1) {
					//fprintf(stderr, "converting %s\n", array_of_commands[i]);
					accel = convert_string_to_long_int(array_of_commands[i]);
					
					if(accel < 0 || accel > 5001)
						return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//decel
				if(i == 2) {
					//fprintf(stderr, "converting %s\n", array_of_commands[i]);
					decel = convert_string_to_long_int(array_of_commands[i]);
					
					if(decel < 0 || decel > 5001)
						return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//jerk
				if(i == 3) {
					//fprintf(stderr, "converting %s\n", array_of_commands[i]);
					jerk = convert_string_to_long_int(array_of_commands[i]);
					
					if(jerk < 0 || jerk > 5001)
						return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//speed
				if(i == 4) {
					//fprintf(stderr, "converting %s\n", array_of_commands[i]);
					speed = (uint32_t)convert_string_to_long_int(array_of_commands[i]);
					
					if(speed < 0 || speed > 3000000)
						return SMD_RETURN_INVALID_PARAMETER;
				}
			}
			
			//figure out the direction
			if(strstr(input, JOG_CCW) !=NULL)
				direction = 1;
			
			//tell the motor to jog
			if(jog(direction, accel, decel, jerk, speed) < 0)
				return SMD_RETURN_INVALID_PARAMETER;
			else
				return SMD_RETURN_COMMAND_SUCCESS;

		}
		
		//now that we are sure that we have 5 commands, parse them out
		else
			return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(strncmp(input, RELATIVE_MOVE, strlen(RELATIVE_MOVE)-1) == 0) {
		
		char *token, *string, *tofree;
		char *array_of_commands[6];
		
		int num_tokens = 0;
		int32_t rel_pos = 0;
		int32_t speed = 0;
		int16_t accel = 0;
		int16_t decel = 0;
		int16_t jerk = 0;
		tofree = string = strdup(input);
		
		//re-tokenize the input
		while ((token = strsep(&string, ",")) != NULL) {
			array_of_commands[num_tokens] = (char*)malloc(sizeof(char) * (strlen(token) + 1 ) );
			strcpy(array_of_commands[num_tokens], token);
			num_tokens++;
		}
		
		free(tofree);
		
		//loop through the input and convert to int
		if(num_tokens == 6) {
			
			int i;
			
			for(i=0; i<num_tokens; i++) {
				
				//relative position
				if(i==1) {
					rel_pos = convert_string_to_long_int(array_of_commands[i]);
					
					if(rel_pos < -8388607 || rel_pos > 8388607)
						return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//accel
				if(i==2) {
					accel = convert_string_to_long_int(array_of_commands[i]);
					
					if(accel < 0 || accel > 5001)
						return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//decel
				if(i==3) {
					decel = convert_string_to_long_int(array_of_commands[i]);
					
					if(decel < 0 || decel > 5001)
						return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//jerk
				if(i==4) {
					jerk = convert_string_to_long_int(array_of_commands[i]);
					
					if(jerk < 0 || jerk > 5001)
						return SMD_RETURN_INVALID_PARAMETER;
				}
				
				//speed
				if(i==5) {
					speed = convert_string_to_long_int(array_of_commands[i]);
					
					if(speed < 0 || speed > 2999999)
						return SMD_RETURN_INVALID_PARAMETER;
				}
			}
			
			//clean-up
			int j;
			
			for(j=0; j<num_tokens; j++) {
				free(array_of_commands[j]);
			}
			
			if(relative_move(rel_pos, accel, decel, jerk, speed) < 0)
				return SMD_RETURN_COMMAND_FAILED;
			else
				return SMD_RETURN_COMMAND_SUCCESS;
		}
		
		else {
			
			//clean-up
			int j;
			
			for(j=0; j<num_tokens; j++) {
				free(array_of_commands[j]);
			}
			
			return SMD_RETURN_INVALID_PARAMETER;
		}
		
	}
	
	else if(strncmp(input, DRIVE_ENABLE, strlen(DRIVE_ENABLE)-1) == 0) {
		
		if(drive_enable() < 0)
			return SMD_RETURN_COMMAND_FAILED;
		else
			return SMD_RETURN_ENABLE_SUCCESS;
	}
	
	else if(strncmp(input, DRIVE_DISABLE, strlen(DRIVE_DISABLE)-1) == 0) {
		
		if(drive_disable() < 0)
			return SMD_RETURN_COMMAND_FAILED;
		else
			return SMD_RETURN_DISABLE_SUCCESS;
	}
	
	else if(strncmp(input, HOLD_MOVE, strlen(HOLD_MOVE)-1) == 0) {
		
		if(hold_move() < 0)
			return SMD_RETURN_COMMAND_FAILED;
		else
			return SMD_RETURN_COMMAND_SUCCESS;
	}
	
	else if(strncmp(input, IMMED_STOP, strlen(IMMED_STOP)-1) == 0) {
		
		if(immed_stop() < 0)
			return SMD_RETURN_COMMAND_FAILED;
		else
			return SMD_RETURN_COMMAND_SUCCESS;
	}
	
	else if(strncmp(input, RESET_ERRORS, strlen(RESET_ERRORS)-1) == 0) {
		
		if(reset_errors() < 0)
			return SMD_RETURN_COMMAND_FAILED;
		else
			return SMD_RETURN_COMMAND_SUCCESS;
	}

	else if(strncmp(input, READ_INPUT_REGISTERS, strlen(READ_INPUT_REGISTERS)-1) == 0) {
		
		if(read_input_registers(cl) < 0)
			return SMD_RETURN_COMMAND_FAILED;
		else
			return SMD_RETURN_HANDLED_BY_CLIENT;
	}
	
	else if(strncmp(input, READ_CURRENT_CONFIGURATION, strlen(READ_CURRENT_CONFIGURATION)-1) == 0) {
		
		if(read_current_configuration(cl) < 0)
			return SMD_RETURN_READ_CURRENT_CONFIG_FAIL;
		else
			return SMD_RETURN_HANDLED_BY_CLIENT;
	}
	
	else if(strncmp(input, PRESET_MOTOR_POSITION, strlen(PRESET_MOTOR_POSITION)-1) == 0) {
		
		char *token, *string, *tofree;
		char *array_of_commands[2];
		
		int num_tokens = 0;			//how many tokens do we have?
		int32_t pos;
	
		num_tokens = 0;
		tofree = string = strdup(input);
		
		//re-tokenize the input
		while ((token = strsep(&string, ",")) != NULL) {
			array_of_commands[num_tokens] = (char*)malloc(sizeof(char) * (strlen(token) + 1 ) );
			strcpy(array_of_commands[num_tokens], token);
			num_tokens++;
		}
		
		free(tofree);
		
		//loop through the input and convert to int
		if(num_tokens == 2) {
			
			int i;
			pos = (int32_t)convert_string_to_long_int(array_of_commands[1]);
			
			//clean-up
			for(i=0; i<num_tokens; i++) {
				free(array_of_commands[i]);
			}
			
			if(pos < -8388607 || pos > 8388607)
				return SMD_RETURN_INVALID_PARAMETER;
			
			//preset the position
			if(preset_motor_position(pos) < 0)
				return SMD_RETURN_PRESET_POS_FAIL;
			else
				return SMD_RETURN_PRESET_POS_SUCCESS;
		}
		
		else {
			
			//clean-up
			int i;
			
			for(i=0; i<num_tokens; i++) {
				free(array_of_commands[i]);
			}
			
			return SMD_RETURN_INVALID_PARAMETER;
		}
	}
	
	else if(strncmp(input, PRESET_ENCODER_POSITION, strlen(PRESET_ENCODER_POSITION)-1) == 0) {
		
		char *token, *string, *tofree;
		char *array_of_commands[2];
		
		int num_tokens = 0;			//how many tokens do we have?
		int32_t pos;
		
		num_tokens = 0;
		tofree = string = strdup(input);
		
		//re-tokenize the input
		while ((token = strsep(&string, ",")) != NULL) {
			array_of_commands[num_tokens] = (char*)malloc(sizeof(char) * (strlen(token) + 1 ) );
			strcpy(array_of_commands[num_tokens], token);
			num_tokens++;
		}
		
		free(tofree);
		
		//loop through the input and convert to int
		if(num_tokens == 2) {
			
			int j;
			pos = (int32_t)convert_string_to_long_int(array_of_commands[1]);
			
			//clean-up
			for(j=0; j<num_tokens; j++) {
				free(array_of_commands[j]);
			}
			
			if(pos < -8388607 || pos > 8388607)
				return SMD_RETURN_INVALID_PARAMETER;
			
			//preset the position
			if(preset_encoder_position(pos) < 0)
				return SMD_RETURN_PRESET_ENC_FAIL;
			else
				return SMD_RETURN_PRESET_ENC_SUCCESS;
		}
		
		else {
			
			//clean-up
			int j;
			
			for(j=0; j<num_tokens; j++) {
				free(array_of_commands[j]);
			}
			
			return SMD_RETURN_INVALID_PARAMETER;
		}
	}
	
	//invalid input/command
	else {
		return SMD_RETURN_INVALID_INPUT;
	}
}

void parse_smd_response(int smd_response, char *input, int fd, int cl) {
	
	//successful run - tell the client
	if((smd_response=parse_socket_input(input,cl)) == SMD_RETURN_COMMAND_SUCCESS ) {
		
		//Send the message back to client
		if(write(cl, COMMAND_SUCCESS , sizeof(COMMAND_SUCCESS)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//succesful connect to SMD
	if(smd_response == SMD_RETURN_CONNECT_SUCCESS) {
		//Send the message back to client
		if(write(cl, SMD_CONNECT_SUCCESS , sizeof(SMD_CONNECT_SUCCESS)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//outputting string for parsing by client - don't do anything - the helper function handles the client write
	if(smd_response == SMD_RETURN_HANDLED_BY_CLIENT) {
		//do nothing
	}
	
	//successful SMD enable
	if(smd_response == SMD_RETURN_ENABLE_SUCCESS) {
		if(write(cl, ENABLE_SUCCESS, sizeof(ENABLE_SUCCESS)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//successful SMD disable
	if(smd_response == SMD_RETURN_DISABLE_SUCCESS) {
		if(write(cl, DISABLE_SUCCESS, sizeof(DISABLE_SUCCESS)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//command borked
	if(smd_response == SMD_RETURN_COMMAND_FAILED){
		if(write(cl, COMMAND_ERROR , sizeof(COMMAND_ERROR)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//invalid input
	if(smd_response == SMD_RETURN_INVALID_INPUT) {
		if(write(cl, INVALID_INPUT , sizeof(INVALID_INPUT)) == -1) {
			perror("Error writing to client");
		}
		
		close(cl);
		
		//start a new socket connection accept
		if ( (cl = accept(fd, NULL, NULL)) == -1) {
			perror("Socket Accept Error");
		}
	}
	
	//no route to SMD - can't connect
	if(smd_response == SMD_RETURN_NO_ROUTE_TO_HOST){
		if(write(cl, NO_ROUTE_TO_SMD , sizeof(NO_ROUTE_TO_SMD)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//invalid parameter, such as in a jog function
	if(smd_response == SMD_RETURN_INVALID_PARAMETER){
		if(write(cl, INVALID_PARAMETER , sizeof(INVALID_PARAMETER)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//preset encoder success
	if(smd_response == SMD_RETURN_PRESET_ENC_SUCCESS) {
		if(write(cl, PRESET_ENCODER_SUCCESS , sizeof(PRESET_ENCODER_SUCCESS)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//preset encoder fail
	if(smd_response == SMD_RETURN_PRESET_ENC_FAIL) {
		if(write(cl, PRESET_ENCODER_FAIL , sizeof(PRESET_ENCODER_FAIL)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//preset motor success
	if(smd_response == SMD_RETURN_PRESET_POS_SUCCESS) {
		if(write(cl, PRESET_POSITION_SUCCESS , sizeof(PRESET_POSITION_SUCCESS)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//preset motor fail
	if(smd_response == SMD_RETURN_PRESET_POS_FAIL) {
		if(write(cl, PRESET_POSITION_FAIL , sizeof(PRESET_POSITION_FAIL)) == -1) {
			perror("Error writing to client");
		}
	}
	
	//save config to drive success
	if(smd_response == SMD_RETURN_SAVE_CONFIG_SUCCESS) {
		
		if(write(cl, CONFIG_SAVE_SUCCESS, sizeof(CONFIG_SAVE_SUCCESS)) == -1) {
			fprintf(stderr, "wrote to client");
			perror("Error writing to client");
		}
	}
	
	//save config to drive fail
	if(smd_response == SMD_RETURN_SAVE_CONFIG_FAIL) {
		
		if(write(cl, CONFIG_SAVE_FAIL, sizeof(CONFIG_SAVE_FAIL)) == -1) {
			fprintf(stderr, "wrote to client");
			perror("Error writing to client");
		}
	}
	
	//read current config fail
	if(smd_response == SMD_RETURN_READ_CURRENT_CONFIG_FAIL) {
		
		if(write(cl, GET_CURRENT_CONFIG_FAIL, sizeof(GET_CURRENT_CONFIG_FAIL)) == -1) {
			fprintf(stderr, "wrote to client");
			perror("Error writing to client");
		}
	}
}

int open_smd_command_connection() {
	
	smd_command_connection = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if(strlen(DEVICE_IP) == 0 || modbus_connect(smd_command_connection) == -1) {
		modbus_free(smd_command_connection);
		return -1;
	}
	else {
		return 0;
	}
	
	return 0;
}

void close_smd_command_connection() {
	
	if( smd_command_connection != NULL) {
		modbus_close(smd_command_connection);
		modbus_free(smd_command_connection);
		smd_command_connection = NULL;
	}
}


/* Functions - Program Command */

void daemonize() {
	pid_t pid;
	
	/* Fork off the parent process */
	pid = fork();
	
	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);
	
	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);
	
	/* On success: The child process becomes session leader */
	if (setsid() < 0)
		exit(EXIT_FAILURE);
	
	/* Catch, ignore and handle signals */
	//TODO: Implement a working signal handler */
	//signal(SIGCHLD, SIG_IGN);
	//signal(SIGHUP, SIG_IGN);
	
	/* Fork off for the second time*/
	pid = fork();
	
	/* An error occurred */
	if (pid < 0)
		exit(EXIT_FAILURE);
	
	/* Success: Let the parent terminate */
	if (pid > 0)
		exit(EXIT_SUCCESS);
	
	/* Set new file permissions */
	umask(0);
	
	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/");
	
	/* Close all open file descriptors */
	int8_t x;
	for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
	{
		close (x);
	}
	
	/* Open the log file */
	//openlog("SMD Server", (LOG_CONS|LOG_PERROR|LOG_PID), LOG_DAEMON);
}


/* Functions - Motor Commands */

int read_input_registers(int cl) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	
	else {
		
		int rc, i;
		uint16_t tab_reg[64];
		char client_write_string[64];
		
		memset(&tab_reg, 0, sizeof(tab_reg));
		memset(&client_write_string, 0, sizeof(client_write_string));
		
		if( (rc=modbus_read_registers(ctx, 0, 10, tab_reg)) == -1 ) {
			perror("Error reading input registers");
			return -1;
			
		}
		
		else {
			for( i=0; i < rc; i++ ) {
				//fprintf(stderr, "reg[%d]=\t\t\t%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
				
				char temp[16];
				
				memset(&temp, 0, sizeof(temp));
				
				//only add leading 0x for items that are legitimately in hex
				if(i == 0 || i == 1)
					snprintf(temp, 16, "0x%X", tab_reg[i]);
				else
					snprintf(temp, 16, "%d", tab_reg[i]);
				
				if(i==0) {
					snprintf(client_write_string, sizeof(client_write_string), ",,%s", temp);
				}
				
				else {
					snprintf(client_write_string, sizeof(client_write_string), "%s,%s", client_write_string, temp);
				}
			}
			
			//close the string with a linebreak so that the data is sent
			snprintf(client_write_string, sizeof(client_write_string), "%s%s", client_write_string, "\n");
			
			//write the registers to the client
			if(write(cl, client_write_string , sizeof(client_write_string)) == -1) {
				perror("Error writing to client");
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int read_current_configuration(int cl) {
	
	//put the drive into configuration mode - if it doesn't work, fail.
	if( set_configuration_mode() < 0) {
		
		sleep(1);
		set_command_mode();
		return -1;
	}
	
	else {
		
		//lets the connection clean-up from set_configuration_mode
		sleep(3);
		
		modbus_t *ctx = NULL;
		ctx = modbus_new_tcp(DEVICE_IP, 502);
		
		//try and connect to see what happens
		if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1) {
			
			fprintf(stderr, "Connection failed when trying to read config: %s\n", modbus_strerror(errno));
			
			modbus_close(ctx);
			modbus_free(ctx);
			
			sleep(1);
			set_command_mode();
			
			return -3;
		}
		
		else {
			
			int rc, i;
			uint16_t tab_reg[64];
			char client_write_string[64];
			
			memset(&tab_reg, 0, sizeof(tab_reg));
			memset(&client_write_string, 0, sizeof(client_write_string));
			
			if( (rc=modbus_read_registers(ctx, 0, 10, tab_reg)) == -1 ) {
				perror("Error reading input registers");
				return -1;
				
			}
			
			else {
				for( i=0; i < rc; i++ ) {
					//fprintf(stderr, "reg[%d]=\t\t\t%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
					
					char temp[16];
					
					memset(&temp, 0, sizeof(temp));
					
					//only add leading 0x for items that are legitimately in hex
					if(i == 0 || i == 1)
						snprintf(temp, 16, "0x%X", tab_reg[i]);
					else
						snprintf(temp, 16, "%d", tab_reg[i]);
					
					if(i==0) {
						snprintf(client_write_string, sizeof(client_write_string), ",,,%s", temp);
					}
					
					else {
						snprintf(client_write_string, sizeof(client_write_string), "%s,%s", client_write_string, temp);
					}
				}
				
				//close the string with a linebreak so that the data is sent
				snprintf(client_write_string, sizeof(client_write_string), "%s%s", client_write_string, "\n");
				
				fprintf(stderr, "config registers: %s\n", client_write_string);
				
				//write the registers to the client
				if(write(cl, client_write_string , sizeof(client_write_string)) == -1) {
					perror("Error writing to client");
				}
			}
			
			//modbus_flush(ctx);
			modbus_close(ctx);
			modbus_free(ctx);
			
			//put the drive back into command mode
			sleep(1);
			set_command_mode();
			return 0;
		}

	}
}

int jog(int direction, int16_t accel, int16_t decel, int16_t jerk, int32_t speed) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc,i;
		int direc = direction == 0 ? 128 : 256;
		int16_t speed_UW = 0;
		int16_t speed_LW = 0;
		
		//determine our speed upper words and lower words
		if(speed / 1000 > 0) {
			speed_LW = speed % 1000;
			speed_UW = (speed - (speed % 1000)) / 1000;
		}
		
		else {
			speed_LW = speed;
		}
		
		//write the registers
		int registers[9] = {1025, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1024};
		int values[9] = {32768, 0, speed_UW, speed_LW, accel, decel, 0, jerk, direc};
		
		for(i=0; i<9; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int relative_move(int32_t rel_pos, int16_t accel, int16_t decel, int16_t jerk, int32_t speed) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc,i;
		int16_t speed_UW = 0;
		int16_t speed_LW = 0;
		int16_t pos_UW = 0;
		int16_t pos_LW = 0;
		
		//determine our speed upper words and lower words
		if(speed / 1000 > 0) {
			speed_LW = speed % 1000;
			speed_UW = (speed - (speed % 1000)) / 1000;
		}
		
		else {
			speed_LW = speed;
		}
		
		//determine our position upper words and lower words
		if(rel_pos / 1000 > 0) {
			pos_LW = rel_pos % 1000;
			pos_UW = (rel_pos - (rel_pos % 1000)) / 1000;
		}
		
		else {
			pos_LW = rel_pos;
		}
	
		//write the registers
		int registers[9] = {1025, 1026, 1027, 1028, 1029, 1030, 1031, 1033, 1024};
		int values[9] = {32768, pos_UW, pos_LW, speed_UW, speed_LW, accel, decel, jerk, 2};
		
		for(i=0; i<9; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int drive_enable() {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc, i;
		
		//write the registers
		int registers[2] = {1025, 1024};
		int values[2] =	{32768, 0};
		
		for(i=0; i<2; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int drive_disable() {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc, i;
		
		//write the registers
		int registers[2] = {1025, 1024};
		int values[2] =	{0, 0};
		
		for(i=0; i<2; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int hold_move() {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc, i;
		
		//write the registers
		int registers[2] = {1025, 1024};
		int values[2] =	{32768, 4};
		
		for(i=0; i<2; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int immed_stop() {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc, i;
		
		//write the registers
		int registers[2] = {1025, 1024};
		int values[2] =	{32768, 16};
		
		for(i=0; i<2; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int reset_errors() {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc, i;
		
		//write the registers
		int registers[10] = {1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033};
		int values[10] = {1024, 32768, 0, 0, 0, 0, 0, 0, 0, 0};
		
		for(i=0; i<10; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}

}

int preset_encoder_position(int32_t pos) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc,i;
		struct Words returnWords = convert_int_to_words(pos);
		
		//write the registers
		int registers[4] = {1025, 1026, 1027, 1024};
		int values[4] = {32768, returnWords.upper_word, returnWords.lower_word, 16384};
		
		for(i=0; i<4; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int preset_motor_position(int32_t pos) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc,i;
		struct Words returnWords = convert_int_to_words(pos);
		
		//write the registers
		int registers[4] = {1025, 1026, 1027, 1024};
		int values[4] = {32768, returnWords.upper_word, returnWords.lower_word, 512};
		
		for(i=0; i<4; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int set_configuration_mode() {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc, i;
		
		int registers[2] = {1025, 1024};
		int values[2] =	{34816, 32768};
		
		for(i=0; i<2; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		
		return 0;
	}
}

int set_command_mode() {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	else {
		
		int rc, i;
		
		int registers[2] = {1025, 1024};
		int values[2] =	{32768, 0};
		
		for(i=0; i<2; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}

}

int set_configuration(int32_t control_word, int32_t config_word, int32_t starting_speed, int16_t steps_per_turn, int16_t enc_pulses_per_turn, int16_t idle_current_percentage, int16_t motor_current) {
	
	//put the drive into configuration mode - if it doesn't work, fail.
	if( set_configuration_mode() < 0) {
		
		sleep(1);
		set_command_mode();
		return -1;
	}
	
	else {
		
		//lets the connection clean-up from set_configuration_mode
		sleep(1);
		
		modbus_t *ctx = NULL;
		ctx = modbus_new_tcp(DEVICE_IP, 502);
		
		//try and connect to see what happens - we also have to be in config mode
		if( (strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1)) {
			
			fprintf(stderr, "Connection failed when trying to read config: %s\n", modbus_strerror(errno));
			
			modbus_close(ctx);
			modbus_free(ctx);
			
			sleep(1);
			set_command_mode();
			return -3;
		}
		
		else {
			
			int rc, i;
			int16_t starting_speed_UW = 0;
			int16_t starting_speed_LW = 0;
			
			//calc starting speed
			if(starting_speed / 1000 > 0) {
				starting_speed_LW = starting_speed % 1000;
				starting_speed_UW = (starting_speed - (starting_speed % 1000)) / 1000;
			}
			
			else {
				starting_speed_LW = starting_speed;
			}
			
			//write the registers
			int registers[8] = {1025, 1024, 1026, 1027, 1028, 1030, 1031, 1032};
			int values[8] = {config_word, control_word, starting_speed_UW, starting_speed_LW, steps_per_turn, enc_pulses_per_turn, idle_current_percentage, motor_current};
			
			for(i=0; i<8; i++) {
				
				//fprintf(stderr, "setting register %d to %d\n", registers[i], values[i]);
				rc = modbus_write_register(ctx, registers[i], values[i]);
				
				if( rc == -1 ) {
					return -1;
				}
			}
			
			//modbus_flush(ctx);
			modbus_close(ctx);
			modbus_free(ctx);
			
			sleep(1);
			set_command_mode();
			
			return 0;
		}
	}
}
