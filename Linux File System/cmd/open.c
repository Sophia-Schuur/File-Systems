#define _DEFAULT_SOURCE 

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "../utils/input.h"
#include "../utils/search.h"
#include "../utils/readwrite.h"
#include "../utils/file.h"


extern PROC *running;

int js_open(int argc, char *argv[])
{
	char *mode_input;
	int mode, device, ino, dev = running->cwd->dev, i, fd;
	MINODE *mip;
	OFT *oftp;

	if(argc < 3)
	{
		set_error("open <file> <mode> [R|W|RW|AP]");
		return -1;
	}
	mode_input = argv[2];


	if(strncmp(mode_input, "R", strlen(mode_input)) == 0)
	{
		mode = READ;
	}
	else if(strncmp(mode_input, "W", strlen(mode_input)) == 0)
	{
		mode = WRITE;
	}
	else if(strncmp(mode_input, "RW", strlen(mode_input)) == 0)
	{
		mode = READ_WRITE;
	}
	else if(strncmp(mode_input, "AP", strlen(mode_input)) == 0)
	{
		mode = APPEND;
	}
	else
	{
		set_error("Invalid mode");
		free(mode_input);
		return -1;
	}

	ino = get_inode_number(argv[1]);
	if(ino < 1)
	{
		set_error("File does not exist");
		return -1;
	}
	mip = get_minode(dev, ino);

	if(!S_ISREG(mip->ip.i_mode))
	{
		set_error("File is not regular");
		return -1;
	}

	fd = open_file(mip, mode);
	if(thrown_error == TRUE)
	{
		put_minode(mip);
		return -1;
	}
	return fd;
}