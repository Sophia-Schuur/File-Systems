#ifndef _READWRITE_H_
#define _READWRITE_H_

#include "type.h"

/*-----------------------------------------
Function: get_minode
Use: searches memory inodes, returns memory
	 inode or loads inode from disk
	 into memory if inode not found
Throws errors?: -get_block
-----------------------------------------*/
MINODE *get_minode(int dev, int ino);


/*-----------------------------------------
Function: put_minode
Use: checks if mip is dirty/refCount < 0,
	 if so writes mip information back to
	 disk
Throws errors?: -put_block
-----------------------------------------*/
int put_minode(MINODE *mip);


/*-----------------------------------------
Function: enter_dir_entry
Use: enters a new directory entry into the
	 file pointed at by 'parent_mip'
Throws errors?: -put_block
				-get_block
				-allocate_inode
-----------------------------------------*/
int enter_dir_entry(MINODE *parent_mip, int inode_number, char *name);

int remove_dir_entry(MINODE *parent_mip, char *name);


/*-----------------------------------------
Function: clear_blocks
Use: searches every possible block in *mip 
	 and clears it, not very efficient, should
	 use i_size to see how many blocks actually
	 need to be changed
Throws errors?: -put_block
				-get_block
-----------------------------------------*/
int clear_blocks(MINODE *mip);

#endif