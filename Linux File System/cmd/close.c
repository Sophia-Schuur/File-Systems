#include <stdlib.h>
#include <string.h>

#include "../utils/type.h"
#include "../utils/error_manager.h"
#include "../utils/readwrite.h"

extern PROC *running;

int js_close(int argc, char *argv[])
{

	int fd;
	OFT *ofp;

	if(argc < 2)
	{
		set_error("close <fd>");
		return -1;
	}

	if(strcmp(argv[1], "0") == 0)
	{
		fd = 0;
	}
	else
	{
		fd = atoi(argv[1]);
		if(fd == 0)
		{
			set_error("Invalid file descriptor");
			return -1;
		}
	}
	

	if(fd < 0 || fd > NOFT)
	{
		set_error("Invalid file descriptor");
		return -1;
	}

	if(running->fd[fd] == NULL)
	{
		set_error("Not open file descriptor");
		return -1;
	}
	else if(running->fd[fd]->refCount == 0)
	{
		set_error("Not open file descriptor");
		return -1;
	}

	ofp = running->fd[fd];
	running->fd[fd] = 0;

	ofp->refCount--;

	if(ofp->refCount == 0)
	{
		put_minode(ofp->mptr);
	}
	return 0;
}