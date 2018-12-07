#include <sys/stat.h>

#include "../cmd.h"
#include "../utils/readwrite.h"
#include "../utils/search.h"
#include "../utils/error_manager.h"

extern MINODE *root;
extern PROC *running;

int js_cd(int argc, char *argv[])
{
	int ino, device = running->cwd->dev;
	MINODE *new_cwd, *old_cwd = running->cwd;


	//if no arguments provided, cd to root
	if(argc < 2)
	{
		put_minode(old_cwd);
		if(thrown_error == TRUE)
		{
			return -1;
		}
		running->cwd = root;
		running->cwd->refCount++;
		return 0;
	}

	ino = get_inode_number(argv[1]);	//get inode number of last token in pathname
	if(ino < 0)
	{
		set_error("File does not exist");
		return -1;
	}

	new_cwd = get_minode(device, ino);	//return minode of the ino
	if(thrown_error == TRUE)
	{
		return -1;
	}

	if(!S_ISDIR(new_cwd->ip.i_mode))	//if file
	{
		put_minode(new_cwd);
		if(thrown_error == TRUE)
		{
			return -1;
		}
		set_error("Cannot cd to a file\n");
		return -1;
	}

	put_minode(old_cwd);	//write old_cwd info back to disk
	if(thrown_error == TRUE)
	{
		return -1;
	}

	running->cwd = new_cwd;

	return 0;
}