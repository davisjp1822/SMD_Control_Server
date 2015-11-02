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
#include "cJSON.h"

typedef struct assembled_move_segment {
	
	int32_t target_pos_inches;
	int32_t programmed_speed;
	int16_t accel;
	int16_t decel;
	int16_t jerk;
	
} assembled_move_segment;

static SMD_RESPONSE_CODES _SMD_program_block_first_block(int cl);
static SMD_RESPONSE_CODES _SMD_program_move_segment(int32_t target_pos, int32_t speed, int16_t accel, int16_t decel, int16_t jerk);
static SMD_RESPONSE_CODES _SMD_prepare_for_next_segment();

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
		STATUS_WAITING_FOR_ASSEMBLED_MOVE = 0;
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

SMD_RESPONSE_CODES SMD_program_assembled_move(int cl) {
	
	if(_SMD_program_block_first_block(cl) == SMD_RETURN_COMMAND_SUCCESS) {
		
		return SMD_RETURN_READY_FOR_ASSEMBLED_MOVE;
	}
	
	else {
		
		SMD_reset_errors();
		return SMD_RETURN_COMMAND_FAILED;
	}
}

SMD_RESPONSE_CODES SMD_parse_and_upload_assembled_move(const char *json_string) {
	
	/*
	 *	Steps for success!
	 *	1. Parse the JSON - if it doesn't pass, fail.
	 *	2. If it passes, check number of segments. If the number is greater than 16, fail.
	 *	3. Figure out what kind of move we are - blend or dwell. Set global flags accordingly.
	 *	4. Send each move with _program_move_segment
	 *	4a. Check input registers to see if move was successful
	 *	4b.	If successful, prepare for next move with _SMD_prepare_for_next_segment()
	 *	5. Return successfully when all is done.
	 *
	 *	sleep() statements are in here to give the drive/network time to catch-up.
	 *
	 */
	
	cJSON *root = cJSON_Parse(json_string);
	int i = 0;
	int num_segments = 0;
	assembled_move_segment segments[16];
	
	//1. valid JSON?
	if(root == 0) {
		
		log_message("!!! Invalid JSON describing assembled move\n");
		return SMD_RETURN_COMMAND_FAILED;
		
	}
	
	//2. Okay, it's valid, but how many segments?
	for (i=0; i<cJSON_GetArraySize(root); i++) {
		
		cJSON *subitem=cJSON_GetArrayItem(root,i);
		
		if(strncmp(subitem->string, "segment", strlen("segment")) == 0) {
			
			assembled_move_segment new_segment;
			new_segment.target_pos_inches = cJSON_GetObjectItem(subitem, "target_pos_inches")->valueint;
			new_segment.programmed_speed = cJSON_GetObjectItem(subitem, "programmed_speed")->valueint;
			new_segment.accel = cJSON_GetObjectItem(subitem, "accel")->valueint;
			new_segment.decel = cJSON_GetObjectItem(subitem, "decel")->valueint;
			new_segment.jerk = cJSON_GetObjectItem(subitem, "jerk")->valueint;
			
			segments[i] = new_segment;
			num_segments++;
		}
	}
	
	//no segments or too many segments? Bad!
	if(num_segments == 0 || num_segments > 16) {
		
		log_message("!!! Too few, or too many moves for assembled move\n");
		return SMD_RETURN_COMMAND_FAILED;
	}
	
	//3. TODO - set flags
	
	//4. loop through moves and send each one
	for(i=0; i<ARRAYSIZE(segments); i++) {
		
		assembled_move_segment s = segments[i];
		char input_registers[128];
		
		memset(input_registers, 0, sizeof(input_registers));
		memset(input_registers, 0, sizeof(input_registers));
		
		fprintf(stderr, "Programming Move Segment #%d with position %d\n", i+1, s.target_pos_inches);
		_SMD_program_move_segment(s.target_pos_inches, s.programmed_speed, s.accel, s.decel, s.jerk);
		sleep(1);
		
		//4a. check input registers to see if segment was accepted
		
		uint16_t tab_status_words_reg[10];
		memset(&tab_status_words_reg, 0, sizeof(tab_status_words_reg));
		
		int rc = return_modbus_registers(tab_status_words_reg, SMD_READ_INPUT_REGISTERS, -1, input_registers, sizeof(input_registers));
		sleep(1);
		
		if(rc == SMD_RETURN_NO_ROUTE_TO_HOST || rc == SMD_RETURN_COMMAND_FAILED) {
			
			log_message("!!! Could not read input registers when programming assembled move\n");
			return SMD_RETURN_COMMAND_FAILED;
		}
		
		//move was accepted if 0, tell drive to get ready for next segment...
		if(input_registers[6] == '0') {
			
			//4b. Prepare for the next segment!
			if(_SMD_prepare_for_next_segment() == SMD_RETURN_COMMAND_SUCCESS) {
				
				sleep(1);
				
				int rc = return_modbus_registers(tab_status_words_reg, SMD_READ_INPUT_REGISTERS, -1, input_registers, sizeof(input_registers));
				
				if(rc == SMD_RETURN_NO_ROUTE_TO_HOST || rc == SMD_RETURN_COMMAND_FAILED) {
					
					log_message("!!! Could not read input registers when programming assembled move\n");
					return SMD_RETURN_COMMAND_FAILED;
				}
				
				//check the input registers again to make sure bit 9 is set (bit 9 is 6 for our purposes)
				// if it is set - transmit next move!
				if(input_registers[6] == '1') {
					
					if( i !=ARRAYSIZE(segments)) {
						log_message("Segment Accepted! Ready for the next one...\n");
					}
				}
				
			}
			
			//prepare_for_next_segment could not connect/was not successful
			else {
				
				log_message("!!! Drive did not accept prepare_for_next_segment command\n");
				return SMD_RETURN_COMMAND_FAILED;
			}
		}
		
		//something went wrong and input registers were not set - bail
		else {
			
			log_message("!!! Assembled Move: First move segment sent, but input registers MSW 9 not reset\n");
			return SMD_RETURN_COMMAND_FAILED;
			
		}
		
	}
	
	//return if nothing goes wrong
	return SMD_RETURN_COMMAND_SUCCESS;
}

//TODO change to SMD_run_...
SMD_RESPONSE_CODES run_assembled_move(int16_t blend_move_direction, int32_t dwell_time, SMD_ASSEMBLED_MOVE_TYPE move_type) {
	
	int32_t LSW = 0;
	
	if(move_type == SMD_ASSEMBLED_MOVE_TYPE_BLEND) {
		
		//0 == CW
		LSW = blend_move_direction == 0 ? 32768 : 32784;
	}
	
	if(move_type == SMD_ASSEMBLED_MOVE_TYPE_DWELL) {
		
		if(dwell_time >= 0)
			LSW = 33280;
		
		else {
			return SMD_RETURN_INVALID_PARAMETER;
		}
	}
	
	int registers[3] = {1025, 1033, 1024};
	int values[3] =	{LSW, dwell_time, 8192};
	
	return send_modbus_command(registers, values, 3, "run_assembled_move");
	
}


/***** ASSEMBLED MOVE FUNCTIONS *****/

static SMD_RESPONSE_CODES _SMD_program_block_first_block(int cl) {
	
	int rc;
	
	int registers[2] = {1025, 1024};
	int values[2] =	{32768, 2048};
	
	//1. send the modbus command
	rc = send_modbus_command(registers, values, 2, "program_first_block");
	
	//2. Check the input registers to see if we were successful. If we were, return COMMAND_SUCCESS
	if(rc == SMD_RETURN_COMMAND_SUCCESS) {
		
		sleep(1);
		
		char input_registers[128];
		memset(input_registers, 0, sizeof(input_registers));
		
		uint16_t tab_status_words_reg[10];
		memset(&tab_status_words_reg, 0, sizeof(tab_status_words_reg));
		
		int rc = return_modbus_registers(tab_status_words_reg, SMD_READ_INPUT_REGISTERS, -1, input_registers, sizeof(input_registers));
		
		if(rc == SMD_RETURN_NO_ROUTE_TO_HOST || rc == SMD_RETURN_COMMAND_FAILED) {
			return SMD_RETURN_COMMAND_FAILED;
		}
		
		else {
			
			//check bits to make sure that the drive actually is ready to accept a move segment
			//per AMCI documentation, bits 6 & 7 should be set to 1 if the drive is good to accept move segments
			char binString[16] = {0};
			int num_tokens = number_of_tokens(input_registers);
			char *input_register_tokens[num_tokens];
			
			if(tokenize_client_input(input_register_tokens, input_registers, num_tokens, ARRAYSIZE(input_register_tokens)) != 0) {
				return SMD_RETURN_COMMAND_FAILED;
			}
			
			if(hex_string_to_bin_string(binString, sizeof(binString), input_register_tokens[2]) == 0) {
				
				if(binString[6] == '1' && binString[7] == '1') {
					return SMD_RETURN_COMMAND_SUCCESS;
				}
			}
			
			else {
				return SMD_RETURN_COMMAND_FAILED;
			}
		}
	}
	
	return SMD_RETURN_COMMAND_FAILED;
}

static SMD_RESPONSE_CODES _SMD_program_move_segment(int32_t target_pos, int32_t speed, int16_t accel, int16_t decel, int16_t jerk) {
	
	struct Words posWords = convert_int_to_words(target_pos);
	struct Words speedWords = convert_int_to_words(speed);
	
	int registers[9] = {1025, 1026, 1027, 1028, 1029, 1030, 1031, 1033, 1024};
	int values[9] = {32768, posWords.upper_word, posWords.lower_word, speedWords.upper_word, speedWords.lower_word, accel, decel, jerk, 6144};
		
	return send_modbus_command(registers, values, 9, "program_move_segment");
	
}

static SMD_RESPONSE_CODES _SMD_prepare_for_next_segment() {
		
	int registers[2] = {1025, 1024};
	int values[2] =	{32768, 2048};

	return send_modbus_command(registers, values, 2, "prepare_for_next_segment");
}

