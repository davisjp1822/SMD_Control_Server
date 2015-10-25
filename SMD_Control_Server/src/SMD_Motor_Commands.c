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
#include "SMD_Utilities.h"
#include "SMD_Modbus.h"
#include "SMD_SocketOps.h"

/***** MOTOR COMMAND CONNECTION FUNCTIONS *****/

SMD_RESPONSE_CODES  SMD_open_command_connection() {
	
	smd_command_connection = modbus_new_tcp(DEVICE_IP, 502);
	
	//try and connect to see what happens
	if(strlen(DEVICE_IP) == 0 || modbus_connect(smd_command_connection) == -1) {
		
		modbus_free(smd_command_connection);
		SMD_CONNECTED = 0;
		
		return SMD_RETURN_NO_ROUTE_TO_HOST;
		
	}
	//connect successful - set SMD_CONNECTED
	else {
		
		SMD_CONNECTED = 1;
		
		return SMD_RETURN_CONNECT_SUCCESS;
	}
	
	return SMD_RETURN_UNKNOWN_ERROR;
}

void SMD_close_command_connection() {
	
	if( smd_command_connection != NULL && SMD_CONNECTED == 1) {
		
		modbus_close(smd_command_connection);
		modbus_free(smd_command_connection);
		smd_command_connection = NULL;
		SMD_CONNECTED = 0;
		free(DEVICE_IP);
	}
}


/***** MOTOR MOVE FUNCTIONS *****/

SMD_RESPONSE_CODES SMD_read_input_registers(int cl) {
	
	uint16_t tab_status_words_reg[10];
	memset(&tab_status_words_reg, 0, sizeof(tab_status_words_reg));
	
	int rc = read_modbus_registers(tab_status_words_reg, SMD_READ_INPUT_REGISTERS, cl);
	
	if(rc == SMD_RETURN_NO_ROUTE_TO_HOST || rc == SMD_RETURN_COMMAND_FAILED) {
		return SMD_RETURN_COMMAND_FAILED;
	}
	
	else {
		
		//read_modbus_registers writes the string to the client
		return SMD_RETURN_HANDLED_BY_CLIENT;
	}
	
}

SMD_RESPONSE_CODES SMD_load_current_configuration(int cl) {
	
	int registers[10] = {1025, 1024, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033};
	int values[10] =	{34816, 32768, 0, 0, 0, 0, 0, 0, 0, 0};
	
	SMD_RESPONSE_CODES response = send_modbus_command(registers, values, 10, "load_configuration");
	
	if(response == SMD_RETURN_COMMAND_SUCCESS)
		return SMD_RETURN_READY_TO_READ_CONFIG;
	else
		return SMD_RETURN_READ_CURRENT_CONFIG_FAIL;
}

SMD_RESPONSE_CODES SMD_read_current_configuration(int cl) {

	uint16_t tab_status_words_reg[10];
	memset(&tab_status_words_reg, 0, sizeof(tab_status_words_reg));
	
	int rc = read_modbus_registers(tab_status_words_reg, SMD_READ_CONFIG_REGISTERS, cl);
	
	if(rc == SMD_RETURN_NO_ROUTE_TO_HOST || rc == SMD_RETURN_COMMAND_FAILED) {
		return SMD_RETURN_COMMAND_FAILED;
	}
	
	else {
		
		//read_modbus_registers writes the string to the client
		return SMD_RETURN_HANDLED_BY_CLIENT;
	}
	
}

SMD_RESPONSE_CODES SMD_jog(int direction, int16_t accel, int16_t decel, int16_t jerk, int32_t speed) {
	
	//do input checking
	if(direction < 0 || direction > 1) {
		log_message("Jog: Direction invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(accel < 1 || accel > 5000) {
		log_message("Jog: Accel invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(decel < 1 || decel > 5000) {
		log_message("Jog: Decel invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(jerk < 0 || jerk > 5000) {
		log_message("Jog: Jerk invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(speed < 0 || speed > 2999999) {
		log_message("Jog: Speed invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else {
		
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
		
		int registers[10] = {1024, 1025, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1024};
		int values[10] = {0, 32768, 0, speed_UW, speed_LW, accel, decel, 0, jerk, direc};
		
		return send_modbus_command(registers, values, 10, "jog");
		
	}
}

SMD_RESPONSE_CODES SMD_relative_move(int32_t rel_pos, int16_t accel, int16_t decel, int16_t jerk, int32_t speed) {
	
	if(rel_pos < -8388607 || rel_pos > 8388607) {
		log_message("Relative Move: Position Invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(accel < 1 || accel > 5000) {
		log_message("Relative Move: Accel Invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(decel < 1 || decel > 5000) {
		log_message("Relative Move: Decel Invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(jerk < 0 || jerk > 5000) {
		log_message("Relative Move: Jerk Invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(speed < 0 || speed > 2999999) {
		log_message("Relative Move: Speed Invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else {
		
		struct Words posWords = convert_int_to_words(rel_pos);
		struct Words speedWords = convert_int_to_words(speed);
		
		//write the registers
		int registers[9] = {1025, 1026, 1027, 1028, 1029, 1030, 1031, 1033, 1024};
		int values[9] = {32768, posWords.upper_word, posWords.lower_word, speedWords.upper_word, speedWords.lower_word, accel, decel, jerk, 2};
		
		return send_modbus_command(registers, values, 9, "relative_move");
	}
}

SMD_RESPONSE_CODES SMD_drive_enable() {
	
	int registers[2] = {1025, 1024};
	int values[2] =	{32768, 0};
	
	return send_modbus_command(registers, values, 2, "drive_enable");
	
}

SMD_RESPONSE_CODES SMD_drive_disable() {
	
	int registers[2] = {1025, 1024};
	int values[2] =	{0, 0};
	
	return send_modbus_command(registers, values, 2, "drive_disable");
}

SMD_RESPONSE_CODES SMD_hold_move() {
	
	int registers[3] = {1024, 1025, 1024};
	int values[3] =	{0, 32768, 4};
	
	return send_modbus_command(registers, values, 3, "hold_move");
}

SMD_RESPONSE_CODES SMD_immed_stop() {
	
	int registers[2] = {1025, 1024};
	int values[2] =	{32768, 16};
	
	return send_modbus_command(registers, values, 2, "immediate_stop");
}

SMD_RESPONSE_CODES SMD_reset_errors() {
	
	int registers[3] = {1024, 1024, 1025};
	int values[3] = {0, 1024, 32768};
	
	return send_modbus_command(registers, values, 3, "reset_errors");
}

SMD_RESPONSE_CODES SMD_preset_encoder_position(int32_t pos) {
	
	if(pos < -8388607 || pos > 8388607) {
		log_message("Preset Encoder: Position Invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else {
		
		struct Words returnWords = convert_int_to_words(pos);
		int registers[4] = {1025, 1026, 1027, 1024};
		int values[4] = {32768, returnWords.upper_word, returnWords.lower_word, 16384};
		
		SMD_RESPONSE_CODES response = send_modbus_command(registers, values, 4, "preset_encoder");
		
		if(response == SMD_RETURN_COMMAND_SUCCESS) {
			return SMD_RETURN_PRESET_ENC_SUCCESS;
		}
		
		else {
			return SMD_RETURN_PRESET_ENC_FAIL;
		}
		
	}
	
}

SMD_RESPONSE_CODES SMD_preset_motor_position(int32_t pos) {
	
	if(pos < -8388607 || pos > 8388607) {
		log_message("Preset Motor Position: Position Invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else {
		
		struct Words returnWords = convert_int_to_words(pos);
		int registers[4] = {1025, 1026, 1027, 1024};
		int values[4] = {32768, returnWords.upper_word, returnWords.lower_word, 512};
		
		SMD_RESPONSE_CODES response = send_modbus_command(registers, values, 4, "preset_motor");
		
		if(response == SMD_RETURN_COMMAND_SUCCESS) {
			return SMD_RETURN_PRESET_POS_SUCCESS;
		}
		
		else {
			return SMD_RETURN_PRESET_POS_FAIL;
		}
	}
}

SMD_RESPONSE_CODES SMD_set_configuration(int32_t control_word, int32_t config_word, int32_t starting_speed, int16_t steps_per_turn, int16_t enc_pulses_per_turn, int16_t idle_current_percentage, int16_t motor_current) {
	
	if(starting_speed < 1 || starting_speed > 1999999) {
		log_message("Set Config: Speed invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(steps_per_turn < 200 || steps_per_turn > 32767) {
		log_message("Set Config: Steps per turn invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(enc_pulses_per_turn != 1024) {
		log_message("Set Config: Encoder pulses invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(idle_current_percentage < 0 || idle_current_percentage > 100) {
		log_message("Set Config: Idle current percentage invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(motor_current < 10 || motor_current > 34) {
		log_message("Set Config: Motor current invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else {
		
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
		
		SMD_RESPONSE_CODES response = send_modbus_command(registers, values, 8, "set_configuration");
		
		if(response == SMD_RETURN_COMMAND_SUCCESS)
			return SMD_RETURN_SAVE_CONFIG_SUCCESS;
		else
			return SMD_RETURN_SAVE_CONFIG_FAIL;
	}
}

SMD_RESPONSE_CODES SMD_find_home(int direction, int32_t speed, int16_t accel, int16_t decel, int16_t jerk) {
	
	//do input checking
	if(direction < 0 || direction > 1) {
		log_message("Find Home: Direction invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(accel < 1 || accel > 5000) {
		log_message("Find Home: Accel invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(decel < 1 || decel > 5000) {
		log_message("Find Home: Decel invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(jerk < 0 || jerk > 5000) {
		log_message("Find Home: Jerk invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else if(speed < 0 || speed > 2999999) {
		log_message("Find Home: Speed invalid\n");
		return SMD_RETURN_INVALID_PARAMETER;
	}
	
	else {
		
		int direc = direction == 0 ? 32 : 64;
		struct Words speed_words = convert_int_to_words(speed);
		
		//write the registers
		int registers[8] = {1025, 1028, 1029, 1030, 1031, 1033, 1032, 1024};
		int values[8] = {32768, speed_words.upper_word, speed_words.lower_word, accel, decel, jerk, 0 ,direc};
		
		return send_modbus_command(registers, values, 8, "find_home");
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