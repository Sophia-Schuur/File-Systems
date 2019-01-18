#include "server_utils.h"
#include "server_utils.c"


struct sockaddr_in server_addr, client_addr;
char input[MAX_BUF_SIZE], response[MAX_BUF_SIZE];
char cmd[32], pathname[224];

int server_socket, client_socket;
int result, length, nbytes;

int main(int argc, char *argv[])
{
	if(init_server() < 0)
	{
		exit(1);
	}


	//wait and listen for a new connection
	while(1)
	{
		if(accept_new_connection() < 0)
			exit(1);

		response[0] = 0;	
		input[0] = 0;
		cmd[0] = 0;
		pathname[0] = 0;


		while(1)
		{

			response[0] = 0;	
			input[0] = 0;
			cmd[0] = 0;
			pathname[0] = 0;

			nbytes = read(client_socket, input, MAX_BUF_SIZE);
			if(nbytes == 0)
			{
				printf("%s\n", "server: client died\n");
				break;
			}

			sscanf(input, "%s %s", cmd, pathname);

			printf("cmd: %s path: %s\n", cmd, pathname);

			if(strcmp(cmd, "mkdir") == 0)
			{
				server_mkdir();
			}

			if(strcmp(cmd, "rmdir") == 0)
			{
				server_rmdir();
			}

			if(strcmp(cmd, "rm") == 0)
			{
				server_rm();
			}

			if(strcmp(cmd, "cd") == 0)
			{
				server_cd();
			}

			if(strcmp(cmd, "pwd") == 0)
			{
				server_pwd();
			}

			if(strcmp(cmd, "ls") == 0)
			{
				server_ls();
				response[0] = 0;
			}

			if(strcmp(cmd, "get") == 0)
			{
				server_get();
			}

			if(strcmp(cmd, "put") == 0)
			{
				server_put();
			}

			nbytes = write(client_socket, response, MAX_BUF_SIZE);
			
		}
	}
}