#include <string.h>
#include <stdlib.h>

#include "parse.h"
#include "error_manager.h"

/*-----------------------------------------
Function: parse
Use: tokenizes 'line' by the value of
	 'delimiters' and dynamically stores
	 result in 'buf.'
	 *DOES NOT AFFECT INPUT*
Throws errors?: -native
-----------------------------------------*/
int parse(const char *input, char *delimiters, char ***buf)
{
	char *tok, *copy;
	int size = INITIAL_BUF_SIZE;	//8
	int i = 0;

	(*buf) = (char**)malloc(size * sizeof(char*));	//dynamically allocate buf, check null
	if(check_null_ptr((*buf)))
	{
		return -1;
	}

	copy = (char *)malloc((strlen(input) + 1) * sizeof(char));	//dynamically allocate a copy, +1 for newline
	if(check_null_ptr(copy))
	{
		free_array(*buf);
		return -1;
	}

	strcpy(copy, input);	//copy input to copy

	tok = strtok(copy, delimiters);	//grab first token

	if(tok == NULL)	//can't have an empty token
	{
		//free everything, start again
		set_error("invalid input\n");
		free(copy);		
		free_array(*buf);
		return -1;
	}

	while(tok)		//while there is parsing to be done
	{
		//dynamically allocate length
		int length = strlen(tok) + 1;
		(*buf)[i] = (char*)malloc(length);

		if(check_null_ptr((*buf)[i]))
		{
			return -1;
		}

		strcpy((*buf)[i], tok);	//copy tok to buf[i]

		if((i + 1) == size)
		{
			size *= 2;		//if we have too many characters, multiply size by 2. doesn't break pwd if pathnames get too long

			(*buf) = (char**)realloc((*buf), size * sizeof(char*));
			if(check_null_ptr(*buf))
			{
				//if null, free it, start over
				free(copy);
				return -1;
			}
		}
		//go to next token
		tok = strtok(NULL, delimiters);
		i++;
	}
	(*buf)[i] = NULL;	//set last i to null

	free(copy);	//free copy, we do not need it anymore
	return i;
}


/*-----------------------------------------
Function: free_array
Use: properly frees dynamically allocated
	 two-dimensional char array
Throws errors?:
-----------------------------------------*/
void free_array(char **arr)
{
	int i = 0;
	
	if(!arr)
	{
		return;
	}

	while(arr[i])
	{
		free(arr[i]);
		i++;
	}

	free(arr);
}