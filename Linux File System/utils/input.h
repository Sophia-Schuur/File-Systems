#ifndef _INPUT_H_
#define _INPUT_H_


#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "type.h"
#include "error_manager.h"

int get_input(char **buf);

#endif