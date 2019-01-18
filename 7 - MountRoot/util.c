#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <errno.h>

#include "type.h"

#include "parse.c"

//gcc mount_root.c -w
//./a.out KCmydisk

extern char* error_message;
extern int block_size, inodes_per_block, inode_table_block;

extern MINODE minodes[NMINODE];
extern MINODE *root;
extern PROC *running;


int get_block(int dev, int blk, char buf[])
{
	if(lseek(dev, (long)BLKSIZE * blk, 0) == -1)
	{
		error_message = strerror(errno);
		return -1;
	}

	if(read(dev, buf, BLKSIZE) == -1)
	{
		error_message = strerror(errno);
		return -1;
	}

	return 0;

}

int put_block(int dev, int blk, char buf[])
{
	int bytes;
	if(lseek(dev, (long)(block_size * blk), 0) == -1)
	{
		error_message = strerror(errno);
		return -1;
	}

	if(write(dev, buf, block_size) == -1)
	{
		error_message = strerror(errno);
		return -1;
	}

	return 0;

}

int put_inode(int dev, MINODE *mip)
{
	char buf[BLKSIZE];
	INODE *ip;
	int block, offset, ino = mip->ino;

	mip->refCount--;
	if(mip->refCount > 0) return -1;
	if(!mip->dirty) return -1;

	mip->dirty = 0;


	block = (ino - 1) / inodes_per_block + inode_table_block;
	offset = (ino - 1) % inodes_per_block;

	get_block(dev, block, buf);

	ip = (INODE *)buf;
	ip[offset] = mip->ip;

	return put_block(dev, block, (char*)ip);

}

int search(MINODE *mip, char *name)
{
	int dev = mip->dev, i;
	char buf[BLKSIZE], *cp;
	INODE *ip = &mip->ip;
	DIR *dp;

	if(!S_ISDIR(ip->i_mode))
	{
		error_message = "Not a dir";
		return -1;
	}

	//only checks direct blocks for now
	for(i = 0; i < (ip->i_size / block_size); i++)
	{	
		if(i > NUM_DIRECT_BLOCKS || !ip->i_block[i])
			break;

		get_block(dev, ip->i_block[i], buf);

		cp = buf;
		dp = (DIR *)buf;

		while(cp < (buf + block_size))
		{
			if(strncmp(dp->name, name, dp->name_len) == 0)
			{
				return dp->inode;
			}

			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
	}
	return 0;
}


//equivalent to 'iget()'
MINODE *get_minode(int dev, int ino)
{
	MINODE *mip;
	INODE *ip;
	char buf[BLKSIZE];
	int i, block, offset;

	if(ino < ROOT_INODE)
	{
		return 0;
	}

	for(i = 0; i < NMINODE; i++)
	{
		mip = &minodes[i];
		if(mip->refCount > 0)
			if(mip->dev == dev &&
				mip->ino == ino)
			{
				mip->refCount++;
				return mip;
			}

	}

	for(i = 0; i < NMINODE; i++)
	{
		mip = &minodes[i];
		if(mip->refCount == 0)
		{
			mip->dev = dev;
			mip->ino = ino;
			mip->refCount = 1;
			break;
		}
	}

	block = (ino - 1) / inodes_per_block + inode_table_block;
	offset = (ino - 1) % inodes_per_block;

	get_block(dev, block, buf);


	ip = (INODE *)buf + offset;

	mip->ip = *ip;

	return mip;
}	
//equivalent to 'getino()'
int get_inode_number(int dev, char *pathname)
{
	char **names;
    int i = 0, ino;     

	if(tokenize(pathname, "/", &names) < 0)
	{
		return -1;
	}

	if(pathname[0] == '/')
	{	
		ino = search(root, names[0]);
	}
	else
	{
		ino = search(running->cwd, names[0]);
	}

	if(ino <= 0)
	{
		free_array(names);
		error_message = "[!] ERROR: no such directory";
		return -1;
	}

	while(names[++i])
	{
		if((ino = search(get_minode(dev, ino), names[i])) <= 0)
		{
			free_array(names);
			error_message = "[!] ERROR: no such directory";
			return -1;
		}
	}

	free_array(names);
	return ino;
}