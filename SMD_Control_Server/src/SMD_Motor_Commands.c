//
//  SMD_Motor_Commands.c
//  SMD_Control_Server
//
//  Created by John Davis on 9/30/15.
//  Copyright © 2015 3ML LLC. All rights reserved.
//

#include <modbus.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "SMD_Motor_Commands.h"
#include "SMD_Constants.h"
#include "SMD_Utilities.h"

/***** MOTOR COMMAND CONNECTION FUNCTIONS *****/

int SMD_open_command_connection() {
	
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

void SMD_close_command_connection() {
	
	if( smd_command_connection != NULL) {
		modbus_close(smd_command_connection);
		modbus_free(smd_command_connection);
		smd_command_connection = NULL;
	}
}


/***** MOTOR MOVE FUNCTIONS *****/

int SMD_read_input_registers(int cl) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	
	else {
		
		int rc, i, rk;
		uint16_t tab_status_words_reg[10];
		int16_t tab_motor_data_reg[10];
		char client_write_string[128];
		
		memset(&tab_status_words_reg, 0, sizeof(tab_status_words_reg));
		memset(&tab_motor_data_reg, 0, sizeof(tab_motor_data_reg));
		memset(&client_write_string, 0, sizeof(client_write_string));
		
		if( (rc=modbus_read_registers(ctx, 0, 10, tab_status_words_reg)) == -1 ) {
			perror("Error reading input registers");
			return -1;
			
		}
		
		if( (rk=modbus_read_registers(ctx, 0, 10, (uint16_t *)&tab_motor_data_reg)) == -1 ) {
			perror("Error reading input registers");
			return -1;
			
		}
		
		else {
			for( i=0; i < rc; i++ ) {
				//fprintf(stderr, "reg[%d]=\t\t\t%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
				
				char temp[16];
				
				memset(&temp, 0, sizeof(temp));
				
				//only add leading 0x for items that are legitimately in hex
				if(i == 0)
					snprintf(temp, 16, "%X", tab_status_words_reg[i]);
				
				else if(i == 1) {
					
					snprintf(temp, 16, "%X", tab_status_words_reg[i]);
				}
				
				else
					snprintf(temp, 16, "%d", tab_motor_data_reg[i]);
				
				if(i==0) {
					snprintf(client_write_string, sizeof(client_write_string), ",,%s", temp);
				}
				
				else {
					snprintf(client_write_string, sizeof(client_write_string), "%s,%s", client_write_string, temp);
				}
			}
			
			//close the string with a linebreak so that the data is sent
			snprintf(client_write_string, sizeof(client_write_string), "%s%s", client_write_string, "\n");
			
			//debug
			//fprintf(stderr, "%s", client_write_string);
			
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

int SMD_load_current_configuration(int cl) {
	
	//Step 1 - Write the current configuration into the input registers (this puts the drive into configuration mode)
	
	//Step 2 - Tell the client that the registers are ready to be read
	
	//Step 3 - The client will then make a single call to read_input_registers to parse the configuration
	
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
		
		int registers[10] = {1025, 1024, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033};
		int values[10] =	{34816, 32768, 0, 0, 0, 0, 0, 0, 0, 0};
		
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

int SMD_read_current_configuration(int cl) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1) {
		
		fprintf(stderr, "Connection failed when trying to read config: %s\n", modbus_strerror(errno));
		
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
					snprintf(client_write_string, sizeof(client_write_string), "###%s", temp);
				}
				
				else {
					snprintf(client_write_string, sizeof(client_write_string), "%s,%s", client_write_string, temp);
				}
			}
			
			//close the string with a linebreak so that the data is sent
			snprintf(client_write_string, sizeof(client_write_string), "%s%s", client_write_string, "\n");
			
			//fprintf(stderr, "config registers: %s\n", client_write_string);
			
			//write the registers to the client
			if(write(cl, client_write_string , sizeof(client_write_string)) == -1) {
				perror("Error writing to client");
			}
		}
		
		//modbus_flush(ctx);
		modbus_close(ctx);
		modbus_free(ctx);
		
		return 0;
	}
}

int SMD_jog(int direction, int16_t accel, int16_t decel, int16_t jerk, int32_t speed) {
	
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
		int registers[10] = {1024, 1025, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1024};
		int values[10] = {0, 32768, 0, speed_UW, speed_LW, accel, decel, 0, jerk, direc};
		
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

int SMD_relative_move(int32_t rel_pos, int16_t accel, int16_t decel, int16_t jerk, int32_t speed) {
	
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
		struct Words posWords = convert_int_to_words(rel_pos);
		struct Words speedWords = convert_int_to_words(speed);
		
		//write the registers
		int registers[9] = {1025, 1026, 1027, 1028, 1029, 1030, 1031, 1033, 1024};
		int values[9] = {32768, posWords.upper_word, posWords.lower_word, speedWords.upper_word, speedWords.lower_word, accel, decel, jerk, 2};
		
		for(i=0; i<9; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		//fprintf(stderr, "relative move with:\n posUpper: %d\n pos lower: %d, speed upper: %d\n speed lower: %d\n accel: %d\n decel: %d\n jerk: %d\n", posWords.upper_word, posWords.lower_word, speedWords.upper_word, speedWords.lower_word, accel, decel, jerk);
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int SMD_drive_enable() {
	
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

int SMD_drive_disable() {
	
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

int SMD_hold_move() {
	
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
		int registers[3] = {1024, 1025, 1024};
		int values[3] =	{0, 32768, 4};
		
		for(i=0; i<3; i++) {
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

int SMD_immed_stop() {
	
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

int SMD_reset_errors() {
	
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
		int registers[3] = {1024, 1024, 1025};
		int values[3] = {0, 1024, 32768};
		
		for(i=0; i<3; i++) {
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

int SMD_preset_encoder_position(int32_t pos) {
	
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

int SMD_preset_motor_position(int32_t pos) {
	
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

int SMD_set_configuration(int32_t control_word, int32_t config_word, int32_t starting_speed, int16_t steps_per_turn, int16_t enc_pulses_per_turn, int16_t idle_current_percentage, int16_t motor_current) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens - we also have to be in config mode
	if( (strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1)) {
		
		fprintf(stderr, "Connection failed when trying to read config: %s\n", modbus_strerror(errno));
		
		modbus_close(ctx);
		modbus_free(ctx);
		
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
		
		return 0;
	}
}

int SMD_find_home(int direction, int32_t speed, int16_t accel, int16_t decel, int16_t jerk) {
	
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
		int direc = direction == 0 ? 32 : 64;
		struct Words speed_words = convert_int_to_words(speed);
		
		//write the registers
		int registers[8] = {1025, 1028, 1029, 1030, 1031, 1033, 1032, 1024};
		int values[8] = {32768, speed_words.upper_word, speed_words.lower_word, accel, decel, jerk, 0 ,direc};
		
		for(i=0; i<8; i++) {
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


/***** ASSEMBLED MOVE FUNCTIONS *****/

int program_block_first_block() {
	
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
		int values[2] =	{32768, 2048};
		
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

int prepare_for_next_segment() {
	
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
		int values[2] =	{32768, 2048};
		
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

int program_move_segment(int32_t target_pos, int32_t speed, int16_t accel, int16_t decel, int16_t jerk) {
	
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
		struct Words posWords = convert_int_to_words(target_pos);
		struct Words speedWords = convert_int_to_words(speed);
		
		//write the registers
		int registers[9] = {1025, 1026, 1027, 1028, 1029, 1030, 1031, 1033, 1024};
		int values[9] = {32768, posWords.upper_word, posWords.lower_word, speedWords.upper_word, speedWords.lower_word, accel, decel, jerk, 6144};
		
		//fprintf(stderr, "trying to program segment with:\n posUpper: %d\n pos lower: %d\n speed upper: %d\n speed lower: %d\n accel: %d\n decel: %d\n jerk: %d\n", posWords.upper_word, posWords.lower_word, speedWords.upper_word, speedWords.lower_word, accel, decel, jerk);
		
		for(i=0; i<9; i++) {
			rc = modbus_write_register(ctx, registers[i], values[i]);
			
			if( rc == -1 ) {
				return -1;
			}
		}
		
		//fprintf(stderr, "programmed move with:\n posUpper: %d\n pos lower: %d\n speed upper: %d\n speed lower: %d\n accel: %d\n decel: %d\n jerk: %d\n", posWords.upper_word, posWords.lower_word, speedWords.upper_word, speedWords.lower_word, accel, decel, jerk);
		
		modbus_close(ctx);
		modbus_free(ctx);
		return 0;
	}
}

int run_assembled_move(int16_t blend_move_direction, int32_t dwell_time) {
	
	modbus_t *ctx = NULL;
	ctx = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if( strlen(DEVICE_IP) == 0 || modbus_connect(ctx) == -1 ) {
		modbus_close(ctx);
		modbus_free(ctx);
		return -3;
	}
	
	//blend_move_direction and dwell_time cannot be simultaneously set
	else if(blend_move_direction >= 0 && dwell_time >= 0) {
		return -1;
	}
	
	else {
		
		int rc, i;
		int32_t LSW = 0;
		
		//what are we doing? blend move or dwell move?
		if(blend_move_direction >= 0)
			//0 == CW
			LSW = blend_move_direction == 0 ? 32768 : 32784;
		
		if(dwell_time >= 0)
			LSW = 33280;
		
		int registers[3] = {1025, 1033, 1024};
		int values[3] =	{LSW, dwell_time, 8192};
		
		for(i=0; i<3; i++) {
			
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