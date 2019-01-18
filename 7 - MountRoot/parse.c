#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <errno.h>

extern char *error_message;


#define INITIAL_BUF_SIZE 8

int tokenize(char *str, char *delimiters, char ***buf)
{
	char *tok = ""; 
	int size = INITIAL_BUF_SIZE; int i = 0;

	(*buf) = (char**)malloc(size * sizeof(char*));

	if((*buf) == NULL)
	{
		error_message = strerror(errno);
		return -1;
	}

	tok = strtok(str, delimiters);

	while(tok && strcmp(tok, "") != 0 && tok !=NULL)
	{
		(*buf)[i] = (char*)malloc(strlen(tok) + 1);

		if((*buf)[i] == NULL)
		{
			error_message = strerror(errno);
			return -1;
		}

		strcpy((*buf)[i], tok);

		if(i >= size)
		{
			size *= 2;
			(*buf) = (char**)realloc((*buf), (size * sizeof(char*)));
			if(*buf == NULL)
			{
				error_message = strerror(errno);
				return -1;
			}
		}

		tok = strtok(NULL, delimiters);
		i++;
	}

	(*buf)[i] = 0;

	return 0;
}

void free_array(char **str)
{
	int i = 0;
	while(str[i])
	{
		//testing	
		/* printf("Freeing: %s\n", str[i]); */

		free(str[i]);
		i++;	
	}
}