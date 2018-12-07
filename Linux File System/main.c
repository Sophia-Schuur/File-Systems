#include <stdio.h>
#include <fcntl.h>

#include "utils/input.h"
#include "utils/parse.h"

#include "cmd.h"

PROC   proc[NPROC], *running;
MINODE minodes[NMINODE], *root;
OFT oft[NOFT];

int block_bitmap, inode_bitmap, inode_table_block;
int nblocks, ninodes, block_size, inode_size, inodes_per_block;

SUPER *sp;
GD    *gp;

int thrown_error;
char *error_message;

char *disk;
int main(int argc, char *argv[])
{
	char *input_line, **my_argv;
	int my_argc, fd, dev;


	int (*fptr)(int, char**) = NULL;

	if(argc > 1)
	{
		disk = argv[1];
	}
	else 
	{
		disk = "disk";
	}

	fd = open(disk, O_RDWR);
	if(fd < 0)
	{
		printf("%s\n", strerror(errno));
		exit(1);
	}

	dev = fd;

	mount_root(dev);

	if(thrown_error == TRUE)
	{
		printf("%s\n", error_message);
		exit(1);
	}

	while(TRUE)
	{
		do
		{
			printf("js_ext2fs %c : ", '%');
			if(get_input(&input_line) < 0)
			{
				print_error();
				exit(1);
			}

		} while(input_line == NULL);
		
		my_argc = parse(input_line, " ", &my_argv);

		if(my_argc < 0)
		{
			print_error();
			continue;
		}

		if(strcmp(my_argv[0], "sudo") == 0)
		{
			running = &proc[SUPER_USER];

			for(int i = 0; i < my_argc; i++)
			{
				my_argv[i] = my_argv[i + 1];
			}
			my_argc--;
			my_argv[my_argc] = 0;
		}

		fptr = get_cmd(my_argv[0]);

		fptr(my_argc, my_argv);

		if(thrown_error == TRUE)
		{
			print_error();
		}

		free(input_line);
		free_array(my_argv);
	}
}

/*-----------------------------------------
Function: 
Use: 
Throws errors?: 
-----------------------------------------*/
