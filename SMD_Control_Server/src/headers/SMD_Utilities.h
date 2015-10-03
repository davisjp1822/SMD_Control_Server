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
#include <string.h>
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
	@fn int32_t convert_string_to_long_int(char *str)
	@brief Converts a string into a long int - useful for turning client commands into integers for accel, decel, speed, etc
	@param str string to convert into 32 bit integer
	@return int32_t
 */
int32_t convert_string_to_long_int(const char *str);

/**
	@fn int write_to_client(int cl)
	@brief Writes a message back to the client
	@param cl Integer specifying a valid socket handle
	@param message Message to send to client
	@return int 0 if successful
 */
int write_to_client(int cl, const char *message);

int number_of_tokens();

#endif /* SMD_UTILITIES_H */