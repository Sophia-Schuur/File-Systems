#define _DEFAULT_SOURCE

#include <time.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../cmd.h"
#include "../utils/search.h"
#include "../utils/readwrite.h"
#include "../utils/error_manager.h"

extern PROC *running;

int js_symlink(int argc, char *argv[])
{
	//like shortcuts. points to existing file
	int old_ino, new_ino, new_parent_ino, device = running->cwd->dev;
	MINODE *old_mip, *new_mip, *new_parent_mip;
	char *path, *child, *parent, buf[BLOCK_SIZE];

	//- - error checking - -
	if(argc < 3)
	{
		set_error("symlink <file1> <file2>");
		return -1;
	}

	if(strlen(argv[1]) >= 60)
	{
		set_error("Name must be less than 60 chars");
		return -1;
	}

	old_ino = get_inode_number(argv[1]);
	if(old_ino < 0)
	{
		set_error("File does not exist");
		return -1;
	}

	old_mip = get_minode(device, old_ino);
	if(thrown_error == TRUE)
	{
		put_minode(old_mip);
		return -1;
	}

	if(S_ISDIR(old_mip->ip.i_mode))
	{
		put_minode(old_mip);
		set_error("File cannot be a directory");
		return -1;
	}

	put_minode(old_mip);

	path = strdup(argv[2]);	//argv[2] + null character
	if(check_null_ptr(path))
	{
		put_minode(old_mip);
		return -1;
	}

	child = basename(path);
	parent = dirname(path);

	new_parent_ino = get_inode_number(parent);
	if(new_parent_ino < 0)
	{
		free(path);
		set_error("Directory does not exist");
		return -1;
	}

	new_parent_mip = get_minode(device, new_parent_ino);

	if(!S_ISDIR(new_parent_mip->ip.i_mode))
	{
		free(path);
		put_minode(new_parent_mip);
		set_error("File not a directory");
		return -1;
	}

	if(get_inode_number(argv[2]) > 0)
	{
		free(path);
		put_minode(new_parent_mip);
		set_error("File already exists");
		return -1;
	}

	//- - error checking done - -
	new_ino = creat_file(new_parent_mip, child);	//create symbolic link file. return inode number of file

	new_parent_mip->ip.i_atime = time(0L);
	new_parent_mip->dirty = TRUE;

	free(path);

	if(thrown_error == TRUE)
	{
		put_minode(new_parent_mip);
		return -1;
	}

	new_mip = get_minode(device, new_ino);	//get minode of new_ino

	strcpy((char*)new_mip->ip.i_block, argv[1]);	//copy filename to i_block

	new_mip->ip.i_mode = 0120777;
	new_mip->ip.i_size = strlen(argv[1]);
	new_mip->dirty = TRUE;
	
	put_minode(new_mip);
	put_minode(new_parent_mip);


	return 0;
}