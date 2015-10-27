/*
 *  SMD_Constants.h
 *  SMD_Control_Server
 *
 *  Created by John Davis on 9/30/15.
 *  Copyright © 2015 3ML LLC. All rights reserved.
 */

/**
 * @file SMD_Constants.h
 * @author John Davis <jd@pauldavisautomation.com>
 * @date September 30, 2015
 * @brief Constants pertaining to network communication and program configuration.
 */

#ifndef SMD_CONSTANTS_H
#define SMD_CONSTANTS_H

#include <stdio.h>
#include <modbus.h>

extern char *DEVICE_IP;								/**< SMD device IP - this is defined by client upon connection */
extern const char VERSION[8];						/**< Version string */
extern const char SOCKET_PATH[16];					/**< The socket path - on Linux, this will be a hidden socket. Otherwise, it will be in /tmp/smd.socket. */

extern int16_t SMD_CONNECTED;						/**< Bit specifying if the SMD is currently connected */
extern int16_t SERVER_PORT;							/**< The port on which this server should listen */
extern int8_t VERBOSE;								/**< Enable verbose logging */
extern modbus_t *smd_command_connection;			/**< Command connection to SMD Motor */

/**
 Enum defining internal response codes for how the motor responds to direct modbus commands. These are not variables that are sent externally to the client.
 */
typedef enum SMD_RESPONSE_CODES {
 
	SMD_RETURN_COMMAND_SUCCESS,						/**< Successful command (Modbus returned success) */
	SMD_RETURN_COMMAND_FAILED,						/**< Failed command Modbus returned failure */
	SMD_RETURN_CONNECT_SUCCESS,						/**< Success specific to connecting the command connection to the drive */
	SMD_RETURN_DISCONNECT_SUCCESS,					/**< Success specific to disconnecting command connection from the drive */
	SMD_RETURN_HANDLED_BY_CLIENT,					/**< Return code indicating that data is being sent back to the client (input register data) */
	SMD_RETURN_ENABLE_SUCCESS,						/**< Drive enable success */
	SMD_RETURN_DISABLE_SUCCESS,						/**< Drive disable success */
	SMD_RETURN_INVALID_INPUT,						/**< The client provided an invalid command */
	SMD_RETURN_NO_ROUTE_TO_HOST,					/**< The drive is not accessible via the network */
	SMD_RETURN_INVALID_PARAMETER,					/**< An invalid parameter was provided with a valid function */
	SMD_RETURN_PRESET_POS_SUCCESS,					/**< Preset motor position command was successful */
	SMD_RETURN_PRESET_ENC_SUCCESS,					/**< Preset encoder position command was successful */
	SMD_RETURN_PRESET_POS_FAIL,						/**< Preset motor position command failed */
	SMD_RETURN_PRESET_ENC_FAIL,						/**< Preset encoder position command failed */
	SMD_RETURN_SAVE_CONFIG_SUCCESS,					/**< Write config to RAM successful */
	SMD_RETURN_SAVE_CONFIG_FAIL,					/**< Could not write config to RAM */
	SMD_RETURN_READ_CURRENT_CONFIG_FAIL,			/**< Current configuration could not be loaded into the input registers */
	SMD_RETURN_READY_TO_READ_CONFIG,				/**< Drive ready to put current configuration into input registers */
	SMD_RETURN_RESET_ERRORS_SUCCESS,				/**< Error reset command successful */
	SMD_RETURN_UNKNOWN_ERROR,						/**< Unknown error - shouldn't really be seen under normal circumstances */
	
	SMD_RETURN_READY_FOR_SEGMENTS,					/**< Assembled Move - Drive ready and waiting for segments */
	SMD_RETURN_SEND_NEXT_SEGMENT,					/**< Assembled Move - Drive ready to accept next segment */
	SMD_RETURN_SEGMENT_ACCEPTED						/**< Assembled Move - Segment accepted successfully */
	
	
} SMD_RESPONSE_CODES;


/**
 Enum defining what type of registers should be read from the SMD, and how they should be formatted to be sent to the client
 */
typedef enum SMD_REGISTER_READ_TYPE {
	
	SMD_READ_INPUT_REGISTERS,					/** Read input registers (sends client back string starting with ,, */
	SMD_READ_CONFIG_REGISTERS					/** Read input registers (sends client back string with configuration starting with ### */
	
} SMD_REGISTER_READ_TYPE;

typedef enum SMD_ASSEMBLED_MOVE_TYPE {
	
	SMD_ASSEMBLED_MOVE_TYPE_BLEND,				/** Assembled move type is blend - have to specify direction of movement (CW or CCW) upfront */
	SMD_ASSEMBLED_MOVE_TYPE_DWELL				/** Assembled move type is dwell - direction can change and is specified in the individual moves */
	
} SMD_ASSEMBLED_MOVE_TYPE;

/*
 Definitions of string constants that are sent to the client (responses to commands).
 */
extern const char COMMAND_SUCCESS[32];				/**< Sends COMMAND_SUCCESS to client */
extern const char ENABLE_SUCCESS[32];				/**< Sends ENABLE_SUCCESS to client */
extern const char DISABLE_SUCCESS[32];				/**< Sends DISABLE_SUCCESS to client */
extern const char SMD_CONNECT_SUCCESS[32];			/**< Sends SMD_CONNECT_SUCCESS to client */
extern const char COMMAND_ERROR[32];				/**< Sends COMMAND_ERROR to client */
extern const char INVALID_INPUT[32];				/**< Sends INVALID_INPUT to client */
extern const char WRITE_ERROR[32];					/**< Sends WRITE_ERROR to client */
extern const char NO_ROUTE_TO_SMD[32];				/**< Sends NO_ROUTE_TO_SMD to client */
extern const char INVALID_PARAMETER[32];			/**< Sends INVALID_PARAMETER to client */
extern const char PRESET_ENCODER_SUCCESS[32];		/**< Sends PRESET_ENCODER_SUCCESS to client */
extern const char PRESET_POSITION_SUCCESS[32];		/**< Sends PRESET_POSITION_SUCCESS to client */
extern const char PRESET_ENCODER_FAIL[32];			/**< Sends PRESET_ENCODER_FAIL to client */
extern const char PRESET_POSITION_FAIL[32];			/**< Sends PRESET_POSITION_FAIL to client */
extern const char CONFIG_SAVE_SUCCESS[32];			/**< Sends CONFIG_SAVE_SUCCESS to client */
extern const char CONFIG_SAVE_FAIL[32];				/**< Sends CONFIG_SAVE_FAIL to client */
extern const char READY_TO_READ_CONFIG[32];			/**< Sends READY_TO_READ_CONFIG to client */
extern const char GET_CURRENT_CONFIG_FAIL[32];		/**< Sends GET_CURRENT_CONFIG_FAIL to client */
extern const char RELATIVE_MOVE_COMPLETE[32];		/**< Sends RELATIVE_MOVE_COMPLETE to client */
extern const char RESET_ERRORS_SUCCESS[32];			/**< Sends RESET_ERRORS_SUCCESS to client */

extern const char READY_FOR_SEGMENTS[32];			/**< Sends READY_FOR_SEGMENTS to client */
extern const char SEND_NEXT_SEGMENT[32];			/**< Sends SEND_NEXT_SEGMENT to client */
extern const char SEGMENT_ACCEPTED[32];				/**< Sends SEGMENT_ACCEPTED to client */

/*
 Definitions of string constants that define motor commands to the server from the client
 */
 
extern const char CONNECT[32];						/**< Command (connect,192.168.1.50) to connect to drive */
extern const char DISCONNECT[32];					/**< Command to disconnect (disconnect) from drive */
extern const char DRIVE_ENABLE[32];					/**< Command to enable drive (driveEnable) */
extern const char DRIVE_DISABLE[32];				/**< Command to disable drive (driveDisable) */
extern const char JOG_CW[32];						/**< Command to jog clockwise (jogCW,accel,decel,jerk,speed) */
extern const char JOG_CCW[32];						/**< Command to jog counter-clockwise (jogCCW,accel,decel,jerk,speed) */
extern const char RELATIVE_MOVE[32];				/**< Command to execute a relative move (relativeMove,rel_pos,accel,decel,jerk,speed)*/
extern const char HOLD_MOVE[32];					/**< Command to stop motion with controlled deceleration (holdMove) */
extern const char IMMED_STOP[32];					/**< Command to stop move with an immediate hard stop (immedStop) */
extern const char RESET_ERRORS[32];					/**< Command to reset errors */
extern const char READ_INPUT_REGISTERS[32];			/**< Command to read input registers, outputs registers back to client in format 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 0 */
extern const char PRESET_MOTOR_POSITION[32];		/**< Command to preset motor position (presetMotorPosition,pos) */
extern const char PRESET_ENCODER_POSITION[32];		/**< Command to preset encoder position (presetEncoderPosition,pos) */
extern const char SAVE_CONFIG_TO_DRIVE[32];			/**< Command to save configuration to drive (saveConfig, control_word,config_word,start_speed,motor_steps_per_turn, encoder_pulses_per_turn,idle_current_percentage,motor_current_percentage */
extern const char LOAD_CURRENT_CONFIGURATION[32];	/**< Command to load current configuration into the input registers (loadCurrentConfiguration)*/
extern const char READ_CURRENT_CONFIGURATION[32];	/**< Command to read current configuration (readCurrentConfiguration), returns string to client */
extern const char FIND_HOME_CW[32];					/**< Command to home clockwise (homeCW,accel,decel,jerk,speed) */
extern const char FIND_HOME_CCW[32];				/**< Command to home counter-clockwise (homeCCW,accel,decel,jerk,speed) */

extern const char PROGRAM_FIRST_BLOCK[32];			/**< Assembled Move - Command telling drive to wait for assembled move sequence (programFirstBlock) */
extern const char PREPARE_FOR_NEXT_SEGMENT[32];		/**< Assembled Move - Command telling drive to prepare for next segment (prepareForNextSegment) */
extern const char PROGRAM_MOVE_SEGMENT[32];			/**< Assembled Move - Command telling drive to program segment (programMoveSegment,targetPos,speed,accel,decel,jerk */
extern const char RUN_ASSEMBLED_DWELL_MOVE[32];		/**< Assembled Move - Command telling drive to run the loaded assembled move (runAssembledDwellMove,direc,dwell_time) */

#endif /* SMD_CONSTANTS_H */