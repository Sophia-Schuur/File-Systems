#ifndef _ERROR_MANAGER_H_
#define _ERROR_MANAGER_H_

#include "type.h"

extern int thrown_error; 
extern char *error_message;

int check_null_ptr(void *ptr);

void set_error(char *message);

void print_error();

void ignore_error();


#endif