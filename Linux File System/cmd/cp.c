#define _DEFAULT_SOURCE

#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>

#include "../utils/file.h"
#include "../utils/error_manager.h"
#include "../utils/readwrite.h"
#include "../utils/search.h"

extern PROC *running;

int js_cp(int argc, char *argv[])
{
	int srcfd, destfd, nbytes, dev = running->cwd->dev;
	int srcino, destino, parent_ino;
	MINODE *src_mip, *dest_mip, *parent_mip;
	char *parent, *child, *basec, *dirc, buf[BLOCK_SIZE];
	OFT *ofp;

	if(argc < 3)
	{
		set_error("cp <source> <dest>");
		return -1;
	}

	srcino = get_inode_number(argv[1]);
	if(srcino < 0)
	{
		set_error("File does not exist");
		return -1;
	}

	destino = get_inode_number(argv[2]);

	if(destino < 0)
	{
		basec = strdup(argv[2]);
		dirc = strdup(argv[2]);

		parent = dirname(dirc);
		child = basename(basec);

		parent_ino = get_inode_number(parent);
		if(parent_ino < 0)
		{
			set_error("File does not exist");
			return -1;
		}

		parent_mip = get_minode(dev, parent_ino);
		destino = creat_file(parent_mip, child);

		put_minode(parent_mip);

		if(thrown_error == TRUE)
		{
			return -1;
		}
	}

	src_mip = get_minode(dev, srcino);
	dest_mip = get_minode(dev, destino);

	srcfd = open_file(src_mip, READ);
	destfd = open_file(dest_mip, WRITE);

	if(thrown_error == TRUE)
	{
		put_minode(src_mip);
		put_minode(dest_mip);
		return -1;
	}

	while((nbytes = myread(srcfd, buf, BLOCK_SIZE)))
	{
		mywrite(destfd, buf, nbytes);
	}

	ofp = running->fd[srcfd];
	running->fd[srcfd] = 0;

	ofp->refCount--;
	if(ofp->refCount == 0)
	{
		put_minode(ofp->mptr);
	}

	ofp = running->fd[destfd];
	running->fd[destfd] = 0;

	ofp->refCount--;
	if(ofp->refCount == 0)
	{
		put_minode(ofp->mptr);
	}
	
	return 0;
}