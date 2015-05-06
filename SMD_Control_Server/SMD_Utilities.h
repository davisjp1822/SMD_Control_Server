//
//  SMD_Utilities.h
//  SMD_Control_Server
//
//  Created by John Davis on 4/28/15.
//  Copyright (c) 2015 3ML LLC. All rights reserved.
//

#ifndef SMD_UTILITIES_H
#define SMD_UTILITIES_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

struct Words {
	int32_t lower_word, upper_word;
};


int32_t convert_string_to_long_int(char *str);
struct Words convert_int_to_words(int32_t number);

#endif /* SMD_UTILITIES_H */