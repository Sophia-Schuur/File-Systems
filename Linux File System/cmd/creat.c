#define _DEFAULT_SOURCE

#include <string.h>
#include <libgen.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../cmd.h"
#include "../utils/search.h"
#include "../utils/bitmap.h"
#include "../utils/readwrite.h"
#include "../utils/error_manager.h"
#include "../utils/file.h"

extern MINODE *root;
extern PROC *running;
extern int block_size;

int js_creat(int argc, char *argv[])
{
	int i, parent_ino, device = running->cwd->dev;
	char *pathname, *parent = NULL, *child = NULL;
	MINODE *mip, *parent_mip;

	if(argc < 2)
	{
		set_error("creat: missing operand");
		return -1;
	}

	for(i = 1; i < argc; i++)
	{
		pathname = strdup(argv[i]);//copy contents of argv[i] into pathname terminated by null character
		if(check_null_ptr(pathname))
		{
			return -1;
		}

		if(pathname[0] == '/')
		{
			mip = root;
		}
		else
		{
			mip = running->cwd;
		}
		child = basename(pathname);///filename
		parent = dirname(pathname);	//./././
		
		
		
		parent_ino = get_inode_number(parent);//get inode number of parent directory (where the file exists)
		if(parent_ino < 0)
		{
			set_error("File does not exist");
			return -1;
		}

		parent_mip = get_minode(device, parent_ino);//get mip of parent
		//- - error checking for awhile - -
		if(thrown_error == TRUE)
		{
			put_minode(parent_mip);//for refcount
			free(pathname);
			return -1;
		}

		if(!S_ISDIR(parent_mip->ip.i_mode))
		{
			put_minode(parent_mip);
			free(pathname);
			set_error("Not a dir\n");
			return -1;
		}
		if(get_inode_number(argv[i]) > 0)
		{
			put_minode(parent_mip);
			free(pathname);
			set_error("File already exists");
			return -1;
		}
		//- - passed previous error checking - -
		creat_file(parent_mip, child);//actually make the file

		parent_mip->ip.i_atime = time(0L);
		parent_mip->dirty = TRUE;

		put_minode(parent_mip);//write parent back to disk
		free(pathname);
	}
	return 0;
}