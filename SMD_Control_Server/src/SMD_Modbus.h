/*
  SMD_Modbus.h
  SMD_Control_Server

  Created by John Davis on 10/11/15.
  Copyright © 2015 3ML LLC. All rights reserved.
*/

/**
 * @file SMD_Modbus.h
 * @author John Davis <jd@pauldavisautomation.com>
 * @date September 30, 2015
 * @brief SMD wrapper functions for SMD Modbus/TCP communications
 */

#ifndef SMD_Modbus_h
#define SMD_Modbus_h

#include "SMD_Constants.h"
#include <stdio.h>

/**
	@fn SMD_RESPONSE_CODES send_modbus_command(const int *registers, const int *values, const int num_registers, const char *command_name)
	@brief Sends a Modbus/TCP command to the SMD drive. Handles all of the overhead, such as threading, etc.
	@param registers Array (int) that contains the registers to be written to
	@param values Array (int) of the values that are meant to go in int *registers
	@param num_registers int describing how many registers are going to be sent (the length of registers)
	@param command_name String specifying the English name of the command. This is used for logging purposes (you can put anything here)
	@return SMD_RESPONSE_CODES
 */
SMD_RESPONSE_CODES send_modbus_command(const int *registers, const int *values, const int num_registers, const char *command_name);


/**
	@fn read_modbus_registers(char registers[][INPUT_REGISTER_STRING_SIZE], const int registers_count, const SMD_REGISTER_READ_TYPE reg_read_type, const int cl)
	@brief Reads input registers from the drive so that the drive status and configuration can be formatted to send to the client (reads the first 10 registers (0-10)
	@param registers 2D array that must be size 10 that will contain string values of the individual input registers
	@param registers_count Size of registers
	@param reg_read_type Specify how the client return string will be formatted (input registers (,,) or configuration (###)
	@param cl Socket handle for client that is to be written the return string
	@return SMD_RESPONSE_CODES
 */
SMD_RESPONSE_CODES read_modbus_registers(char registers[][INPUT_REGISTER_STRING_SIZE], const int registers_count, const SMD_REGISTER_READ_TYPE reg_read_type, const int cl);

/**
	@fn SMD_RESPONSE_CODES return_modbus_registers(char registers[][INPUT_REGISTER_STRING_SIZE], const int registers_count, const SMD_REGISTER_READ_TYPE reg_read_type,
 const int cl, char *registers_string, int registers_string_buf_size)
	@brief Reads input registers from the drive so that the drive status and configuration can be formatted to send to the client (reads the first 10 registers (0-10). Also returns the input registers string to registers_string
	@param registers 2D array that must be size 10 that will contain string values of the individual input registers
	@param registers_count Size of registers
	@param reg_read_type Specify how the client return string will be formatted (input registers (,,) or configuration (###)
	@param cl Socket handle for client that is to be written the return string
	@param registers_string String showing retrieved input registers values
	@param registers_string_buf_size Size of registers_string buffer. MUST BE AT LEAST 128.
	@return SMD_RESPONSE_CODES
 */
SMD_RESPONSE_CODES return_modbus_registers(char registers[][INPUT_REGISTER_STRING_SIZE], const int registers_count, const SMD_REGISTER_READ_TYPE reg_read_type,
										   const int cl, char *registers_string, int registers_string_buf_size);

#endif /* SMD_Modbus_h */
