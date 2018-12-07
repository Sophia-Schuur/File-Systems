#include <string.h>
#include <stdio.h>

#include "bitmap.h"

#include "get_put_block.h"
#include "readwrite.h"
#include "error_manager.h"

extern SUPER *sp;
extern GD *gp;

extern int block_bitmap, inode_bitmap, nblocks, ninodes;

//8 bits in a byte
//test whether or not a bit at buf[byte] is 1 or 0
int test_bit(char *buf, int bit)
{
	//convert bits to byte
	int byte = bit / 8;
	bit = bit % 8;
	
	if (buf[byte] & (1 << bit))
	{
		return 1;
	}
	return 0;
}

//set a bit at buf[byte] to 1
void set_bit(char *buf, int bit)
{
	int byte = bit / 8;
	bit = bit % 8;

	buf[byte] |= (1 << bit);
}

//set a bit at buf[byte] to 0
void clear_bit(char *buf, int bit)
{
	int byte = bit / 8;
	bit = bit % 8;

	buf[byte] &= ~(1 << bit);
}

int decFreeBlocks(int dev)
{
	//decrement one free block from the superblock and group descriptor each
	sp->s_free_blocks_count--;
	gp->bg_free_blocks_count--;

	//write the data in sp/gp to the superblock/group descriptor on device dev
	put_block(dev, SUPER_BLOCK_OFFSET, (char*)sp);
	put_block(dev, GD_BLOCK_OFFSET, (char*)gp);

	return 0;
}


int decFreeInodes(int dev)
{
	//decrement one free inode from the superblock and group descriptor each
	sp->s_free_inodes_count--;
	gp->bg_free_inodes_count--;

	//write the data in sp/gp to the superblock/group descriptor on device dev
	put_block(dev, SUPER_BLOCK_OFFSET, (char*)sp);
	put_block(dev, GD_BLOCK_OFFSET, (char*)gp);

	return 0;
}

int incFreeBlocks(int dev)
{
	//increment one free block from the superblock and group descriptor each
	sp->s_free_blocks_count++;
	gp->bg_free_blocks_count++;

	//write the data in sp/gp to the superblock/group descriptor on device dev
	put_block(dev, SUPER_BLOCK_OFFSET, (char*)sp);
	put_block(dev, GD_BLOCK_OFFSET, (char*)gp);

	return 0;
}

int incFreeInodes(int dev)
{
	//increment one free inode from the superblock and group descriptor each
	sp->s_free_inodes_count++;
	gp->bg_free_inodes_count++;

	//write the data in sp/gp to the superblock/group descriptor on device dev
	put_block(dev, SUPER_BLOCK_OFFSET, (char*)sp);
	put_block(dev, GD_BLOCK_OFFSET, (char*)gp);

	return 0;
}

/*-----------------------------------------
Function: allocate_block
Use: loads block bitmap, tests each bit in
	 order, returns index of first unallocated
	 block, -1 if none found
Throws errors?: -get_block
				-put_block
				-decFreeBlocks
-----------------------------------------*/
int allocate_block(int dev)
{
	//"block group" = one super, one gd, etc
	int i;
	char buf[BLOCK_SIZE];

	//load block bitmap into buf, checks for an error
	get_block(dev, block_bitmap, buf);	//block_bitmap = gp->bg_block_bitmap 
	if(thrown_error == TRUE)				//(represents which blocks in this block group are used or free by a 1 or 0 bit)
	{
		return -1;
	}

	for(i = 0; i < nblocks; i++)	//nblocks = sp->s_blocks_count (total blocks in entire system)
	{
		if(test_bit(buf, i) == 0)	//is this bit empty?
		{
			set_bit(buf, i);	//set bit to 1, now it is not empty
			decFreeBlocks(dev);	//decrement a free block in the system

			put_block(dev, block_bitmap, buf);		//write buf data to block bitmap

			clear_block(dev, i);

			return i;	//return index of this block (which is in i)
		}
	}
	set_error("No more free blocks");
	return -1;
}

/*-----------------------------------------
Function: allocate_inode
Use: loads inode bitmap, tests each bit in
	 order, returns index of first unallocated
	 inode, -1 if none found
Throws errors?: -get_block
				-put_block
				-decFreeInodes
-----------------------------------------*/
int allocate_inode(int dev)
{
	int i;
	char buf[BLOCK_SIZE];

	get_block(dev, inode_bitmap, buf);	//inode_bitmap = gp->bg_inode_bitmap;
											//(represents which inodes in this block group (not entire system) are used or free by a 1 or 0 bit)

	for(i = 0; i < ninodes; i++)	//ninodes = sp->s_inodes_count (total inodes in system)
	{
		if(test_bit(buf, i) == 0)	//is this bit empty?
		{
			set_bit(buf, i);	//set bit to 1, now it is not empty
			decFreeInodes(dev);	//decrement a free inode in the system
				
			put_block(dev, inode_bitmap, buf);	//write buf data to inode bitmap

			return i + 1;	//return index of this inode (which is in i)
		}
	}
	set_error("No more free inodes");
	return -1;
}

/*-----------------------------------------
Function: deallocate_block
Use: loads block bitmap, switches 'block'
	 bit to 0
Throws errors?: -get_block
				-put_block
				-incFreeInodes
-----------------------------------------*/
int deallocate_block(int dev, int block)
{
	char buf[BLOCK_SIZE];
	if(block <= 0)	//can't have negative block 
	{
		set_error("block number invalid (bitmap.c)");
		return -1;
	}

	get_block(dev, block_bitmap, buf);	//load block bitmap into buf, check for error
	if(thrown_error == TRUE)
	{
		return -1;
	}

	clear_bit(buf, block);	//set block bit to 0 (empty)
	incFreeBlocks(dev);	//increment a free block as it is now empty

	put_block(dev, block_bitmap, buf);	//write back cleared buf bits to block bitmap

	return 0;
}


/*-----------------------------------------
Function: deallocate_inode
Use: loads inode bitmap, switches 'ino' bit
	 to 0
Throws errors?: -get_block
				-put_block
				-incFreeBlocks
-----------------------------------------*/
int deallocate_inode(int dev, int ino)
{
	char buf[BLOCK_SIZE];
	if(ino <= 0)	//can't have negative ino
	{
		set_error("inode number invalid (bitmap.c)");
		return -1;
	}

	get_block(dev, inode_bitmap, buf);	//load block bitmap into buf, check for error
	clear_bit(buf, ino);	//set ino bit to 0 (empty)
	incFreeInodes(dev);	//increment a free inode as it is now empty

	put_block(dev, inode_bitmap, buf);	//write buf data to inode bitmap

	return 0;
}

void clear_block(int dev, int block)
{
	char buf[BLOCK_SIZE];
	get_block(dev, block, buf);
	memset(buf, 0, BLOCK_SIZE);
	put_block(dev, block, buf);
}