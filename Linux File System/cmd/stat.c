#define _DEFAULT_SOURCE

#include <string.h>
#include <libgen.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>

#include "../cmd.h"
#include "../utils/readwrite.h"
#include "../utils/search.h"
#include "../utils/error_manager.h"

extern PROC *running;

int js_stat(int argc, char *argv[])
{
    int ino, i, device = running->cwd->dev;
    MINODE *mip;
    INODE *ip;
    char *cp, *my_atime, *my_mtime, *my_ctime, *name;
    DIR dp;

    for(i = 1; i < argc; i++)
    {
    	ino = get_inode_number(argv[i]); //get inode of basename
	    if(ino < 0)
	    {
	    	set_error("File does not exist");
	    	return -1;
	    }
	    mip = get_minode(device, ino);	//get minode of ino

	    name = basename(argv[i]);	//get basename of pathname

	    ip = &mip->ip;

		//print all the things
	    printf("  File: %s\n", name);
	    printf("  Size: %d\tBlocks: %12d ", ip->i_size, ip->i_blocks);
	    if(S_ISDIR(ip->i_mode))
	    {
	    	 printf("  Directory\n");
	    }        
	    else
	    {
	    	 printf("  File\n");
	    }              
	    printf("Inode: %d Links:%d \n", ino, ip->i_links_count);

	    my_atime = ctime((long int *)(time_t)&ip->i_atime);
	    my_mtime = ctime((long int *)(time_t)&ip->i_mtime);
	    my_ctime = ctime((long int *)(time_t)&ip->i_ctime);

	    printf("Access: %26s", my_atime);
	    printf("Modify: %26s", my_mtime);
	    printf("Change: %26s", my_ctime);

	    if(argc > 2)
	    {
	        printf("\n\n");	
	    }
	   
	    mip->dirty = 1;
	    put_minode(mip);
    }
    

          
	return 0;
}