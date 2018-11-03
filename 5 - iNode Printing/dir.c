#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>

#define BLKSIZE 1024
#define SUPER_OFFSET 1024
#define ROOT_INODE_NUMBER 2

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
DIR   *dp; 

int LEN = 255;
char search_dir_name[128];
int fd;
int iblock;
int block_size;
char buf[BLKSIZE];

char *blk;
char *blkcpy;

void get_block(int fd, int blk, char buf[ ])
{
  lseek(fd,(long)blk*BLKSIZE, 0);
   read(fd, buf, BLKSIZE);
}

int search(INODE *ip, char *name)
{
  char sbuf[BLKSIZE];
  char *sblk, *sblkcpy;
  DIR *dir;

  for(int i = 0; i < (ip->i_size / block_size); i++)
  {
    get_block(fd, ip->i_block[i], sbuf);
    sblk = sbuf;
    sblkcpy = sblk;

    dir = (DIR *)sblk;

    while (sblkcpy < (sblk + block_size))
        {
            if(strncmp(name, dir->name, strlen(name)) == 0)
            {
              return dir->inode;
            }
            sblkcpy += dir->rec_len;       // advance cp by rec_len BYTEs
            dir = (DIR*)sblkcpy;           // pull dp along to the next record
        } 
  }
  printf("NOT FOUND. ");
  return 0;
}

void dir()
{
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  if (sp->s_magic != 0xEF53)
  {
      printf("NOT an EXT2 FS\n");
      exit(1);
    }

  block_size = 1024 << sp->s_log_block_size;
  get_block(fd, 2, buf);
  gp = (GD *)buf;


  
  int block_group = 0, local_index = 1;

  int block = (block_group * sp->s_blocks_per_group) + gp->bg_inode_table;

  char *inodebuf;

  get_block(fd, block, inodebuf);

  INODE *inode_table = (INODE *)inodebuf; 
  INODE ip = inode_table[local_index];



  for(int i = 0; i < (ip.i_size / block_size); i++)
  {
    get_block(fd, ip.i_block[i], buf);

    blk = buf;
    blkcpy = blk;
    dp = (DIR*)blk;

        printf("\n inode  rec_len  name_len  name\n\n");
       
        while (blkcpy < (blk + block_size))
        {
            printf(" %3u    %5u      %3u     %s\n", 
                    dp->inode, dp->rec_len, dp->name_len, dp->name);
            blkcpy += dp->rec_len;       // advance cp by rec_len BYTEs
            dp = (DIR*)blkcpy;           // pull dp along to the next record
        } 

  }

  printf("\nEnter a directory to search for: ");
  fgets(search_dir_name, 128, stdin);
  search_dir_name[strlen(search_dir_name) - 1] = 0;

  printf("inode number of %s: %d\n", search_dir_name, search(&ip, search_dir_name));
}

char *disk = "mydisk";

int main(int argc, char* argv[])
{ 
    if (argc > 1)
      disk = argv[1];

  fd = open(disk, O_RDONLY);
    if (fd < 0){
      printf("open %s failed\n", disk);
      exit(1);
    }
  
    dir();
}