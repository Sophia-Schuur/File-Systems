#include "input.h"


//read input from command line (user)
int get_input(char **buf)
{
	int c, i = 0, size = INITIAL_BUF_SIZE;;	//8

	(*buf) = (char *)malloc(size * sizeof(char));	//dynamically allocate buf
	
	if(check_null_ptr((*buf)) == TRUE)
	{
		return -1;
	}

	(*buf)[0] = 0;	//set buf to null

	while(TRUE)	//while there is stuff to read
	{
		c = getchar();	//read character

		if((i + 1) >= size)	
		{
			size *= 2;	//reallocate space to buf if i exceeds size. doesn't break pwd if pathanmes get too long
			(*buf) = (char *)realloc((*buf), size * sizeof(char));

			if(check_null_ptr((*buf)))
			{
				return -1;
			}
		}

		if(c == ' ')	//space?
		{
			while(c == ' ')	//if there is a space read the next character
			{
				c = getchar();
			}
			
			if(i != 0 && c != '\n')
			{
				(*buf)[i] = ' ';	
				i++;
			}
		}

		if(c == '\n')	//new line?
		{
			(*buf)[i] = 0;	//set to 0 
			if((*buf)[0] == '\0')	//if first character at buf is newline 
			{
				(*buf) = NULL;	//set entire buf to empty and exit function
			}
			return 0;
		}
		(*buf)[i] = c;	//set buf[i] to character, increment
		i++;
	}
}