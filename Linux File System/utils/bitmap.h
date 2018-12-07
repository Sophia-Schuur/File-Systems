#ifndef _BITMAP_C_
#define _BITMAP_C_

#include "type.h"





int test_bit(char *buf, int bit);
/*-----------------------------------------
Function: allocate_block
Use: loads block bitmap, tests each bit in
	 order, returns index of first unallocated 
	 block, -1 if none found 
Throws errors?: -get_block
				-put_block
				-decFreeBlocks
-----------------------------------------*/
int allocate_block(int dev);

/*-----------------------------------------
Function: allocate_inode
Use: loads inode bitmap, tests each bit in
	 order, returns index of first unallocated 
	 inode, -1 if none found 
Throws errors?: -get_block
				-put_block
				-decFreeInodes
-----------------------------------------*/
int allocate_inode(int dev);


/*-----------------------------------------
Function: deallocate_block
Use: loads block bitmap, switches 'block' 
	 bit to 0 
Throws errors?: -get_block
				-put_block
				-incFreeInodes
-----------------------------------------*/
int deallocate_block(int dev, int block);

/*-----------------------------------------
Function: deallocate_inode
Use: loads inode bitmap, switches 'ino' bit
	 to 0
Throws errors?: -get_block
				-put_block
				-incFreeBlocks
-----------------------------------------*/
int deallocate_inode(int dev, int ino);

void init_bitmap(int dev);

void clear_block(int dev, int block);


#endif
