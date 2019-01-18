#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#define MAX_BUF_SIZE 256
#define SERVER_HOST  "localhost"
#define SERVER_PORT  1234

#define ENDOFSTREAM  "-----ENDOFSTREAM-----"
#define FILEOK "-----FILEOK-----"

extern struct sockaddr_in server_addr;
extern int sock, result, nbytes;
extern char input[MAX_BUF_SIZE], server_response[MAX_BUF_SIZE]; 

int init_client();

int client_help();

int client_ls();

int client_get(char *pathname);

int client_put(char *pathname);
	

#endif
