#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "util.c"

MINODE minodes[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

char gpath[256]; // holder of component strings in pathname
char *name[64];  // assume at most 64 components in pathnames
int  n;

int  fd, dev;
int  nblocks, ninodes, bmap, imap, iblk, inode_table_block, block_size, inodes_per_block;
char line[256], cmd[32], pathname[256];

char *error_message;

MINODE *iget();

int init()
{
	int i;
	error_message = "";
  	MINODE *mip;
  	PROC   *p;

  	for (i=0; i<NMINODE; i++){
      	mip = &minodes[i];
      	mip->refCount = 0;
      // set all entries to 0;
  	}
  	for (i=0; i<NPROC; i++){
       	p = &proc[i];
      	p->pid = i;
      	p->uid = i;
      	p->cwd = 0;
  	}
  	return 0;
}

int mount_root()
{
	char buf[BLKSIZE];
	int inode_size;
	SUPER *sp;
	GD *gp;


	if(get_block(dev, 1, buf) < 0)
	{
		return -1;
	}

	sp = (SUPER *)buf;

	//printf("%d\n", sp->s_magic);

	if(sp->s_magic != 0xEF53)
	{
		error_message = "NOT an ext2 FS";
		return -1;
	}

	ninodes = sp->s_inodes_count;
	nblocks = sp->s_blocks_count;
	block_size = 1024 << sp->s_log_block_size;
	inode_size = sp->s_inode_size;

	if(get_block(dev, GD_BLOCK, buf) < 0)
	{
		return -1;
	}

	gp = (GD *)buf;

	bmap = gp->bg_block_bitmap;
	imap = gp->bg_inode_bitmap;
	inode_table_block = gp->bg_inode_table;
	inodes_per_block = block_size / inode_size;


	printf("- - - - - - - - BLOCK INFO - - - - - - - -\n");
	printf(" Number of inodes: %d\n", ninodes);
	printf(" Blocks available: %d\n", nblocks);
	printf(" Block size: %d\n", block_size);
	printf(" Inode size: %d\n", inode_size);
	printf(" Bmap start block number: %d\n", bmap);
	printf(" Imap start block number: %d\n", imap);
	printf(" Inode table start block number: %d\n", inode_table_block);
	//printf(" Inodes per block: %d\n", inodes_per_block);
	printf("- - - - - - - - BLOCK INFO - - - - - - - -\n");

	root = get_minode(dev, ROOT_INODE);
	if(!root)
	{
		return -1;
	} 
	return 0;
}
void ls_file(MINODE *mip, char *name)
{
	int mode, links, uid, gid, size;
	char *time;
	static const char* Permissions = "rwxrwxrwx";
	INODE *ip = &mip->ip;

	mode   = ip->i_mode;
    links  = ip->i_links_count;
    uid    = ip->i_uid;
    gid    = ip->i_gid;
    size   = ip->i_size;

    time = ctime((time_t*)&ip->i_mtime);
    
    switch(mode & 0xF000) 
    {
        case 0x8000:  putchar('-');     break; // 0x8 = 1000
        case 0x4000:  putchar('d');     break; // 0x4 = 0100
        case 0xA000:  putchar('l');     break; // oxA = 1010
        default:      putchar('?');     break;
    }

    for(int i = 0; i < strlen(Permissions); i++)
        putchar(mode & (1 << (strlen(Permissions) - 1 - i)) ? Permissions[i] : '-');

    printf("%d %d %d %d %26s %s", links, gid, uid, size, time, name);

    // Trace link
    if(S_ISLNK(ip->i_mode))
        printf(" -> %s", (char*)ip->i_block);


}
void ls_dir(MINODE *mip)
{
	char buf[BLKSIZE], *cp, name[256];
	DIR *dp;
	INODE *ip = &mip->ip;
	MINODE *dir_entry_mip;
	int i;

	for(i = 0; i < (ip->i_size / block_size); i++)
	{
		if (ip->i_block[i] == 0 || i >= NUM_DIRECT_BLOCKS)
            break;

        get_block(dev, ip->i_block[i], buf);

        cp = buf;
        dp = (DIR*)buf;

        while(cp < buf + block_size)
        {
        	if(((strncmp(dp->name, ".", dp->name_len)) != 0) && ((strncmp(dp->name, "..", dp->name_len)) != 0))
        	{
        		strncpy(name, dp->name, dp->name_len);
        		name[dp->name_len] = 0;

        		dir_entry_mip = get_minode(dev, dp->inode);
        		ls_file(dir_entry_mip, name);

        		printf("\n");
        	}        	   
        	cp += dp->rec_len;
        	dp = (DIR*)cp;
        }
	}
}


void ls(char *pathname)
{
	int ino;
	MINODE *mip;

	if(pathname[0] == '\0')
	{
		mip = running->cwd;
	}
	else
	{
		ino = get_inode_number(dev, pathname);

		if(ino <= 0)
		{
			printf("%s\n", error_message);
			return;
		}

		mip = get_minode(dev, ino);
	}


	if(S_ISDIR(mip->ip.i_mode))
		ls_dir(mip);
	else
		ls_file(mip, pathname);

}


void cd(char *pathname)
{
	int ino;
	MINODE *mip;
	INODE *pip;
	if(pathname[0] == '\0')
		running->cwd = root;
	else
	{
		ino = get_inode_number(dev, pathname);
		mip = get_minode(dev, ino);

		if(mip == 0)
		{
			printf("%s\n", error_message);
			return;
		}

		running->cwd = mip;
	}
}

void find_my_name(MINODE *parent, int myino, char** myname)
{
	int i, device;
	INODE *ip;
	char buf[BLKSIZE];
	DIR *dp;
	char *cp;

	device = parent->dev;
	ip = &parent->ip;

	for(i = 0; i < (ip->i_size / block_size); i++)
	{
        if (ip->i_block[i] == 0 || i >= NUM_DIRECT_BLOCKS)
            break;

        get_block(dev, ip->i_block[i], buf);

        cp = buf;
        dp = (DIR*)buf;

        while(cp < (buf + block_size))
        {
        	if(dp->inode == myino)
        	{
        		*myname = (char*)malloc((dp->name_len + 1) * sizeof(char));
              	strncpy(*myname, dp->name, dp->name_len);
        		(*myname)[dp->name_len] = 0;
        	}
        	cp += dp->rec_len;
        	dp = (DIR *)cp;
        }
	}
}

void rpwd(MINODE *mip)
{
	int i, my_ino, parent_ino;
	DIR *dp;
	MINODE *parent_mip;
	char *myname, buf[BLKSIZE], *cp;
	if(mip == root) return;

	get_block(dev, mip->ip.i_block[0], buf);

	cp = buf;
	dp = (DIR*)buf;

	my_ino = dp->inode;

	cp += dp->rec_len;
	dp = (DIR*)cp;

	parent_ino = dp->inode;

	parent_mip = get_minode(dev, parent_ino);

	if(my_ino <= 0 || parent_ino <= 0 || parent_mip == 0)
	{
		printf("%s\n", error_message);
		return;
	}

	find_my_name(parent_mip, my_ino, &myname);

	rpwd(parent_mip);

	printf("/%s", myname);

	free(myname);


	//printf("")

}

void pwd(MINODE *wd)
{
	if(wd == root)
		printf("/");
	else
		rpwd(wd);
	printf("\n");
}

void quit()
{
	exit(0);
}

char *disk = "mydisk";
int main(int argc, char *argv[])
{
	int ino, fd; 
	char buf[BLKSIZE];

	if (argc > 1)
		disk = argv[1];

	fd = open(disk, O_RDWR);
	if(fd < 0)
	{
		printf("open %s failed\n", disk);
		return -1;
	}

	dev = fd;



	init();
	if(mount_root() < 0)
	{
		printf("%s\n", error_message);
		return -1;
	}
	printf(" Mounting %s\n", disk);
	printf("\n Mount root success\n");

	running = &proc[0];
	if((running->cwd = get_minode(dev, ROOT_INODE)) == NULL)
	{
		printf("%s\n", error_message);
		return -1;
	}

	printf(" root refCount = %d\n", root->refCount);

	while(1)
	{
		printf("\n Input command: [ls|cd|pwd|quit] ");
		fgets(line, 256, stdin);

		line[strlen(line) - 1] = '\0';

		if(line[0] == '\0')
			continue;

		pathname[0] = 0;
		cmd[0] = 0;

		sscanf(line, "%s %s", cmd, pathname);

		//printf("cmd=%s pathname=%s\n", cmd, pathname);
		printf("\n");
    	if (strcmp(cmd, "ls")==0)
       		ls(pathname);

    	if (strcmp(cmd, "cd")==0)
       		cd(pathname);

    	if (strcmp(cmd, "pwd")==0)
       		pwd(running->cwd);

    	if (strcmp(cmd, "quit")==0)
       		quit();
	}

	return 0;
}
