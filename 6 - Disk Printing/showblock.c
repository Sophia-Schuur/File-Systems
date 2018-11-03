#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>

typedef unsigned int u32;

//Define shorter TYPES, save typing efforts
typedef struct ext2_group_desc GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR; //Need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

#define BLKSIZE 1024
#define BLOCK_OFFSET(block) (1024 + (block - 1) * 1024)

char buf[BLKSIZE];
int fd;
int imap, bmap;
int ninodes, nblocks, nfreeInodes, nfreeBlocks;
char *device;
int iblock;

void get_block(int fd, int blk, char buf[BLKSIZE])
{
	lseek(fd, (long)(blk*BLKSIZE), 0);
	read(fd, buf, BLKSIZE);
}

static void get_inode(int fd, int ino, INODE *inode)
{
	lseek(fd, BLOCK_OFFSET(iblock) + (ino - 1) * sizeof(INODE), SEEK_SET);
	read(fd, inode, sizeof(INODE));
}



int dirSearch(char name[1024][1024], int ind, INODE *inoptr)
{
	char dirname[256];
	int i = 0;
	char *cp;
	int temp = 0;

								

	printf("\n-------- Dir Search --------\n");
	printf("Searching for %s\n", name[ind]);
	while (inoptr->i_block[i] != 0)
	{
																
		get_block(fd, inoptr->i_block[i], buf);
		dp = (DIR *)buf;
		cp = buf;

		printf("\n inode  rec_len  name_len  name\n\n");
       
		while (cp < buf + BLKSIZE)
		{
			strncpy(dirname, dp->name, dp->name_len);
			dirname[dp->name_len] = 0;
			
printf(" %3u    %5u      %3u     %s\n", 
                    dp->inode, dp->rec_len, dp->name_len, dp->name);		

			if (!strcmp(name[ind], dirname))
			{
				
				return dp->inode;
			}
																						
			cp += dp->rec_len;
			dp = (DIR *)cp;
																								
		}
		i++;
	}
	printf("[!] ERROR: %s Not Found\n", name[ind]);
	return 0;
}

int fullSearch(char name [1024][1024], int num_paths)
{
	int i = 0;
	int mail = 0;
	int ino = 0;
	char *cp;
	INODE dirnode;

	get_inode(fd, 2, &dirnode);
	for (i = 0; i < num_paths; i++)
	{

		if (i < num_paths)
		{
			if(!S_ISDIR(dirnode.i_mode))
			{
				printf("[!] ERROR: Cannot search through file\n");
				exit(0);
			}
		}

		ino = dirSearch(name, i, &dirnode);
		if (ino == 0)//file not found. exit
		{
			exit(0);
		}

		printf("\n[!] FOUND: %s inode: #%d\n", name[i], ino);
		get_inode(fd, ino, &dirnode);
	}
	return ino;
}

mailMan(int ino)
{
	int blocknum = 0;
	int inodenum = 0;

	blocknum = (ino-1)/8 + ip->i_block[0] - 1;
	printf("blocknum = %d\n", blocknum);
								
	return blocknum;
}

void printStuff(int ino, char name[1024][1024], int num)
{
	int i, j, cycle_blocks, num_blocks, indirect[256], double_indirect[256];
	INODE file;
	SUPER super;
	int blk_size = 1024;

	lseek(fd, iblock, SEEK_SET);
	read(fd, &super, sizeof(super));

	get_inode(fd, ino, &file);
	num_blocks = file.i_size / BLKSIZE;
	cycle_blocks = num_blocks;
	
	printf("size: %u\n", file.i_size);
	printf("blocks: %u\n", num_blocks);
	printf("gid: %d\n", file.i_gid);
	printf("links count: %d\n", file.i_links_count);
	printf("flags: %d\n", file.i_flags);
	printf("blk = %d\n", blk_size);

	printf("-------- DISK BLOCKS --------\n");
	for (i = 0; i < 14; i++)
	{
		printf("block[%d]: %d\n", i, file.i_block[i]);
	}

	printf("\n-------- DIRECT BLOCKS --------\n");
	if (cycle_blocks > 12)
	{
		cycle_blocks = 12;
	}

	printBlocks(cycle_blocks, file.i_block);
	num_blocks -= cycle_blocks;
	printf("\nBlocks Remaining: %u\n", num_blocks);

	// if (num_blocks > 0)
	// {
		printf("-------- INDIRECT BLOCKS --------\n");
		cycle_blocks = num_blocks;
		if (cycle_blocks > 256)
		{
			cycle_blocks = 256;
		}
		get_block(fd, file.i_block[12], indirect);
		printBlocks(cycle_blocks, indirect);
		num_blocks -= cycle_blocks;
		printf("\nBlocks Remaining: %u\n", num_blocks);

		// if (num_blocks > 0)
		// {
			printf("-------- DOUBLE INDIRECT BLOCKS --------\n");
			get_block(fd, file.i_block[13], double_indirect);
			for (j = 0; j < 256; j++)
			{
				if (double_indirect[j] == 0)
				{
					break;
				}

				printf("-------- %d --------\n", double_indirect[j]);
				cycle_blocks = num_blocks;

				if (cycle_blocks > 256)
				{
					cycle_blocks = 256;
				}

				get_block(fd, double_indirect[j], indirect);
				printBlocks(cycle_blocks, indirect);
				num_blocks -= cycle_blocks;
				printf("\nBlocks Remaining: %u\n", num_blocks);
			}
		//}
	//}

}


void printBlocks(int cycle_blocks, int indirect[256])
{
	int i;
	for (i = 0; i < cycle_blocks; i++)
	{
		printf("%d ", indirect[i]);
		if ((i + 1) % 16 == 0)
		{
			printf("\n");
		}
	}
}

int main(int argc, char *argv[])
{
	int i, j, ino;
	char buf[BLKSIZE];
	char path[2048];
	char *temp;
								
	char name[1024][1024];
	char *cp;

								
	if(argc > 2)
	{
		device = argv[1];
		strcpy(path, argv[2]);
	}
	else
	{
		printf("[!] ERROR: Too few arguments. Are you missing a search token?\n");
		exit(0);
	}

	//parse command					
	i = 0;
	j = 0;
	strcat(path, "/");
	temp = strtok(path, "/");
	while(temp != NULL)
	{
		strcpy(name[i], temp);
		temp = strtok(NULL, "/");
		i++;
		j++;
	}

	fd = open(device, O_RDONLY);
	if(fd < 0)
	{
		printf("Open %s failed\n", device);
		exit(1);
	}

						
	get_block(fd, 2, buf);
	gp = (GD *)buf; 
	iblock = gp->bg_inode_table; 
	printf("inode_block: %d\n", iblock);

								
	get_block(fd, iblock, buf); 

	//point inode to root						
	ip = (INODE *)buf + 1; 	
	mailMan(ino);

	ino = fullSearch(name, j);
						
	printStuff(ino, name, j);
	return 0;
}