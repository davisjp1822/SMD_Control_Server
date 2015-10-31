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

//TODO - this implementation is unsafe
void tokenize_client_input(char **array_of_commands, const char *input, int num_tokens) {

	char *token, *string, *tofree;
	tofree = string = strdup(input);
	num_tokens = 0;
	
	while ((token = strsep(&string, ",")) != NULL) {
		array_of_commands[num_tokens] = (char*)malloc(sizeof(char) * (strlen(token) + 1 ) );
		strcpy(array_of_commands[num_tokens], token);
		num_tokens++;
	}
	
	free(tofree);
}

void log_message(const char *message) {
	
	if(VERBOSE > 0) {
		fprintf(stderr, "%s", message);
	}
}

int hex_string_to_bin_string(char *return_string, size_t buf_size, const char *input) {
	
	/*
	 0 - 0000
	 1 - 0001
	 2 - 0010
	 3 - 0011
	 4 - 0100
	 5 - 0101
	 6 - 0110
	 7 - 0111
	 8 - 1000
	 9 - 1001
	 A - 1010
	 B - 1011
	 C - 1100
	 D - 1101
	 E - 1110
	 F - 1111
	 */
	
	//check input to make sure length is good
	if(strlen(input) == 0 ) {
		
		return -1;
	}
	
	//check for leading '0x' and strip if necessary
	if(input[0] == '0' && input[1] == 'x') {
		input += 2;
	}
	
	//loop through input and replace
	int i;
	
	char bits[5];
	char *temp = NULL;
	int temp_idx_counter = 0;
	
	for(i=0; i<strlen(input); i++) {
		
		switch(input[i]) {
				
			case '0': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "0000");
				break;
			}
			case '1': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "0001");
				break;
			}
			case '2': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "0010");
				break;
			}
			case '3': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "0011");
				break;
			}
			case '4': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "0100");
				break;
			}
			case '5': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "0101");
				break;
			}
			case '6': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "0110");
				break;
			}
			case '7': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "0111");
				break;
			}
			case '8': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "1000");
				break;
			}
			case '9': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "1001");
				break;
			}
			case 'A': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "1010");
				break;
			}
			case 'B': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "1011");
				break;
			}
			case 'C': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "1100");
				break;
			}
			case 'D': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "1101");
				break;
			}
			case 'E': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "1110");
				break;
			}
			case 'F': {
				
				temp = realloc(temp, 4);
				temp_idx_counter +=4;
				snprintf(bits, sizeof(bits), "1111");
				break;
			}
				
			default: {
				
				return -1;
			}
		}
		
		//add the bits to the end of the temp string
		strlcat(temp, bits, temp_idx_counter+1);
		
	}
	
	//bounds check the return string
	if(temp_idx_counter > buf_size) {
		fprintf(stderr, "Buffer too small!\n");
		
		free(temp);
		return -1;
	}
	
	//copy the pointer if the buffer size is agreeable
	else {
		
		strlcpy(return_string, temp, buf_size+1);
		
		free(temp);
		return 0;
	}
	
}
