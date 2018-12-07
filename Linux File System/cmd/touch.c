#include <time.h>

#include "../cmd.h"
#include "../utils/search.h"
#include "../utils/readwrite.h"
#include "../utils/error_manager.h"

extern PROC *running;

int js_touch(int argc, char *argv[])
{
	int device = running->cwd->dev, i, target_inode;
	MINODE *mip;

	for(i = 1; i < argc; i++)
	{
		target_inode = get_inode_number(argv[i]);	//get ino of basename
		if(target_inode < 0)	//target inode doesnt exist
		{
			//touch will do same thing as creat if the filename doesnt already exist
			char *my_argv[] = { "creat", argv[i], NULL };	
			js_creat(2, my_argv);
			if(thrown_error == TRUE)
			{
				return -1;
			}
		}
		else
		{
			mip = get_minode(device, target_inode);	//get minode of ino
			if(thrown_error == TRUE)
			{
				return -1;
			}
			//update the date of last modification on a file
			mip->ip.i_atime = time(0L);
			mip->dirty = TRUE;

			put_minode(mip);	//write it back
			if(thrown_error == TRUE)
			{
				return -1;
			}
		}
	}
	return 0;
}