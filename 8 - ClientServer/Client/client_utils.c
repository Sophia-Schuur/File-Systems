#include "client_utils.h"

int init_client()
{
	printf("- - - - - - - - - - - - - - INIT CLIENT - - - - - - - - - - - - - -\n"); 

	printf("[1]: create a TCP socket\n");

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock < 0)
	{
		printf("[ERROR]: socket call failed\n");
		return -1;
	}

	printf("[2]: fill server_addr struct with server's IP and PORT NUMBER\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(SERVER_PORT); 

	printf("[3]: connecting to server ....\n");

	result = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(result < 0)
	{
		printf("connection failed\n");
		printf("Error: %s: \n", strerror(errno));
		return -1;
	}

	printf("[4]: connected OK to\n"); 
	printf("---------------------------------------\n");
	printf("Server hostname: %s PORT: %d\n", SERVER_HOST, SERVER_PORT);
	printf("---------------------------------------\n");
	printf("- - - - - - - - - - - - - INIT CLIENT DONE - - - - - - - - - - - - -\n");

	return 0;
}

int client_help()
{

}

int client_ls()
{
	nbytes = read(sock, server_response, MAX_BUF_SIZE);

	while((strcmp(server_response, ENDOFSTREAM) != 0))
	{
		printf("%s\n", server_response);
		server_response[0] = 0;
		nbytes = read(sock, server_response, MAX_BUF_SIZE);
	}
}

int client_get(char *pathname)
{
	nbytes = read(sock, server_response, MAX_BUF_SIZE);
	FILE *fp;

	if(strcmp(server_response, FILEOK) != 0)
	{
		return -1;
	}

	fp = fopen(pathname, "wb+");
	if(fp == NULL)
	{
		printf("%s\n", strerror(errno));
		return -1;
	}

	nbytes = read(sock, server_response, MAX_BUF_SIZE);
	

	while((strcmp(server_response, ENDOFSTREAM)) != NULL)
	{
		fprintf(fp, server_response);
		nbytes = read(sock, server_response, MAX_BUF_SIZE);
	}
	fclose(fp);
	return 0;

}

int client_put(char *pathname)
{
	FILE *fp;
	char buf[MAX_BUF_SIZE];

	fp = fopen(pathname, "rb");

	if(fp == NULL)
	{
		strcpy(buf, strerror(errno));
		printf("%s\n", buf);
		nbytes = write(sock, buf, MAX_BUF_SIZE);
		return -1;
	}

	//let server know file is ok
	nbytes = write(sock, FILEOK, MAX_BUF_SIZE);

	while(fgets(buf, MAX_BUF_SIZE, fp))
	{
		nbytes = write(sock, buf, MAX_BUF_SIZE);
		buf[0] = 0;
	}
	nbytes = write(sock, ENDOFSTREAM, MAX_BUF_SIZE);

	return 0;
}