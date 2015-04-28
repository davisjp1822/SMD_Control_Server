//
//  SMD_Utilities.c
//  SMD_Control_Server
//
//  Created by John Davis on 4/28/15.
//  Copyright (c) 2015 3ML LLC. All rights reserved.
//

#include "SMD_Utilities.h"

uint16_t convert_string_to_long_int(char *str) {
	
	//convert the token from string to long int
	char *end;
	long value = strtol(str, &end, 10);
	if (end == str || errno == ERANGE) {
		perror("strtol");
		return -1;
	}
	
	else {
		return value;
	}
}