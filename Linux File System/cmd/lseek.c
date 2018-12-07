#include <stdlib.h>
#include <stdio.h>

#include "../utils/type.h"
#include "../utils/error_manager.h"

extern PROC *running;

int js_lseek(int argc, char *argv[])
{
	int fd, offset, original_pos;
	OFT *ofp;

	if(argc < 3)
	{
		set_error("lseek <file> <offset>");
		return -1;
	}

	fd = atoi(argv[1]);
	offset = atoi(argv[2]);

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

	if((offset + ofp->offset) >= ofp->mptr->ip.i_size ||
		(offset + ofp->offset) <= 0)
	{
		set_error("Offset outside bounds of file");
		return -1;
	}

	original_pos = ofp->offset;

	ofp->offset += offset;

	return original_pos;

}