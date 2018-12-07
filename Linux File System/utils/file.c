#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "file.h"
#include "readwrite.h"
#include "get_put_block.h"
#include "error_manager.h"
#include "bitmap.h"


extern OFT oft[NOFT];
extern PROC *running;
extern int nblocks;

int duplicate_oft(MINODE *mip)
{
	int i;
	OFT *oftp;

	for(int i = 0; i < NOFT; i++)
	{
		oftp = &oft[i];


		if(oftp->refCount > 0)
		{
			if(oftp->mptr->dev == mip->dev && 
				oftp->mptr->ino == mip->ino)
			{
				if(oftp->mode != READ)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

OFT *get_oft()
{
	int i;
	OFT *oftp;

	for(int i = 0; i < NOFT; i++)
	{
		oftp = &oft[i];
		if(oftp->refCount == 0)
		{
			oftp->refCount++;	
			return oftp;
		}
	}
	set_error("No more memory open file tables");
	return NULL;
}

int creat_file(MINODE *parent_mip, char *name)
{
	int inode_number, block_number, device = parent_mip->dev, i;
	char buf[BLOCK_SIZE], *current_ptr;
	MINODE *mip;
	INODE *ip;
	DIR *dp;

	inode_number = allocate_inode(device);

	mip = get_minode(device, inode_number);


	ip = &mip->ip;

	ip->i_mode = 0100664;
	ip->i_uid  = running->uid;	
 	ip->i_gid  = running->gid;	
  	ip->i_size = 0;		
  	ip->i_links_count = 1;	       
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  
	ip->i_blocks = 0;                	
	
	for(i = 0; i < 15; i++)
	{
		ip->i_block[i] = 0;
	}

	mip->dirty = TRUE;
	
	put_minode(mip);

	enter_dir_entry(parent_mip, inode_number, name);

	return inode_number;
}

int myread(int fd, char *buf, int nbytes)
{
	int dev;
	int count = 0, remaining, available, logical_block, physical_block, start_byte;
	int ind_index, double_index;
	int size;
	char *src, *dest;
	int *indirect, *double_indirect;
	char local_buf[BLOCK_SIZE];
	OFT *ofp;
	char *cp, *buf_cp;
	INODE *ip;

	ofp = running->fd[fd];
	dev = ofp->mptr->dev;
	ip = &ofp->mptr->ip;
	available = ip->i_size - ofp->offset;
	buf_cp = buf;

	while(nbytes && available)
	{
		logical_block = ofp->offset / BLOCK_SIZE;
		start_byte = ofp->offset % BLOCK_SIZE;


		//direct
		if(logical_block < NUM_DIRECT_BLOCKS)
		{
			physical_block = ip->i_block[logical_block];
		}
		//indirect
		else if(logical_block >= NUM_DIRECT_BLOCKS && 
			logical_block < NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)
		{
			ind_index = logical_block - NUM_DIRECT_BLOCKS;
			get_block(dev, ip->i_block[INDIRECT_BLOCK_NUMBER], local_buf);
			indirect = (int *)local_buf;

			physical_block = indirect[ind_index];
		}
		//double indirect
		else
		{
			double_index = (logical_block - (NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)) / BLOCK_NUMBERS_PER_BLOCK;
			ind_index = (logical_block - (NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)) % BLOCK_NUMBERS_PER_BLOCK;

			get_block(dev, ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER], local_buf);
			double_indirect = (int *)local_buf;

			get_block(dev, double_indirect[double_index], local_buf);
			indirect = (int *)local_buf;

			physical_block = indirect[ind_index];
		}

		get_block(dev, physical_block, local_buf);

		cp = local_buf + start_byte;
		remaining = BLOCK_SIZE - start_byte;

		if(available <= remaining && available <= nbytes)
		{
			size = available;
		}
		else if(remaining <= available && remaining <= nbytes)
		{
			size = remaining;
		}
		else if(nbytes <= remaining && nbytes <= available)
		{
			size = nbytes;
		}

		memcpy(buf_cp, cp, size);
		ofp->offset += size;
		count += size;
		available -= size;
		nbytes -= size;
		remaining -= size;

	}

	return count;
}

int open_file(MINODE *mip, int mode)
{
	OFT *ofp;
	int i;

	if(duplicate_oft(mip) == TRUE)
	{
		set_error("File already opened for something other than READ");
		return -1;
	}

	ofp = get_oft();
	if(thrown_error == TRUE)
	{
		return -1;
	}

	ofp->mode = mode;
	ofp->mptr = mip;
	
	switch(ofp->mode)
	{
		case READ : ofp->offset = 0;
		break;
		case WRITE: clear_blocks(mip);
		break;
		case READ_WRITE: ofp->offset = 0;
		break;
		case APPEND: ofp->offset = mip->ip.i_size;
		break;
	}

	for(i = 0; i < NFD; i++)
	{
		if(running->fd[i] == NULL)
		{
			running->fd[i] = ofp;
			return i;
		}
	}
	set_error("Out of proc oft pointers");
	return -1;
}

int mywrite(int fd, char *buf, int nbytes)
{
	int dev;
	int count = 0, available, logical_block, physical_block, start_byte, ind_bno;
	int ind_index, double_index;
	int size, remaining;
	char *src, *dest;
	int *indirect, *double_indirect;
	char local_buf[BLOCK_SIZE];
	OFT *ofp;
	INODE *ip;
	char *cp, *buf_cp;


	ofp = running->fd[fd];
	dev = ofp->mptr->dev;
	ip = &ofp->mptr->ip;

	buf_cp = buf;

	while(nbytes)
	{
		logical_block = ofp->offset / BLOCK_SIZE;
		start_byte = ofp->offset % BLOCK_SIZE;

		if(logical_block < NUM_DIRECT_BLOCKS)
		{
			if(ip->i_block[logical_block] == 0)
			{
				ip->i_block[logical_block] = allocate_block(dev);
				ip->i_blocks += 2;
			}
			physical_block = ip->i_block[logical_block];
		}


		else if(logical_block >= NUM_DIRECT_BLOCKS && 
			logical_block < BLOCK_NUMBERS_PER_BLOCK + NUM_DIRECT_BLOCKS)
		{
			if(ip->i_block[INDIRECT_BLOCK_NUMBER] == 0)
			{
				ip->i_block[INDIRECT_BLOCK_NUMBER] = allocate_block(dev);
				ip->i_blocks += 2;
			}
			get_block(dev, ip->i_block[INDIRECT_BLOCK_NUMBER], local_buf);
			indirect = (int *)local_buf;
			
			if(indirect[logical_block - NUM_DIRECT_BLOCKS] == 0)
			{
				indirect[logical_block - NUM_DIRECT_BLOCKS] = allocate_block(dev);
				physical_block = indirect[logical_block - NUM_DIRECT_BLOCKS];
				put_block(dev, ip->i_block[INDIRECT_BLOCK_NUMBER], local_buf);
			}
		}

		else
		{
			double_index = (logical_block - (NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)) / BLOCK_NUMBERS_PER_BLOCK;
			ind_index = (logical_block - (NUM_DIRECT_BLOCKS + BLOCK_NUMBERS_PER_BLOCK)) % BLOCK_NUMBERS_PER_BLOCK;

			if(ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER] == 0)
			{
				ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER] = allocate_block(dev);
				ip->i_blocks += 2;
			}

			get_block(dev, ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER], local_buf);
			double_indirect = (int *)local_buf;

			if(double_indirect[double_index] == 0)
			{
				double_indirect[double_index] = allocate_block(dev);
				ind_bno = double_indirect[double_index];
				put_block(dev, ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER], local_buf);
				ip->i_blocks += 2;
			}

			get_block(dev, ind_bno, local_buf);
			indirect = (int *)local_buf;

			if(indirect[ind_index] == 0)
			{
				indirect[ind_index] = allocate_block(dev);
				put_block(dev, ind_bno, local_buf);
				ip->i_blocks += 2;
			}

			physical_block = indirect[ind_index];
		}

		get_block(dev, physical_block, local_buf);

		cp = local_buf + start_byte;
		remaining = BLOCK_SIZE - start_byte;

		if(remaining <= nbytes)
		{
			size = remaining;
		}
		else if(nbytes <= remaining)
		{
			size = nbytes;
		}

		memcpy(cp, buf_cp, size);
		ofp->offset += size;
		if(ofp->offset > ofp->mptr->ip.i_size)
		{
			ofp->mptr->ip.i_size += size;
		}
		count += size;
		available -= size;
		nbytes -= size;
		remaining -= size;

		if(thrown_error == TRUE)
		{
			return -1;
		}

	}

	ofp->mptr->dirty = TRUE;
	return count;
}