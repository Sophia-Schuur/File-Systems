#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>


#include "../utils/get_put_block.h"
#include "../utils/error_manager.h"
#include "../utils/search.h"

extern PROC *running;

int js_mv(int argc, char *argv[])
{
	int src_ino;
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

	


	return 0;
}