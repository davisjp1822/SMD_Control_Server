//
//  SMD_Utilities.c
//  SMD_Control_Server
//
//  Created by John Davis on 4/28/15.
//  Copyright (c) 2015 3ML LLC. All rights reserved.
//

#include "SMD_Utilities.h"

int32_t convert_string_to_long_int(char *str) {
	
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