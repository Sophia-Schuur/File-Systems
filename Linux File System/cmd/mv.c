#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>


#include "../utils/get_put_block.h"
#include "../utils/error_manager.h"
#include "../utils/readwrite.h"
#include "../utils/search.h"

extern PROC *running;

int js_mv(int argc, char *argv[])
{
	int src_ino, dest_ino;
	MINODE *src_mip;

	if(argc < 3)
	{
		set_error("mv <file1> <file2");
		return -1;
	}

	src_ino = get_inode_number(argv[1]);	
	if(src_ino < 0)
	{
		set_error("File does not exist");
		return -1;
	}
	src_mip = get_minode(running->cwd->dev, src_ino);
	
	if(!S_ISREG(src_mip->ip.i_mode))
	{
		set_error("Not a normal file");
		return -1;
	}

	

	return 0;
}