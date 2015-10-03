//
//  SMD_Constants.c
//  SMD_Control_Server
//
//  Created by John Davis on 10/2/15.
//  Copyright © 2015 3ML LLC. All rights reserved.
//

#include "SMD_Constants.h"

#include <stdio.h>

char DEVICE_IP[32];
const char VERSION[8] = "2.0";
modbus_t *smd_command_connection = NULL;
int16_t SERVER_PORT = 7000;

#ifdef __linux
	const char SOCKET_PATH[16] = "\0/tmp/smd.socket";
#else
	const char SOCKET_PATH[16] = "/tmp/smd.socket";
#endif

//client output constants
const char COMMAND_SUCCESS[32] = "COMMAND_SUCCESS\n";
const char ENABLE_SUCCESS[32] = "ENABLE_SUCCESS\n";
const char DISABLE_SUCCESS[32] = "DISABLE_SUCCESS\n";
const char SMD_CONNECT_SUCCESS[32] = "SMD_CONNECT_SUCCESS\n";
const char COMMAND_ERROR[32] = "COMMAND_ERROR\n";
const char INVALID_INPUT[32] = "INVALID_INPUT\n";
const char WRITE_ERROR[32] = "WRITE_ERROR\n";
const char NO_ROUTE_TO_SMD[32] = "NO_ROUTE_TO_SMD\n";
const char INVALID_PARAMETER[32] = "INVALID_PARAMETER\n";
const char PRESET_ENCODER_SUCCESS[32] = "PRESET_ENCODER_SUCCESS\n";
const char PRESET_POSITION_SUCCESS[32] = "PRESET_POSITION_SUCCESS\n";
const char PRESET_ENCODER_FAIL[32] = "PRESET_ENCODER_FAIL\n";
const char PRESET_POSITION_FAIL[32] = "PRESET_POSITION_FAIL\n";
const char CONFIG_SAVE_SUCCESS[32] = "CONFIG_SAVE_SUCCESS\n";
const char CONFIG_SAVE_FAIL[32] = "CONFIG_SAVE_FAIL\n";
const char READY_TO_READ_CONFIG[32] = "READY_TO_READ_CONFIG\n";
const char GET_CURRENT_CONFIG_FAIL[32] = "GET_CURRENT_CONFIG_FAIL\n";
const char RELATIVE_MOVE_COMPLETE[32] = "REL_MOVE_COMPLETE\n";
const char RESET_ERRORS_SUCCESS[32] = "RESET_ERRORS_SUCCESS\n";
const char READY_FOR_SEGMENTS[32] = "READY_FOR_SEGMENTS\n";
const char SEND_NEXT_SEGMENT[32] = "SEND_NEXT_SEGMENT\n";
const char SEGMENT_ACCEPTED[32] = "SEGMENT_ACCEPTED\n";

//command constants (used by client and server)
const char CONNECT[32] = "connect";
const char DISCONNECT[32] = "disconnect";
const char DRIVE_ENABLE[32] = "driveEnable";
const char DRIVE_DISABLE[32] = "driveDisable";
const char JOG_CW[32] = "jogCW";
const char JOG_CCW[32] = "jogCCW";
const char RELATIVE_MOVE[32] = "relativeMove";
const char HOLD_MOVE[32] = "holdMove";
const char IMMED_STOP[32] = "immedStop";
const char RESET_ERRORS[32] = "resetErrors";
const char READ_INPUT_REGISTERS[32] = "readInputRegisters";
const char PRESET_MOTOR_POSITION[32] = "presetMotorPosition";
const char PRESET_ENCODER_POSITION[32] = "presetEncoderPosition";
const char SAVE_CONFIG_TO_DRIVE[32] = "saveConfig";
const char LOAD_CURRENT_CONFIGURATION[32] = "loadCurrentConfiguration";
const char READ_CURRENT_CONFIGURATION[32] = "readCurrentConfiguration";
const char FIND_HOME_CW[32] = "homeCW";
const char FIND_HOME_CCW[32] = "homeCCW";
const char PROGRAM_FIRST_BLOCK[32] = "programFirstBlock";
const char PREPARE_FOR_NEXT_SEGMENT[32] = "prepareForNextSegment";
const char PROGRAM_MOVE_SEGMENT[32] = "programMoveSegment";
const char RUN_ASSEMBLED_DWELL_MOVE[32] = "runAssembledDwellMove";