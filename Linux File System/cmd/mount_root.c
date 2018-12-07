#include <stdlib.h>
#include <stdio.h>
#include "../cmd.h"

#include "../utils/bitmap.h"


#include "../utils/error_manager.h"
#include "../utils/readwrite.h"
#include "../utils/get_put_block.h"
#include "../utils/error_manager.h"


extern MINODE minodes[NMINODE];
extern MINODE *root;
extern PROC proc[NPROC], *running;
extern OFT oft[NOFT];

extern int block_bitmap, inode_bitmap, inode_table_block;
extern int nblocks, ninodes, block_size, inode_size, inodes_per_block, iblk;

extern SUPER *sp;
extern GD *gp;

int mount_root(int dev)
{
	char buf[BLOCK_SIZE];
	
	int i, j;
	error_message = "";
  	MINODE *mip;
  	PROC   *p;
  	OFT *ofp;

	//initialize mip and p
  	for (i=0; i<NMINODE; i++)	//64
  	{
      	mip = &minodes[i];
      	mip->refCount = 0;
      	mip->dirty = FALSE;
      	mip->dev = 0;
      	mip->ino = 0;
  	}
  	for (i=0; i<NPROC; i++)	//2
  	{
       	p = &proc[i];
      	p->pid = i;
      	p->uid = i;
      	p->gid = 0;
      	p->cwd = NULL;
      	p->next = NULL;
      	p->cwd = 0;
      	for(j = 0; j < NFD; j++)
      	{
      		p->fd[j] = NULL;
      	}
  	}
	
  	for(i = 0; i < NOFT; i++)	//64
  	{
  		ofp = &oft[i];
  		ofp->mode = 0;
  		ofp->refCount = 0;
  		ofp->mptr = NULL;
  		ofp->offset = 0;
  	}

  	//get and record super block
  	sp = (SUPER *)malloc(sizeof(SUPER));
  	get_block(dev, SUPER_BLOCK_OFFSET, (char*)sp);
  	if(sp->s_magic != 0xEF53)
	{
		set_error("Not ext2 filesystem\n");
		return -1;
	}

	ninodes = sp->s_inodes_count;
	nblocks = sp->s_blocks_count;
	block_size = 1024 << sp->s_log_block_size;
	inode_size = sp->s_inode_size;

	//get and record group descriptor block
	gp = (GD *)malloc(BLOCK_SIZE);
	get_block(dev, GD_BLOCK_OFFSET, (char*)gp);

	block_bitmap = gp->bg_block_bitmap;		//represents which blocks in this block group are used or free by a 1 or 0 bit
	inode_bitmap = gp->bg_inode_bitmap;		//represents which inodes in this block group (not entire system) are used or free by a 1 or 0 bit
	inode_table_block = gp->bg_inode_table;	//block id of the first block of the "inode table" in this block group
	inodes_per_block = block_size / inode_size;

  get_block(dev, block_bitmap, buf);

  for (i=0; i < nblocks; i++){
    (test_bit(buf, i)) ? putchar('1') : putchar('0');
    if (i && (i % 8)==0)
       printf(" ");
  }
  printf("\n");

	root = get_minode(dev, ROOT_INODE);
	root->refCount++;

	running = &proc[0];
	running->status = READY;
	running->uid = SUPER_USER;
	running->cwd = root;

  	return 0;  	
}
