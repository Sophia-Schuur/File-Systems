#include <string.h>
#include <stdio.h>

#include "indirect.h"

#include "error_manager.h"
#include "readwrite.h"
#include "bitmap.h"
#include "get_put_block.h"


int assign_first_empty_bno(MINODE *mip, int bno)
{

	int i, ind_index, double_index;
	int *indirect, *double_indirect;
	INODE *parent_ip = &mip->ip;
	int block_number;
	int device = mip->dev;
	char *current_ptr, buf[BLOCK_SIZE], buf_ind[BLOCK_SIZE], buf_double[BLOCK_SIZE];

	//direct blocks
	for(i = 0; i < NUM_DIRECT_BLOCKS; i++)
	{
		if(parent_ip->i_block[i] == 0)
		{
			parent_ip->i_block[i] = bno;
			return 0;
		}
	}

	//indirect blocks
	if(parent_ip->i_block[INDIRECT_BLOCK_NUMBER] == 0)
	{
		parent_ip->i_block[INDIRECT_BLOCK_NUMBER] = allocate_block(mip->dev);
		mip->dirty = TRUE;
	}
	get_block(device, parent_ip->i_block[INDIRECT_BLOCK_NUMBER], buf_ind);
	indirect = (int *)buf_ind;

	for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
	{
		if(indirect[i] == 0)
		{
			indirect[i] = bno;
			put_block(device, parent_ip->i_block[INDIRECT_BLOCK_NUMBER], buf_ind);
			return 0;
		}
	}
	
	//double indirect blocks
	if(parent_ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER] == 0)
	{
		parent_ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER] = allocate_block(mip->dev);
		mip->dirty = TRUE;
	}

	get_block(device, parent_ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER], buf_double);
	double_indirect = (int *)buf_double;

	for(double_index = 0; double_index < BLOCK_NUMBERS_PER_BLOCK; double_index++)
	{
		if(double_indirect[double_index] == 0)
		{
			double_indirect[double_index] = allocate_block(mip->dev);
			put_block(device, parent_ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER], buf_double);
			return 0;
		}

		get_block(device, double_indirect[double_index], buf_ind);
		indirect = (int *)buf_ind;

		for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
		{
			if(indirect[i] == 0)
			{
				indirect[i] = bno;
				put_block(device, double_indirect[double_index], buf_ind);
				return 0;
			}
		}
	}

	set_error("you actually managed to run out of blocks...wow");
	return -1;
}