#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <errno.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;   

#define BLKSIZE  1024

#define SUPER_BLOCK  1
#define GD_BLOCK 2
#define ROOT_INODE 2

#define FALSE 0;
#define TRUE 1;

#define NUM_DIRECT_BLOCKS 12

#define INITIAL_BUF_SIZE 8

#define NMINODE    64
#define NOFT       64
#define NFD        10
#define NMOUNT      4
#define NPROC       2

typedef struct minode{
  INODE ip;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mntable *mptr;
}MINODE;

typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;
