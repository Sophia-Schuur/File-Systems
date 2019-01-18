#include "client_utils.h"
#include "client_utils.c"

struct sockaddr_in server_addr;
int sock, result, nbytes;
char input[MAX_BUF_SIZE], server_response[MAX_BUF_SIZE];
char cmd[32], pathname[224];


int main(int argc, char *argv[])
{
	if(init_client() < 0)
		exit(1);

	while(1)
	{

		server_response[0] = 0;	
		input[0] = 0;
		cmd[0] = 0;
		pathname[0] = 0;


		do
		{
			printf("input %c : ", '%');
			fgets(input, MAX_BUF_SIZE, stdin);
		} while(input[0] == '\n');

		nbytes = write(sock, input, MAX_BUF_SIZE);

		sscanf(input, "%s %s", cmd, pathname);

		if(strcmp(cmd, "help") == 0)
		{
			client_help();
		}

		if(strcmp(cmd, "ls") == 0)
		{
			client_ls();
		}

		if(strcmp(cmd, "get") == 0)
		{
			client_get(pathname);
		}

		if(strcmp(cmd, "put") == 0)
		{
			client_put(pathname);
		}

		nbytes = read(sock, server_response, MAX_BUF_SIZE);
		printf("%s\n", server_response);
	}
}