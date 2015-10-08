//
//  SMD_Utilities.c
//  SMD_Control_Server
//
//  Created by John Davis on 4/28/15.
//  Copyright (c) 2015 3ML LLC. All rights reserved.
//

#include <unistd.h>

#include "SMD_Constants.h"
#include "SMD_Utilities.h"

int32_t convert_string_to_long_int(const char *str) {
	
	//convert the token from string to long int
	char *end;
	int32_t value = (int32_t)strtol(str, &end, 10);
	
	if (end == str || errno == ERANGE) {
		perror("strtol");
		return -1;
	}
	
	else {
		return value;
	}
}

struct Words convert_int_to_words(int32_t number) {
	
	int32_t pos_UW = 0;
	int32_t pos_LW = 0;
	
	//determine our speed upper words and lower words
	if(abs(number / 1000) > 0) {
		pos_LW = abs(number % 1000);
		pos_UW = abs(number - (number % 1000)) / 1000;
	}
	
	else {
		pos_LW = abs(number);
	}
	
	//are we negative?
	if(number < 0) {
		pos_UW = pos_UW * -1;
		pos_LW = pos_LW * -1;
	}
	
	//fprintf(stderr, "uw: %d\nlw: %d\n", pos_UW, pos_LW);
	struct Words returnWord = {pos_LW, pos_UW};
	return returnWord;
}

int write_to_client(int cl, const char *message) {
	
	//Send the message back to client
	if(write(cl, message , strlen(message)) == -1) {
		perror("Error writing to client");
		return -1;
	}
	else {
		return 0;
	}
}

int number_of_tokens(const char *command_string) {
	
	int num_tokens = 0;
	char *token, *string, *tofree;
	tofree = string = strdup(command_string);
	
	//check to see if there are spaces - abort if yes
	if(strchr(command_string, ' ') != NULL) {
		free(tofree);
		return -1;
	}
	
	while ((token = strsep(&string, ",")) != NULL) {
		num_tokens++;
	}
	
	free(tofree);
	return num_tokens;
}

void tokenize_client_input(char *array_of_commands, const char *input, int num_tokens) {

	char *token, *string, *tofree;
	tofree = string = strdup(input);
	num_tokens = 0;
	
	while ((token = strsep(&string, ",")) != NULL) {
		array_of_commands[num_tokens] = (char)malloc(sizeof(char) * (strlen(token) + 1 ) );
		strcpy(&array_of_commands[num_tokens], token);
		num_tokens++;
	}
	
	free(tofree);
}