#include <stdlib.h>

#include "../cmd.h"

#include "../utils/search.h"
#include "../utils/readwrite.h"
#include "../utils/error_manager.h"

extern PROC *running;

int js_chown(int argc, char *argv[])
{
	int device = running->cwd->dev, ino;
	MINODE *mip;
	INODE *ip;
	long int new_own;

	if(argc < 2)
	{
		set_error("chown: missing operand\n");
		return -1;
	}

	ino = get_inode_number(argv[1]);
	if(ino < 0)
	{
		set_error("File does not exist");
		return -1;
	}

	mip = get_minode(device, ino);
	
	if(running->uid != mip->ip.i_uid && running->uid != SUPER_USER)
	{
		put_minode(mip);
		set_error("You don't have permission to do that\n");
		return -1;
	}

	ip = &mip->ip;

	new_own = strtol(argv[2], NULL, 10);	//set new_own to 
	ip->i_uid = new_own;

	mip->dirty = TRUE;
	put_minode(mip);

	return 0;
}