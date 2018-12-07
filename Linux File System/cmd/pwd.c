#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "../cmd.h"

#include "../utils/readwrite.h"
#include "../utils/search.h"
#include "../utils/get_put_block.h"
#include "../utils/error_manager.h"


extern PROC *running;
extern MINODE *root;

int rpwd(MINODE *mip, char **path, int size)
{
	int device = mip->dev, i, my_ino, parent_ino;
	DIR *dp;
	MINODE *parent_mip;
	char buf[BLOCK_SIZE], *cp, *my_name;

	if(mip == root)
	{
		if(thrown_error == TRUE)
		{        	
			return -1;
		}
		strcat(*path, "/");
		return 0;
	} 

	get_block(device, mip->ip.i_block[0], buf);	//get data of first direct block in mip and put in buf

	cp = buf;
	dp = (DIR*)buf;

	my_ino = dp->inode;

	cp += dp->rec_len;
	dp = (DIR*)cp;

	parent_ino = dp->inode;

	parent_mip = get_minode(device, parent_ino);

	rpwd(parent_mip, path, size);

	find_my_name(parent_mip, my_ino, &my_name);	//get myname

	while(strlen(*path) + strlen(my_name) + 3 > size)	//if path is too big
	{
		size *= 2;

		(*path) = (char*)realloc(*path, size * sizeof(char));
		if(check_null_ptr(*path))
		{
			put_minode(mip);
			free(my_name);
			return -1;
		}
	}

	strcat(*path, my_name);
	free(my_name);

	if(mip != running->cwd)
	{
		strcat(*path, "/");
	}

	put_minode(parent_mip);		//write parent mip info back to disk
	if(thrown_error == TRUE)
	{        	
		return -1;
	}
	return 0;
}

int js_pwd(int argc, char *argv[])
{
	int device = running->cwd->dev, size = 64;
	char *buf;

	buf = (char*)malloc(size * sizeof(char*));	//dynamically allocate buf
	if(check_null_ptr(buf))
	{
		return -1;
	}

	buf[0] = '\0';	//set buf to null


	rpwd(running->cwd, &buf, size);
	if(thrown_error == TRUE)
	{
		free(buf);
		return -1;
	}

	printf("%s\n", buf);

	free(buf);
	return 0;
}