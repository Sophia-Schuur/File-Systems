#define _DEFAULT_SOURCE

#include <time.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../utils/readwrite.h"
#include "../cmd.h"
#include "../utils/search.h"
#include "../utils/bitmap.h"
#include "../utils/error_manager.h"


extern PROC *running;

int js_unlink(int argc, char *argv[])
{
	int device = running->cwd->dev, inode_number, parent_ino, i, j;
	char *childc, *parentc, *parent, *child;
	MINODE *mip, *parent_mip;

	if(argc < 2)
	{
		set_error("unlink: missing operand");
		return -1;
	}

	for(i = 1; i < argc; i++)
	{
		//- - error checking -- 
		inode_number = get_inode_number(argv[i]);	//get ino of basename
		if(inode_number < 0)
		{
			set_error("File does not exist");
			return -1;
		}

		mip = get_minode(device, inode_number);

		if(S_ISDIR(mip->ip.i_mode))
		{
			set_error("unlink: cannot unlink dir");
			put_minode(mip);
			return -1;
		}

		if(mip->refCount > 1)
		{
			set_error("File busy");
			put_minode(mip);
			return -1;
		}
		//- - error checking done -- 
		mip->ip.i_links_count--;
		mip->dirty = TRUE;

		if(mip->ip.i_links_count == 0 && !S_ISLNK(mip->ip.i_mode))
		{
			clear_blocks(mip);
		}
		//if symbolic link, deallocate inode
		else if(S_ISLNK(mip->ip.i_mode))
		{
			deallocate_inode(device, inode_number);
		}
		//argv[i] + null character
		childc = strdup(argv[i]);
		parentc = strdup(argv[i]);

		child = basename(childc);
		parent = dirname(parentc);



		parent_ino = get_inode_number(parent);
		if(parent_ino < 0)
		{
			free(childc);
			free(parentc);
			set_error("File does not exist");
			return -1;
		}

		parent_mip = get_minode(device, parent_ino);

		clear_blocks(mip);
		remove_dir_entry(parent_mip, child);

		free(childc);
		free(parentc);
		
		put_minode(parent_mip);
		put_minode(mip);

		i++;

	}
	return 0;
}