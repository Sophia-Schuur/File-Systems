#include "get_put_block.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#include "type.h"
#include "error_manager.h"

/*-----------------------------------------
Function: get_block
Use: loads data from block 'block' from
	 device 'dev.' Stores results in 'buf'
Throws errors?: -native
-----------------------------------------*/
int get_block(int dev, int blk, char buf[])
{
	if(lseek(dev, (long)BLOCK_SIZE * blk, 0) == -1)	//lseek() repositions the file offset of the open file description	
	{													//associated with the file descriptor dev to the argument blk
		set_error(strerror(errno));
		return -1;
	}
	if(read(dev, buf, BLOCK_SIZE) == -1)	//read() read up to BLOCK_SIZE bytes 
	{										//from file descriptor dev into the buffer starting at buf
		set_error(strerror(errno));
		return -1;
	}
	return 0;
}


/*-----------------------------------------
Function: put_block
Use: writes data in 'buf' to block 'block'
	 on device 'dev'
Throws errors?: -native
-----------------------------------------*/
int put_block(int dev, int blk, char buf[])
{
	if(lseek(dev, (long)BLOCK_SIZE * blk, 0) == -1)	//lseek() repositions the file offset of the open file description													
	{													//associated with the file descriptor dev to the argument blk
		set_error(strerror(errno));
		return -1;
	}
	if(write(dev, buf, BLOCK_SIZE) == -1)	//write() write up to BLOCK_SIZE bytes from 
	{										//the buffer pointed buf to the file referred to by the file descriptor DEV 
		set_error(strerror(errno));
		return -1;
	}
	return 0;
}
