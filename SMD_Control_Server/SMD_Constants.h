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
	SMD_RETURN_INVALID_PARAMETER
	
	
}SMD_RESPONSE_CODES;

//client output constants
static const char COMMAND_SUCCESS[25] = "COMMAND_SUCCESS\n";
static const char ENABLE_SUCCESS[25] = "ENABLE_SUCCESS\n";
static const char DISABLE_SUCCESS[25] = "DISABLE_SUCCESS\n";
static const char SMD_CONNECT_SUCCESS[25] = "SMD_CONNECT_SUCCESS\n";
static const char COMMAND_ERROR[25] = "COMMAND_ERROR\n";
static const char INVALID_INPUT[25] = "INVALID_INPUT\n";
static const char WRITE_ERROR[25] = "WRITE_ERROR\n";
static const char NO_ROUTE_TO_SMD[25] = "NO_ROUTE_TO_SMD\n";
static const char INVALID_PARAMETER[25] = "INVALID_PARAMETER\n";

//command constants (used by client and server)
static const char CONNECT[16] = "connect";
static const char DISCONNECT[16] = "disconnect";
static const char DRIVE_ENABLE[16] = "driveEnable";
static const char DRIVE_DISABLE[16] = "driveDisable";
static const char JOG_CW[16] = "jogCW";
static const char JOG_CCW[16] = "jogCCW";
static const char HOLD_MOVE[16] = "holdMove\n";
static const char IMMED_STOP[16] = "immedStop\n";
static const char RESET_ERRORS[16] = "resetErrors\n";
static const char READ_INPUT_REGISTERS[32] = "readInputRegisters\n";