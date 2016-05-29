/*
 *  SMD_Constants.h
 *  SMD_Control_Server
 *
 *  Created by John Davis on 9/30/15.
 *  Copyright © 2015 3ML LLC. All rights reserved.
 */

/** \mainpage SMDServer Commands
 *	\section Introduction
 *		The goal of this software is to support all of the commands listed in the AMCI manual for the SMD series motors. Those commands are documented in the AMCI
 *		[SMD user manual](http://www.amci.com/stepper-motor-control/integrated-stepper-motor-control-smd23e.asp).
 *	
 *		If you haven't already done so, you __really__ need to read README.md in the root directory. Dependencies, limitations, and other important info are all
 *		contained in this file.
 *		
 *		Comments, suggestions, bugs, etc. should all be directed to [jd@pauldavisautomation.com](jd@pauldavisautomation.com)
 *
 *	\section Commands
 *		All of these commands are used while connected to the server using `telnet` or some other terminal emulation program. 
 *
 *		If you get unexplained errors, you
 *		may want to run SMDServer in verbose (debug) mode: `SMDServer -v`
 *
 *	\subsection connect
 *	Command format is as so (replace IP address with the SMD IP address):
 *
 *	`connect,10.0.6.50`
 *
 *	If successful, server responds with `SMD_CONNECT_SUCCESS`
 *
 *
 *	\subsection disconnect
 *	Command format is as follows (no IP address required):
 *
 *	`disconnect`
 *
 *	No response is given by the server. The network light on the SMD should stop blinking at this time.
 *
 *
 *	\subsection driveEnable
 *	Command format is as follows:
 *
 *	`driveEnable`
 *
 *	If successful, server responds with `ENABLE_SUCCESS`
 *
 *
 *	\subsection driveDisable
 *	Command format is as follows:
 *
 *	`driveDisable`
 *
 *	If successful, server responds with `DISABLE_SUCCESS`
 *
 *
 *	\subsection  jogCW
 *	Jogs the drive clockwise. 
 *
 *	The format of the command is as follows:
 *
 *	`jogCW,accel,decel,jerk,speed`
 *
 *	The following limits apply:
 *
 *		*accel* must be a positive integer between 1-5000
 *
 *		*decel* must be a postive integer between 1-5000
 *
 *		*jerk* must be a positive integer between 0-5000
 *
 *		*speed* must be a positive integer between configured starting speed (configured in AMCI NetConfigurator) and 2,999,999
 *
 *	 If successful, server responds with `COMMAND_SUCCESS`
 *
 *	\subsection jogCCW
 *	Jogs the drive counter clockwise.
 *
 *	The format of the command is as follows:
 *
 *	`jogCCW,accel,decel,jerk,speed`
 *
 *	The following limits apply:
 *
 *		*accel* must be a positive integer between 1-5000
 *
 *		*decel* must be a postive integer between 1-5000
 *
 *		*jerk* must be a positive integer between 0-5000
 *
 *		*speed* must be a positive integer between configured starting speed (configured in AMCI NetConfigurator) and 2,999,999
 *
 *	If successful, server responds with `COMMAND_SUCCESS`
 *
 *	\subsection relativeMove
 *	Moves the motor a specified number steps, following a trapezoidal move profile. The motor does __not__ need to be homed.
 *
 *	The format of the command is as follows:
 *
 *	`relativeMove,rel_pos,accel,decel,jerk,speed`
 *
 *	The following limits apply:
 *
 *		*rel_pos* must be an integer between -8,388,607 and +8,388,607
 *
 *	    *accel* must be a positive integer between 1-5000
 *
 *	    *decel* must be a postive integer between 1-5000
 *
 *	    *jerk* must be a positive integer between 0-5000
 *
 *	    *speed* must be a positive integer between configured starting speed (configured in AMCI NetConfigurator) and 2,999,999
 *
 *	If successful, server responds with `REL_MOVE_COMPLETE`
 *
 *	\subsection holdMove
 *	Stops motion using the deceleration value specified in the active move profile. The drive will __not__ register a *Position Invalid* error.
 *
 *	The format of the command is as follows:
 *
 *	`holdMove`
 *
 *	If successful, server responds with `COMMAND_SUCCESS`
 *
 *	\subsection immedStop
 *	Stops motion using a hard stop with no deceleration. The drive __will__ register a *Position Invalid* error and will need to be re-homed.
 *
 *	The format of the command is as follows:
 *
 *	`immedStop`
 *
 *	If successful, server responds with `COMMAND_SUCCESS`
 *
 *	\subsection resetErrors
 *	Resets and errors in the drive that prevent motion. Depending on home state, some errors may persist. Consult AMCI manual for more details.
 *
 *	The format of this command is:
 *
 *	`resetErrors`
 *
 *	If successful, server responds with `RESET_ERRORS_SUCCESS`
 *
 *	\subsection readInputRegisters
 *	Command to read input registers, outputs registers back to client in format 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 0.
 *
 *	The format of the input registers follows the byte order as specified in the user manual section titled *Input Registers*. That is, the left most 0x0
 *	above is word 0.
 *
 *	The format of this command is:
 *
 *	`readInputRegisters`
 *
 *
 *	\subsection presetMotorPosition
 *	Command that presets the motor position (motor counts) to a specified value.
 *
 *	The format of the command is as follows:
 *
 *	`presetMotorPosition,pos`
 *	
 *	The following limits apply:
 *
 *		*pos* must be an integer between -8,388,607 and +8,388,607
 *
 *	If successful, server responds with `PRESET_POSITION_SUCCESS`
 *
 *	\subsection presetEncoderPosition
 *	Command that presets the encoder position to a specified value.
 *
 *	The format of the command is as follows:
 *
 *	`presetEncoderPosition,pos`
 *
 *	If successful, server responds with `PRESET_ENCODER_SUCCESS`
 *
 *	The following limits apply:
 *
 *		*pos* must be an integer between -8,388,607 and +8,388,607
 *
 *	\subsection Configuration (loadCurrentConfiguration, readCurrentConfiguration, saveConfig)
 *	Configuration is a bit of a tricky item with these drives. You really need to read the user manual and understand how the drive interprets the "configuration mode"
 *	bit in the Command Word or else you are going to run into problems with annoying errors and the like. Also, you will want to learn the difference between writing
 *	the configuration to the onboard flash versus just loading it into memory.
 *
 *	When wrapping a GUI around this process, you will have to make sure to give the drive ample time to flip bits and process the information.
 *
 *	In general though, the operation goes something like this (in order of operations from the client to the server):
 *
 *	1. Execute `loadCurrentConfiguration` to put the drive into Configuration Mode and load the input registers with the current configuration.
 *	2. Execute `readCurrentConfiguration` which will echo the current configuration to the client terminal using a similar format as the input (status) registers.
 *	3. Optionally, use saveConfig to send a configuration back to the SMD.
 *	4. To put the drive back into Command Mode, use `resetErrors` or `driveEnable` to flip the appropriate bits.
 *
 *	\subsection loadCurrentConfiguration
 *	See above for explanation.
 *
 *	The format of the command is as follows:
 *
 *	`loadCurrentConfiguration`
 *
 *	If successful, server responds with `READY_TO_READ_CONFIG`
 *
 *	\subsection readCurrentConfiguration
 *	See above (point 2) for explanation. The return format of the config input registers is as follows:
 *
 *	`###0x0,0x0,0,0,0,0,0,0,0,0`
 *
 *	Similar to the status input registers, this format is in the same order as the registers in the user manual (the leftmost word is Config Word 0)
 *
 *
 *	\subsection saveConfig
 *	Saves a configuration specified by the following command string to the drive:
 *
 *	`saveConfig,control_word,config_word,motor_starting_speed,motor_steps_per_turn,encoder_pulses_per_turn,idle_current_percentage,motor_current`
 *	
 *		*control_word* is a decimal representation of the Control Word as outlined in the SMD user manual
 *		*config_word* is a decimal representation of the Config Word as outlined in the SMD user manual
 *		*motor_starting_speed is a positive integer between 1 and 1999999
 *		*motor_steps_per_turn* is a positive integer between 200 and 32767
 *		*encoder_pulses_per_turn* is 1024
 *		*idle_current_percentage* is an integer between 1 and 100, representing 1% of total motor current to 100% of 3.4A.
 *		*motor_current* is a value between 1 and 34, representing 1.0A to 3.4A
 *
 *
 *	\subsection homeCW
 *	Executes a predefined homing routine where the motor jogs clockwise at the specified speed until seeing the Home Limit input close.
 *	
 *	The format of the command is as follows:
 *
 *	`homeCW,speed,accel,decel,jerk`
 *
 *		*speed* is an integer between 1 and 2999999
 *		*accel* is an integer between 1 and 5000
 *		*decel* is an integer between 1 and 5000
 *		*jerk* is an integer between 1 and 5000
 *
 *	\subsection homeCCW
 *	Executes a predefined homing routine where the motor jogs counter clockwise at the specified speed until seeing the Home Limit input close.
 *
 *	The format of the command is as follows:
 *
 *	`homeCCW,speed,accel,decel,jerk`
 *
 *		*speed* is an integer between 1 and 2999999
 *		*accel* is an integer between 1 and 5000
 *		*decel* is an integer between 1 and 5000
 *		*jerk* is an integer between 1 and 5000
 *
 *
 *
 *	\subsection programAssembledMove
 *	You will definitely want to home the motor prior to trying an assembled move. 
 *
 *	Once homed, you upload the move profile to the drive using the following command:
 *	
 *	`programAssembledMove,move_json`
 *
 *	*move_json* is a JSON string with the following format:
 *
 *		{
 *
 *			"segment": {
 *				"target_pos_inches": 5200,
 *				"programmed_speed": 1000,
 *				"accel": 150,
 *				"decel": 150,
 *				"jerk": 0
 *			},
 *			"segment": {
 *				"target_pos_inches": 5200,
 *				"programmed_speed": 2000,
 *				"accel": 150,
 *				"decel": 150,
 *				"jerk": 0
 *			},
 *			"segment": {
 *				"target_pos_inches": 5200,
 *				"programmed_speed": 4000,
 *				"accel": 150,
 *				"decel": 150,
 *				"jerk": 0
 *			}
 *		}
 *
 *	An assembled move profile can have up to __16__ segments. 
 *
 *	It does take some time for the move to program, so be patient. Each move segment takes ~2 seconds.
 *
 *	When the profile is successfully accepted, the server responds with `ASSEMBLED_MOVE_ACCEPTED` to the client.
 *
 *	\subsection runAssembledDwellMove
 *	Once a move profile is loaded using `programAssembledMove`, it can be executed with either `runAssembledDwellMove` or `runAssembledBlendMove`.
 *
 *	If the motor __changes direction__, you __MUST__ use a dwell move! The dwell allows the motor to decel and then change directions.
 *
 *	This format for this command is `runAssembledDwellMove,dwell_time`.
 *
 *		*dwell_time* is the time, in milliseconds (ms) that the drive should wait between move segments
 *
 *	\subsection runAssembledBlendMove
 *	Once a move profile is loaded using `programAssembledMove`, it can be executed with either `runAssembledDwellMove` or `runAssembledBlendMove`.
 *
 *	If the motor __changes direction__, you __MUST__ use a dwell move! The dwell allows the motor to decel and then change directions.
 *	There is no delay between segments in a blend move.
 *
 *	This format for this command is `runAssembledBlendMove,direction`.
 *
 *		*direction* is the motor rotation direction - 0 is CW, 1, CCW
 *
 *
 *	\subsection startManualMode
 *	Used to enable manual control mode. Beta, and undocumented. If you want to do analog control over the network, you are good enough to read the source.
 *	I originally used this as a demo using an Arduino Yun. Check out the Arduino file in this repository for an idea of what is going on. Then, read the C source.
 *
 *	\subsection stopManualMode
 *	See above. There be dragons here.
 *
 *
*/

/**
 * @file SMD_Constants.h
 * @author John Davis <jd@pauldavisautomation.com>
 * @date September 30, 2015
 * @brief Constants pertaining to network communication and program configuration.
 */

#ifndef SMD_CONSTANTS_H
#define SMD_CONSTANTS_H

#define ARRAYSIZE(arr) (sizeof(arr) / sizeof(arr[0]))	/**< Macro to help define array size */
#define INPUT_REGISTER_STRING_SIZE 17					/**< Size of the input register user string */

#include <stdio.h>
#include <modbus.h>

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
	SMD_RETURN_READY_FOR_ASSEMBLED_MOVE,			/**< SMD is ready to accept an assembled move profile */
	SMD_RETURN_ASSEMBLED_MOVE_ACCEPTED				/**< The assembled move was accepted by the drive and can now be executed */
	
} SMD_RESPONSE_CODES;


/**
 Enum defining what type of registers should be read from the SMD, and how they should be formatted to be sent to the client
 */
typedef enum SMD_REGISTER_READ_TYPE {
	
	SMD_READ_INPUT_REGISTERS,					/** Read input registers (sends client back string starting with ,, */
	SMD_READ_CONFIG_REGISTERS					/** Read input registers (sends client back string with configuration starting with ### */
	
} SMD_REGISTER_READ_TYPE;

/**
 Enum describing the types of assembled moves available
 */

typedef enum SMD_ASSEMBLED_MOVE_TYPE {
	
	SMD_ASSEMBLED_MOVE_TYPE_BLEND,				/** Assembled move type is blend - have to specify direction of movement (CW or CCW) upfront */
	SMD_ASSEMBLED_MOVE_TYPE_DWELL,				/** Assembled move type is dwell - direction can change and is specified in the individual moves */
	SMD_ASSEMBLED_MOVE_NONE						/** No assembled move type is currently loaded into drive memory */
	
} SMD_ASSEMBLED_MOVE_TYPE;

extern char						*DEVICE_IP;							/**< SMD device IP - this is defined by client upon connection */
extern const char				SMD_VERSION[8];						/**< Version string */
extern const char				SOCKET_PATH[32];					/**< The socket path - on Linux, this will be a hidden socket.
																	 Default is /tmp/smd.socket. */
extern char						*MANUAL_VALS_FILE_PATH;				/**< Manual mode file path - used when in manual mode. Format is 1,1024: first number is the
																	 direction (1=CW, 0=CCW), second number (0-1023) is the potentiometer input value Default is 
																	 /tmp/smd_manual_values */

extern int16_t					SMD_CONNECTED;						/**< Bit specifying if the SMD is currently connected */
extern int16_t					SERVER_PORT;						/**< The port on which this server should listen */
extern int8_t					VERBOSE;							/**< Enable verbose logging */
extern modbus_t					*smd_command_connection;			/**< Command connection to SMD Motor */

extern int8_t					STATUS_WAITING_FOR_ASSEMBLED_MOVE;	/**< Client has told us to wait for either a blend move or dwell move motion profile */
extern int8_t					STATUS_MANUAL_MODE_ENABLE;			/**< Manual mode has been enabled and is active */
extern SMD_ASSEMBLED_MOVE_TYPE	STATUS_TYPE_ASSEMBLED_MOVE;			/**< Type of assembled move that is loaded into drive memory */

/*
 Definitions of string constants that are sent to the client (responses to commands).
 */
extern const char COMMAND_SUCCESS[32];					/**< Sends COMMAND_SUCCESS to client */
extern const char ENABLE_SUCCESS[32];					/**< Sends ENABLE_SUCCESS to client */
extern const char DISABLE_SUCCESS[32];					/**< Sends DISABLE_SUCCESS to client */
extern const char SMD_CONNECT_SUCCESS[32];				/**< Sends SMD_CONNECT_SUCCESS to client */
extern const char COMMAND_ERROR[32];					/**< Sends COMMAND_ERROR to client */
extern const char INVALID_INPUT[32];					/**< Sends INVALID_INPUT to client */
extern const char WRITE_ERROR[32];						/**< Sends WRITE_ERROR to client */
extern const char NO_ROUTE_TO_SMD[32];					/**< Sends NO_ROUTE_TO_SMD to client */
extern const char INVALID_PARAMETER[32];				/**< Sends INVALID_PARAMETER to client */
extern const char PRESET_ENCODER_SUCCESS[32];			/**< Sends PRESET_ENCODER_SUCCESS to client */
extern const char PRESET_POSITION_SUCCESS[32];			/**< Sends PRESET_POSITION_SUCCESS to client */
extern const char PRESET_ENCODER_FAIL[32];				/**< Sends PRESET_ENCODER_FAIL to client */
extern const char PRESET_POSITION_FAIL[32];				/**< Sends PRESET_POSITION_FAIL to client */
extern const char CONFIG_SAVE_SUCCESS[32];				/**< Sends CONFIG_SAVE_SUCCESS to client */
extern const char CONFIG_SAVE_FAIL[32];					/**< Sends CONFIG_SAVE_FAIL to client */
extern const char READY_TO_READ_CONFIG[32];				/**< Sends READY_TO_READ_CONFIG to client */
extern const char GET_CURRENT_CONFIG_FAIL[32];			/**< Sends GET_CURRENT_CONFIG_FAIL to client */
extern const char RELATIVE_MOVE_COMPLETE[32];			/**< Sends RELATIVE_MOVE_COMPLETE to client */
extern const char RESET_ERRORS_SUCCESS[32];				/**< Sends RESET_ERRORS_SUCCESS to client */

extern const char SEND_ASSEMBLED_MOVE_PARAMS[32];		/**< Sends SEND_ASSEMBLED_MOVE_PROFILE to client */
extern const char ASSEMBLED_MOVE_ACCEPTED[32];			/**< Sends ASSEMBLED_MOVE_ACCEPTED to client */
extern const char MOVE_SEGMENT_ACCEPTED[32];			/**< Sends MOVE_SEGMENT_ACCEPTED_<SEGMENT NUMBER> to client */

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
extern const char SAVE_CONFIG_TO_DRIVE[32];			/**< Command to save configuration to drive 
													 (saveConfig, control_word,config_word,start_speed,motor_steps_per_turn, 
													 encoder_pulses_per_turn,idle_current_percentage,motor_current_percentage */
extern const char LOAD_CURRENT_CONFIGURATION[32];	/**< Command to load current configuration into the input registers (loadCurrentConfiguration)*/
extern const char READ_CURRENT_CONFIGURATION[32];	/**< Command to read current configuration (readCurrentConfiguration), returns string to client */
extern const char FIND_HOME_CW[32];					/**< Command to home clockwise (homeCW,accel,decel,jerk,speed) */
extern const char FIND_HOME_CCW[32];				/**< Command to home counter-clockwise (homeCCW,accel,decel,jerk,speed) */
extern const char PROGRAM_ASSEMBLED_MOVE[32];		/**< Command to tell the drive to prepare to accept JSON describing an assembled move. */
extern const char RUN_ASSEMBLED_DWELL_MOVE[32];		/**< Assembled Move - Command telling drive to run the 
													 loaded assembled move (runAssembledDwellMove,dwell_time) */
extern const char RUN_ASSEMBLED_BLEND_MOVE[32];		/**< Assembled Move - Command telling drive to run the loaded blend move
													 (runAssembledBlendMove,direction) where direction is 0=CW, 1=CCW */

extern const char START_MANUAL_MODE[32];			/**< BETA - Command that puts the drive into manual mode (for reading input values from an analog sensor and direction switch). */
extern const char STOP_MANUAL_MODE[32];				/**< BETA - Command that takes drive out of manual mode (for reading input values from an analog sensor and direction switch). */

#endif /* SMD_CONSTANTS_H */

