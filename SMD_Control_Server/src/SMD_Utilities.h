/*
*	SMD_Utilities.h
*	SMD_Control_Server
*
*	Created by John Davis on 4/28/15.
*	Copyright (c) 2015 3ML LLC. All rights reserved.
*/

/**
 * @file SMD_Utilities.h
 * @author John Davis <jd@pauldavisautomation.com>
 * @date September 30, 2015
 * @brief Helper functions and utilities.
 */

#ifndef SMD_UTILITIES_H
#define SMD_UTILITIES_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

/**
	Struct defining the upper and lower words for variables that are split into 32 bit components
 */
struct Words {
	int16_t lower_word; /**< 16 bit integer representing lower word */
	int16_t upper_word; /**< 16 bit integer representing upper word */
};

/**
	@fn struct Words convert_int_to_words(int32_t number)
	@brief Splits a 32 bit integer into 16 bit words. Useful when taking a speed command from a client and converting it into something the AMCI drive understands.
	@return Words
 */
struct Words convert_int_to_words(int32_t number);

/**
	@fn int32_t convert_string_to_long_int(const char *str)
	@brief Converts a string into a long int - useful for turning client commands into integers for accel, decel, speed, etc
	@param str string to convert into 32 bit integer
	@return int32_t
 */
int32_t convert_string_to_long_int(const char *str);

/**
	@fn int number_of_tokens(const char *command_string)
	@brief Counts the number of parameters (separate from the command itself) sent from the client to the server
	@param command_string Raw command string sent from client
	@return int Number of tokens
 */
int number_of_tokens(const char *command_string);

/**
	@fn int tokenize_client_input(char **array_of_commands, const char *input, int num_tokens, const size_t array_of_commands_size)
	@brief Tokenizes the client input (separated by ,)
	@param array_of_commands destination array for tokens
	@param input Input string from the client
	@param num_tokens Number of tokens in array_of_commands
	@param array_of_commands_size Size of array_of_commands
	@return Returns 0 if successful, -1 if error
 */
int tokenize_client_input(char **array_of_commands, const char *input, int num_tokens, const size_t array_of_commands_size);


/**
	@fn int hex_string_to_bin_string(char *return_string, size_t buf_size, const char *input)
	@brief Converts a hex value into a binary string.
	@param return_string String pointer that will hold the binary string. MUST be sized to accomodate bin_length +1
	@param buf_size Size of return_string buffer
	@param input Input hex string, can lead with 0x or not.
	@return Returns 0 if successful, -1 if not
 */
int hex_string_to_bin_string(char *return_string, size_t buf_size, const char *input);

/**
	@fn void log_message(const char *message)
	@brief If in verbose mode, logs message
	@param message Message to log
 */
void log_message(const char *message);

#endif /* SMD_UTILITIES_H */

