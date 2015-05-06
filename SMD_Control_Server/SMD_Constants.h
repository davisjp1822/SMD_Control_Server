//
//  SMD_Constants.h
//  AMCI_SMD
//
//  Created by John Davis on 4/20/15.
//
//

#include <stdio.h>

//magic Linux hidden sockets
//static const char *SOCKET_PATH = "\0/tmp/smd.socket";
static const char SOCKET_PATH[16] = "/tmp/smd.socket";

//return codes used when parsing response from the SMD
typedef enum SMD_RESPONSE_CODES {
 
	SMD_RETURN_COMMAND_SUCCESS,
	SMD_RETURN_COMMAND_FAILED,
	SMD_RETURN_CONNECT_SUCCESS,
	SMD_RETURN_DISCONNECT_SUCCESS,
	SMD_RETURN_HANDLED_BY_CLIENT,
	SMD_RETURN_ENABLE_SUCCESS,
	SMD_RETURN_DISABLE_SUCCESS,
	SMD_RETURN_INVALID_INPUT,
	SMD_RETURN_NO_ROUTE_TO_HOST,
	SMD_RETURN_INVALID_PARAMETER,
	SMD_RETURN_COMMAND_MODE_SUCCESS,
	SMD_RETURN_CONFIGURATION_MODE_SUCCESS,
	SMD_RETURN_PRESET_POS_SUCCESS,
	SMD_RETURN_PRESET_ENC_SUCCESS
	
	
}SMD_RESPONSE_CODES;

//client output constants
static const char COMMAND_SUCCESS[24] = "COMMAND_SUCCESS\n";
static const char ENABLE_SUCCESS[24] = "ENABLE_SUCCESS\n";
static const char DISABLE_SUCCESS[24] = "DISABLE_SUCCESS\n";
static const char SMD_CONNECT_SUCCESS[24] = "SMD_CONNECT_SUCCESS\n";
static const char COMMAND_ERROR[24] = "COMMAND_ERROR\n";
static const char INVALID_INPUT[24] = "INVALID_INPUT\n";
static const char WRITE_ERROR[24] = "WRITE_ERROR\n";
static const char NO_ROUTE_TO_SMD[24] = "NO_ROUTE_TO_SMD\n";
static const char INVALID_PARAMETER[24] = "INVALID_PARAMETER\n";
static const char PRESET_ENCODER_SUCCESS[32] = "PRESET_ENCODER_SUCCESS\n";
static const char PRESET_POSITION_SUCCESS[32] = "PRESET_POSITION_SUCCESS\n";
static const char CONFIG_MODE_SUCCESS[32] = "CONFIG_MODE_SUCCESS\n";
static const char COMMAND_MODE_SUCCESS[32] = "COMMAND_MODE_SUCCESS\n";

//command constants (used by client and server)
static const char CONNECT[24] = "connect\n";
static const char DISCONNECT[24] = "disconnect\n";
static const char DRIVE_ENABLE[24] = "driveEnable\n";
static const char DRIVE_DISABLE[24] = "driveDisable\n";
static const char JOG_CW[24] = "jogCW";
static const char JOG_CCW[24] = "jogCCW";
static const char HOLD_MOVE[24] = "holdMove\n";
static const char IMMED_STOP[24] = "immedStop\n";
static const char RESET_ERRORS[24] = "resetErrors\n";
static const char READ_INPUT_REGISTERS[32] = "readInputRegisters\n";
static const char ENABLE_CONFIGURATION_MODE[32] = "enableConfigurationMode\n";
static const char ENABLE_COMMAND_MODE[32] = "enableCommandMode\n";
static const char PRESET_MOTOR_POSITION[32] = "presetMotorPosition\n";
static const char PRESET_ENCODER_POSITION[32] = "presetEncoderPosition\n";
