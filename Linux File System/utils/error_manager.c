#include "error_manager.h"


#include <errno.h>
#include <string.h>
#include <stdio.h>

int check_null_ptr(void *ptr)
{
	if(ptr == NULL)
	{
		thrown_error = TRUE;
		error_message = strerror(errno);
		return TRUE;
	}
	return FALSE;
}

void set_error(char *message)
{
	if(thrown_error == FALSE)
	{
		thrown_error = TRUE;
		error_message = message;
	}
}

void print_error()
{
	printf("%s\n", error_message);
	thrown_error = FALSE;
}

void ignore_error()
{
	error_message = NULL;
	thrown_error = FALSE;
}