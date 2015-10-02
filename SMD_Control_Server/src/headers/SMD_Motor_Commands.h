/*
*  SMD_Motor_Commands.h
*  SMD_Control_Server
*
*  Created by John Davis on 9/30/15.
*  Copyright © 2015 3ML LLC. All rights reserved.
*/

/**
 * @file SMD_Motor_Commands.h
 * @author John Davis <jd@pauldavisautomation.com>
 * @date September 30, 2015
 * @brief Motor commands for controlling and configuring an AMCI SMD series drive.
*/

#ifndef SMD_Motor_Commands_h
#define SMD_Motor_Commands_h

#include <stdio.h>

/**
	@fn int SMD_open_command_connection()
	@brief Attempts to open the control connection with the SMD. This connection stays open until SMD_close_command_connection() is called. If the connection is lost, all motion stops immediately.
 */
int SMD_open_command_connection();

/**
	@fn void SMD_close_command_connection()
	@brief Attempts to close the control connection with the SMD. When the connection is lost, all motion stops immediately.
 */
void SMD_close_command_connection();

/**
	@fn int SMD_read_input_registers(int cl)
	@brief Reads the input registers, and outputs them in the format ",,0x0,0x0,0,0,0,0,0,0,0,0"
	@param cl The client to write the message to (a socket identifer)
	@return int 0 if successful
*/
int SMD_read_input_registers(int cl);

/**
	@fn int SMD_load_current_configuration(int cl)
	@brief Called prior to calling SMD_read_current_configuration(int cl). Tells the drive to read the current configuration and place it into the input registers. This will return READY_TO_READ_CONFIG to the client, which SHOULD then read the input registers once to get the configuration. This is a bit of a testy function, and care should be taken to make sure that multiple operations are not being written to the drive at the time of issuing this command. This command WILL stop motion and disable the drive.
	@param cl The client to write the message to (a socket identifer)
	@return int 0 if successful
	@note Call before calling SMD_read_current_configuration(int cl)
 */
int SMD_load_current_configuration(int cl);

/**
	@fn SMD_read_current_configuration(int cl)
	@brief Called after SMD_load_current_configuration(int cl) Reads the input registers, and outputs them in the format "###0x0,0x0,0,0,0,0,0,0,0,0"
	@param cl The client to write the message to (a socket identifer)
	@return int 0 if successful
	@warning Must call SMD_read_input_registers(int cl) before calling this function.
 */
int SMD_read_current_configuration(int cl);

/**
	@fn int SMD_set_configuration(int32_t control_word, int32_t config_word, int32_t starting_speed, int16_t steps_per_turn, int16_t enc_pulses_per_turn, int16_t idle_current_percentage, int16_t motor_current)
	@brief Loads configuration into drive memory. This does not write the configuration to permanent memory - configuration will need to be reloaded if power is lost.
	@param control_word Control word as specified by the AMCI documentation
	@param config_word Config word as specified by the AMCI documentation
	@param starting_speed Value between 1 and 1,999,999
	@param steps_per_turn Value between 200 and 32,767
	@param enc_pulses_per_turn Number of encoder pulses per turn
	@param idle_current_percentage 0 to 100
	@param motor_current Value 10-34, represents 1.0 to 3.4 Arms
	@return int 0 if successful
 
 */
int SMD_set_configuration(int32_t control_word,
						  int32_t config_word,
						  int32_t starting_speed,
						  int16_t steps_per_turn,
						  int16_t enc_pulses_per_turn,
						  int16_t idle_current_percentage,
						  int16_t motor_current);

/**
	@fn int SMD_jog(int direction, int16_t accel, int16_t decel, int16_t jerk, int32_t speed)
	@brief Tells the motor to jog. Stop the motor using either SMD_immed_stop() or SMD_hold_move()
	@param direction 0 for CW, 1 for CCW
	@param accel Acceleration, 1-5000
	@param decel Deceleration, 1-5000
	@param jerk Acceleration jerk, 1-5000
	@param speed Motor Starting Speed-2,999,999
	@return int 0 if successful
 */
int SMD_jog(int direction,
			int16_t accel,
			int16_t decel,
			int16_t jerk,
			int32_t speed);


/**
	@fn int SMD_relative_move(int32_t rel_pos, int16_t accel, int16_t decel, int16_t jerk, int32_t speed)
	@brief Tells the motor to do a relative move. Stop the motor using either SMD_immed_stop() or SMD_hold_move()
	@param rel_pos 0 How many counts to move, relative to the current position (drive does NOT need to be homed). -8,388,607 to 8,388,607
	@param accel Acceleration 1-5000
	@param decel Deceleration 1-5000
	@param jerk Acceleration jerk 1-5000
	@param speed Speed Motor Starting Speed-2,999,999
	@return int 0 if successful
 */
int SMD_relative_move(int32_t rel_pos,
					  int16_t accel,
					  int16_t decel,
					  int16_t jerk,
					  int32_t speed);

/**
	@fn int SMD_drive_enable()
	@brief Enables the drive circuitry, applying current to the motor.
	@return int 0 if successful
 */
int SMD_drive_enable();

/**
	@fn int SMD_drive_disable()
	@brief Disables the drive circuitry, removing current from the motor.
	@return int 0 if successful
 */
int SMD_drive_disable();

/**
	@fn int SMD_reset_errors()
	@brief Resets drive errors.
	@return int 0 if successful
 */
int SMD_reset_errors();

/**
	@fn int SMD_immed_stop()
	@brief Immediately stops movement, with no deceleration. This WILL set an error state in the drive, and the motor will need to be re-homed.
	@return int 0 if successful
 */
int SMD_immed_stop();

/**
	@fn int SMD_hold_move()
	@brief Stops movement with a controlled deceleration. No error state is set.
	@return int 0 if successful
 */
int SMD_hold_move();

/**
	@fn int SMD_preset_encoder_position(int32_t pos)
	@brief Presets the encoder to the value specified in pos
	@param pos Position, in counts.
	@return int 0 if successful
 */
int SMD_preset_encoder_position(int32_t pos);

/**
	@fn int int SMD_preset_motor_position(int32_t pos)
	@brief Presets the motor to the value specified in pos
	@param pos Position, in counts. -8,388,607 to 8,388,607
	@return int 0 if successful
 */
int SMD_preset_motor_position(int32_t pos);

/**
	@fn int SMD_find_home(int direction, int32_t speed, int16_t accel, int16_t decel, int16_t jerk)
	@brief Tells the drive to perform the built-in homing function. This function is outlined in the motor user manual.
	@param direction 0 for CW, 1 for CCW
	@param accel Acceleration, 1-5000
	@param decel Deceleration, 1-5000
	@param jerk Acceleration jerk, 1-5000
	@param speed Speed Motor Starting Speed-2,999,999
	@return int 0 if successful
*/
int SMD_find_home(int direction,
				  int32_t speed,
				  int16_t accel,
				  int16_t decel,
				  int16_t jerk);


/**
	@fn int program_block_first_block()
	@brief Tells the drive that it should be expecting assembled move segments as the next commands
	@return int 0 if successful
 */
int program_block_first_block();

/**
	@fn int prepare_for_next_segment()
	@brief Tells the drive that the client is done transmitting the current segment, and should prepare for the next segment from the client
	@return int 0 if successful
 */
int prepare_for_next_segment();

/**
	@fn int program_move_segment(int32_t target_pos, int32_t speed, int16_t accel, int16_t decel, int16_t jerk);
	@brief Programs the drive with the specified segment
	@param target_pos In motor counts, -8,388,607 to 8,388,607
	@param speed Motor Starting Speed-2,999,999
	@param accel Acceleration, 1-5000
	@param decel Deceleration, 1-5000
	@param jerk Jerk, 1-5000
	@return int 0 if successful
 */
int program_move_segment(int32_t target_pos,
						 int32_t speed,
						 int16_t accel,
						 int16_t decel,
						 int16_t jerk);

/**
	@fn run_assembled_move(int16_t blend_move_direction, int32_t dwell_time)
	@brief Runs the current assembled move that is in memory. 
	@param blend_move_direction 0 is CW, 1 is CCW
	@param dwell_time Specified dwell time between segments, in milliseconds
	@return int 0 if successful
 */
int run_assembled_move(int16_t blend_move_direction, int32_t dwell_time);

#endif /* SMD_Motor_Commands_h */
