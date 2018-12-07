
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>

#include "../cmd.h"
#include "../utils/readwrite.h"
#include "../utils/search.h"
#include "../utils/get_put_block.h"
#include "../utils/error_manager.h"

extern PROC *running;
extern int block_size;


void ls_file(MINODE *mip, char *name)
{
	int mode, links, uid, gid, size;
	char *time;
	static const char* Permissions = "rwxrwxrwx";
	INODE *ip = &mip->ip;

	mode   = ip->i_mode;
    links  = ip->i_links_count;
    uid    = ip->i_uid;
    gid    = ip->i_gid;
    size   = ip->i_size;

    time = ctime((time_t*)&ip->i_mtime);
    time[strlen(time) - 1] = 0;
    
    switch(mode & 0xF000) //filetypes (directory, link, etc)
    {
        case 0x8000:  putchar('-');     break; // 0x8 = 1000
        case 0x4000:  putchar('d');     break; // 0x4 = 0100
        case 0xA000:  putchar('l');     break; // oxA = 1010
        default:      putchar('?');     break;
    }
	//print permissions
    for(int i = 0; i < strlen(Permissions); i++)
    {
        putchar(mode & (1 << (strlen(Permissions) - 1 - i)) ? Permissions[i] : '-');
    }
	//print rest of metadata
    printf("%d %d %d %d %26s %s", links, gid, uid, size, time, name);

	//if a link, print what link points to
    if(S_ISLNK(ip->i_mode))
    {
        printf(" -> %s", (char*)ip->i_block);
    }

    printf("\n");
}
void ls_dir(MINODE *mip)
{
	char buf[BLOCK_SIZE], buf_ind[BLOCK_SIZE], buf_double[BLOCK_SIZE], *current_ptr, name[256];
	DIR *dp;
	INODE *parent_ip = &mip->ip;
	MINODE *dir_entry_mip;
	int i, dev = mip->dev, ind_index, double_index;
	int *indirect, *double_indirect;


	//direct blocks
	for(i = 0; i < NUM_DIRECT_BLOCKS; i++)
	{
		if(parent_ip->i_block[i] == 0)
		{
			continue;
		}

		get_block(dev, parent_ip->i_block[i], buf);

        current_ptr = buf;
        dp = (DIR*)buf;

        while(current_ptr < buf + block_size)//traverse thru dir entries, print em out
        {
        	//dont print '.' and '..' entries
        	if(((strncmp(dp->name, ".", dp->name_len)) != 0) && 
        		((strncmp(dp->name, "..", dp->name_len)) != 0))
        	{
        		strncpy(name, dp->name, dp->name_len);
        		name[dp->name_len] = 0;	//set last character of name[] to 0

        		dir_entry_mip = get_minode(dev, dp->inode);
        		ls_file(dir_entry_mip, name);
        		
        		put_minode(dir_entry_mip);
        	}        	   
        	current_ptr += dp->rec_len;
        	dp = (DIR*)current_ptr;
		}
	}

	//indirect blocks
	if(parent_ip->i_block[INDIRECT_BLOCK_NUMBER])
	{
		get_block(dev, parent_ip->i_block[INDIRECT_BLOCK_NUMBER], buf_ind);
		indirect = (int *)buf_ind;

		for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
		{
			if(indirect[i] == 0)
			{
				continue;
			}

			get_block(dev, indirect[i], buf);

	        current_ptr = buf;
	        dp = (DIR*)buf;

	        while(current_ptr < buf + block_size)
	        {
	        	//dont print '.' and '..' entries
	        	if(((strncmp(dp->name, ".", dp->name_len)) != 0) && 
	        		((strncmp(dp->name, "..", dp->name_len)) != 0))
	        	{
	        		strncpy(name, dp->name, dp->name_len);
	        		name[dp->name_len] = 0;

	        		dir_entry_mip = get_minode(dev, dp->inode);
	        		ls_file(dir_entry_mip, name);
	        		
	        		put_minode(dir_entry_mip);
	        	}        	   
	        	current_ptr += dp->rec_len;
	        	dp = (DIR*)current_ptr;
			}
		}
	}
	//double indirect blocks
	if(parent_ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER])
	{
		get_block(dev, parent_ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER], buf_double);
		double_indirect = (int *)buf_double;

		for(double_index = 0; double_index < BLOCK_NUMBERS_PER_BLOCK; double_index++)
		{
			if(double_indirect[double_index] == 0)
			{
				continue;
			}

			get_block(dev, double_indirect[double_index], buf_ind);
			indirect = (int *)buf_ind;

			for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
			{
				if(indirect[i] == 0)
				{
					continue;
				}
				get_block(dev, indirect[i], buf);

				current_ptr = buf;
		        dp = (DIR*)buf;

		        while(current_ptr < buf + block_size)
		        {
		        	//dont print '.' and '..' entries
		        	if(((strncmp(dp->name, ".", dp->name_len)) != 0) && 
		        		((strncmp(dp->name, "..", dp->name_len)) != 0))
		        	{
		        		strncpy(name, dp->name, dp->name_len);
		        		name[dp->name_len] = 0;

		        		dir_entry_mip = get_minode(dev, dp->inode);
		        		ls_file(dir_entry_mip, name);
		        		
		        		put_minode(dir_entry_mip);
		        	}        	   
		        	current_ptr += dp->rec_len;
		        	dp = (DIR*)current_ptr;
				}
			}
		}
	}
}

int js_ls(int argc, char *argv[])
{
	int ino, device = running->cwd->dev, i = 1;
	MINODE *mip;

	if(argc < 2)
	{
		ls_dir(running->cwd);
		return 0;
	}

	while(i < argc)
	{
		ino = get_inode_number(argv[i]);
		if(ino < 0)
		{	
			set_error("File does not exist");
			return -1;
		}

		mip = get_minode(device, ino);

		if(S_ISDIR(mip->ip.i_mode))
		{
			if(argc > 2)
			{
				printf("%s:\n", argv[i]);
			}
			ls_dir(mip);
		}
		else
		{
			ls_file(mip, argv[i]);
		}

		put_minode(mip);
		i++;
	}
	return 0;
}