#ifndef _TYPE_H_
#define _TYPE_H_


#include <ext2fs/ext2_fs.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

#define FREE 0
#define READY 1

#define FALSE 0
#define TRUE 1

#define SUPER_USER 0

#define BLOCK_SIZE 1024
#define NMINODE    64
#define NFD        16
#define NMOUNT      4
#define NPROC       2

#define SUPER_BLOCK_OFFSET 1
#define GD_BLOCK_OFFSET 2

#define NUM_DIRECT_BLOCKS 12
#define INDIRECT_BLOCK_NUMBER 13
#define DOUBLE_INDIRECT_BLOCK_NUMBER 14
#define I_BLOCKS 15

#define BLOCK_NUMBERS_PER_BLOCK 256


#define ROOT_INODE 2

#define INITIAL_BUF_SIZE 8

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
  int          ppid;
  int          status;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

#endif