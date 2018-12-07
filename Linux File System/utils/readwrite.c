#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "readwrite.h"

#include "get_put_block.h"
#include "../utils/bitmap.h"
#include "error_manager.h"
#include "../utils/indirect.h"


extern MINODE minodes[NMINODE];
extern int block_size, inodes_per_block, inode_table_block;
extern PROC *running;
extern OFT oft[NOFT];




MINODE *get_minode(int dev, int ino)
{
	MINODE *mip;
	INODE *ip, *table;
	char buf[BLOCK_SIZE];
	int i, block, offset;

	if(ino < ROOT_INODE)
	{
		return 0;
	}

	for(i = 0; i < NMINODE; i++)
	{
		mip = &minodes[i];

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
			mip->dirty = FALSE;
			mip->refCount = 1;
			
			block = (ino - 1) / inodes_per_block + inode_table_block;
			offset = (ino - 1) % inodes_per_block;

			get_block(dev, block, buf);

			ip = (INODE *)buf + offset;

			mip->ip = *ip;

			return mip;
		}
	}
	set_error((char*)"No more memory inodes\n");
	return 0;
}

int put_minode(MINODE *mip)
{
	if(!mip) 
	{
		return 0;	
	}
	if(--(mip->refCount) <= 0 && mip->dirty == TRUE)
	{
		int block, index;
		char buf[BLOCK_SIZE];
		INODE *inode_table;

		block = (mip->ino - 1) / inodes_per_block + inode_table_block;
		index = (mip->ino - 1) % inodes_per_block;

		get_block(mip->dev, block, buf);
		if(thrown_error == TRUE)
		{
			return -1;
		}

		inode_table = (INODE *)buf;
		inode_table[index] = mip->ip;

		put_block(mip->dev, block, (char*)inode_table);
		if(thrown_error == TRUE)
		{
			return -1;
		}
		mip->dirty = FALSE;
	}
	return 0;
}

int enter_dir_entry(MINODE *parent_mip, int inode_number, char *name)
{
	int i, ind_index, double_index, block_number, device = parent_mip->dev;
	int need_length, remaining_length, ideal_last_entry;
	int *indirect, *double_indirect;
	char buf[BLOCK_SIZE], buf_ind[BLOCK_SIZE], buf_double[BLOCK_SIZE], *current_ptr;
	DIR *dp;
	INODE *parent_ip = &parent_mip->ip;

	//direct blocks
	for(i = 0; i < NUM_DIRECT_BLOCKS; i++)
	{
		if(parent_ip->i_block[i] == 0)
		{
			continue;
		}

		block_number = parent_ip->i_block[i];

		get_block(device, block_number, buf);

		current_ptr = buf;
		dp = (DIR*)buf;

		while(current_ptr + dp->rec_len < buf + block_size)
		{
			current_ptr += dp->rec_len;
			dp = (DIR *)current_ptr;
		}

		need_length = (4 * (( 8 + strlen(name) + 3) / 4));
		ideal_last_entry = (4 * (( 8 + dp->name_len + 3) / 4));
		remaining_length = dp->rec_len - ideal_last_entry;

		if(remaining_length >= need_length)
		{
			dp->rec_len = ideal_last_entry;

			current_ptr += dp->rec_len;
			dp = (DIR*)current_ptr;

			dp->inode = inode_number;
			dp->rec_len = (block_size - (current_ptr - buf));
			dp->name_len = strlen(name);
			dp->file_type = EXT2_FT_DIR;
			strcpy(dp->name, name);

			put_block(device, block_number, buf);
			if(thrown_error == TRUE)
			{
				return -1;
			}
			return 0;
		}
	}

	//indirect blocks
	if(parent_ip->i_block[INDIRECT_BLOCK_NUMBER])
	{
		get_block(device, parent_ip->i_block[INDIRECT_BLOCK_NUMBER], buf_ind);
		indirect = (int *)buf_ind;

		for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
		{
			if(indirect[i] == 0)
			{
				continue;
			}
			get_block(device, indirect[i], buf);

			current_ptr = buf;
			dp = (DIR*)buf;

			while(current_ptr + dp->rec_len < buf + block_size)
			{
				current_ptr += dp->rec_len;
				dp = (DIR *)current_ptr;
			}

			need_length = (4 * (( 8 + strlen(name) + 3) / 4));
			ideal_last_entry = (4 * (( 8 + dp->name_len + 3) / 4));
			remaining_length = dp->rec_len - ideal_last_entry;

			if(remaining_length >= need_length)
			{
				dp->rec_len = ideal_last_entry;

				current_ptr += dp->rec_len;
				dp = (DIR*)current_ptr;

				dp->inode = inode_number;
				dp->rec_len = (block_size - (current_ptr - buf));
				dp->name_len = strlen(name);
				dp->file_type = EXT2_FT_DIR;
				strcpy(dp->name, name);

				put_block(device, indirect[i], buf);
				return 0;
			}
		}
	}
	//double indirect blocks
	if(parent_ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER])
	{
		get_block(device, parent_ip->i_block[DOUBLE_INDIRECT_BLOCK_NUMBER], buf_double);
		double_indirect = (int *)buf_double;

		for(double_index = 0; double_index < BLOCK_NUMBERS_PER_BLOCK; double_index++)
		{
			if(double_indirect[double_index] == 0)
			{
				continue;
			}

			get_block(device, double_indirect[double_index], buf_ind);
			indirect = (int *)buf_ind;

			for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
			{
				if(indirect[i] == 0)
				{
					continue;
				}
				get_block(device, indirect[i], buf);

				current_ptr = buf;
				dp = (DIR*)buf;

				while(current_ptr + dp->rec_len < buf + block_size)
				{
					current_ptr += dp->rec_len;
					dp = (DIR *)current_ptr;
				}

				need_length = (4 * (( 8 + strlen(name) + 3) / 4));
				ideal_last_entry = (4 * (( 8 + dp->name_len + 3) / 4));
				remaining_length = dp->rec_len - ideal_last_entry;

				if(remaining_length >= need_length)
				{
					dp->rec_len = ideal_last_entry;

					current_ptr += dp->rec_len;
					dp = (DIR*)current_ptr;

					dp->inode = inode_number;
					dp->rec_len = (block_size - (current_ptr - buf));
					dp->name_len = strlen(name);
					dp->file_type = EXT2_FT_DIR;
					strcpy(dp->name, name);

					put_block(device, indirect[i], buf);
					return 0;
				}
			}
		}
	}


	block_number = allocate_block(device);

	assign_first_empty_bno(parent_mip, block_number);
	parent_ip->i_size += block_size;
	parent_ip->i_blocks += block_size / 512;
	parent_mip->dirty = TRUE;

	get_block(device, block_number, buf);

	current_ptr = buf;
	dp = (DIR *)buf;

	dp->inode = inode_number;
	dp->rec_len = block_size;
	dp->name_len = strlen(name);
	dp->file_type = EXT2_FT_DIR;
	strcpy(dp->name, name);

	put_block(device, block_number, buf);

	return 0;
}

int remove_dir_entry(MINODE *parent_mip, char *name)
{
	int device = parent_mip->dev, i;
	char buf[BLOCK_SIZE], *current_ptr, *last_ptr, *start, *end;
	int *indirect, *double_indirect;
	char buf_ind[BLOCK_SIZE],  buf_double[BLOCK_SIZE];
	int ind_index, double_index;

	DIR *dp, *prev_dp, *last_dp;
	
	//direct blocks;
	for(i = 0; i < NUM_DIRECT_BLOCKS; i++)
	{
		if(parent_mip->ip.i_block[i] == 0)
		{
			continue;
		}

		get_block(device, parent_mip->ip.i_block[i], buf);

		current_ptr = buf;
		dp = (DIR *)buf;

		while(current_ptr < buf + block_size)
		{
			//child found
			if(strncmp(dp->name, name, dp->name_len) == 0)
			{
				goto found;
			}

			prev_dp = dp;
			current_ptr += dp->rec_len;
			dp = (DIR *)current_ptr;
		}
	}

	//indirect blocks
	if(parent_mip->ip.i_block[INDIRECT_BLOCK_NUMBER] != 0)
	{
		get_block(device, parent_mip->ip.i_block[INDIRECT_BLOCK_NUMBER], buf_ind);
		indirect = (int *)buf_ind;

		for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
		{
			if(indirect[ind_index] == 0)
			{
				continue;
			}
			
			get_block(device, indirect[ind_index], buf);
			current_ptr = buf;
			dp = (DIR *)buf;

			while(current_ptr < buf + block_size)
			{
				//child found
				if(strncmp(dp->name, name, dp->name_len) == 0)
				{
					goto found;
				}

				prev_dp = dp;
				current_ptr += dp->rec_len;
				dp = (DIR *)current_ptr;
			}
		}
	}

	//double indirect blocks
	if(parent_mip->ip.i_block[DOUBLE_INDIRECT_BLOCK_NUMBER] != 0)
	{
		get_block(device, parent_mip->ip.i_block[DOUBLE_INDIRECT_BLOCK_NUMBER], buf_double);
		double_indirect = (int *)buf_double;

		for(double_index = 0; double_index < BLOCK_NUMBERS_PER_BLOCK; double_index++)
		{
			get_block(device, double_indirect[double_index], buf_ind);
			indirect = (int *)buf_ind;
			for(ind_index = 0; ind_index < BLOCK_NUMBERS_PER_BLOCK; ind_index++)
			{
				if(indirect[ind_index] == 0)
				{
					continue;
				}
				
				get_block(device, indirect[ind_index], buf);
				current_ptr = buf;
				dp = (DIR *)buf;

				while(current_ptr < buf + block_size)
				{
					//child found
					if(strncmp(dp->name, name, dp->name_len) == 0)
					{
						goto found;
					}

					prev_dp = dp;
					current_ptr += dp->rec_len;
					dp = (DIR *)current_ptr;
				}
			}
		}
	}


	set_error("File does not exist");
	return -1;

	found:

	if(current_ptr == buf &&
		current_ptr + dp->rec_len == buf + block_size)
	{
		deallocate_block(device, parent_mip->ip.i_block[i]);

		parent_mip->ip.i_size -= block_size;

		while((i + 1) < NUM_DIRECT_BLOCKS)
		{
			if(parent_mip->ip.i_block[i + 1] == 0)
			{
				continue;
			}
			i++;
			get_block(device, parent_mip->ip.i_block[i], buf);
			put_block(device, parent_mip->ip.i_block[i - 1], buf);
		}
	}
	//last entry in block
	else if(current_ptr + dp->rec_len == buf + block_size)
	{
		prev_dp->rec_len += dp->rec_len;
		put_block(device, parent_mip->ip.i_block[i], buf);
	}
	//some middle entry
	else
	{
		last_ptr = current_ptr;
		while(last_ptr + dp->rec_len < buf + block_size)
		{
			last_ptr += dp->rec_len;
			dp = (DIR *)last_ptr;
		}
		dp = (DIR *)current_ptr;
		last_dp = (DIR *)last_ptr;

		last_dp->rec_len += dp->rec_len;

		start = current_ptr + dp->rec_len;
		end = buf + block_size;
		memmove(current_ptr, start, end - start);

		put_block(device, parent_mip->ip.i_block[i], buf);
	}

	parent_mip->dirty = TRUE;

	return 0;

}

//will also clear the data in each block literal
int clear_blocks(MINODE *mip)
{	
	int device = mip->dev;
	int *indirect_block, *double_indirect_block;
	char indirect_buf[BLOCK_SIZE], double_indirect_buf[BLOCK_SIZE];
	int indirect_sentinel, double_indirect_sentinel, total = 0;


	//direct blocks
	for(indirect_sentinel = 0; indirect_sentinel < NUM_DIRECT_BLOCKS; indirect_sentinel++)
	{
		if(mip->ip.i_block[indirect_sentinel] == 0)
		{
			continue;
		}
		deallocate_block(device, mip->ip.i_block[indirect_sentinel]);
		mip->ip.i_block[indirect_sentinel] = 0;
	}

	//indirect blocks
	if(mip->ip.i_block[INDIRECT_BLOCK_NUMBER] != 0)
	{
		get_block(device, mip->ip.i_block[INDIRECT_BLOCK_NUMBER], indirect_buf);
		indirect_block = (int *)indirect_buf;
	
		for(indirect_sentinel = 0; indirect_sentinel < BLOCK_NUMBERS_PER_BLOCK; indirect_sentinel++)
		{
			if(indirect_block[indirect_sentinel] == 0)
			{
				continue;
			}
			deallocate_block(device, indirect_block[indirect_sentinel]);
		}
		deallocate_block(device, mip->ip.i_block[INDIRECT_BLOCK_NUMBER]);
 		mip->ip.i_block[INDIRECT_BLOCK_NUMBER] = 0;
	}

	memset(indirect_buf, 0, BLOCK_SIZE);
	memset(double_indirect_buf, 0, BLOCK_SIZE);
	
	//doubly indirect blocks 
	if(mip->ip.i_block[DOUBLE_INDIRECT_BLOCK_NUMBER] != 0)
	{
		get_block(device, mip->ip.i_block[DOUBLE_INDIRECT_BLOCK_NUMBER], double_indirect_buf);
		double_indirect_block = (int *)double_indirect_buf;

		for(indirect_sentinel = 0; indirect_sentinel < BLOCK_NUMBERS_PER_BLOCK; indirect_sentinel++)
		{
			if(double_indirect_block[indirect_sentinel] == 0)
			{
				continue;
			}
			
			get_block(device, double_indirect_block[indirect_sentinel], indirect_buf);
			indirect_block = (int *)indirect_buf;

			for(double_indirect_sentinel = 0; double_indirect_sentinel < BLOCK_NUMBERS_PER_BLOCK; double_indirect_sentinel++)
			{
				if(indirect_block[double_indirect_sentinel] == 0)
				{
					continue;
				}
				deallocate_block(device, indirect_block[double_indirect_sentinel]);
			}

			memset(indirect_buf, 0, BLOCK_SIZE);

		}
		deallocate_block(device, mip->ip.i_block[DOUBLE_INDIRECT_BLOCK_NUMBER]);
		mip->ip.i_block[DOUBLE_INDIRECT_BLOCK_NUMBER] = 0;
	}

	mip->dirty = TRUE;
	mip->ip.i_size = 0;
	mip->ip.i_ctime = mip->ip.i_mtime = time(0L);  
	mip->ip.i_blocks = 0;

	return 0;
}