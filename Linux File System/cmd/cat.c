#include <string.h>
#include <stdio.h>

#include "../cmd.h"
#include "../utils/file.h"
#include "../utils/search.h"
#include "../utils/error_manager.h"
#include "../utils/readwrite.h"

extern PROC *running;

int js_cat(int argc, char *argv[])
{
	char buf[BLOCK_SIZE];
	char print_buf[BLOCK_SIZE + 1];
	int n, ino, fd, mode;
	OFT *ofp;
	MINODE *mip;

	if(argc < 2)
	{
		set_error("cat <file>");
		return -1;
	}

	ino = get_inode_number(argv[1]);
	if(ino < 0)
	{
		set_error("File does not exist");
		return -1;
	}
	mip = get_minode(running->cwd->dev, ino);

	fd = open_file(mip, mode);
	if(thrown_error == TRUE)
	{
		return -1;
	}

	while((n = myread(fd, buf, BLOCK_SIZE)))
	{
		memcpy(print_buf, buf, BLOCK_SIZE);
		print_buf[n] = 0;

		printf("%s", print_buf);
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