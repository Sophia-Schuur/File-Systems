#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>

#include <time.h>

#include <fcntl.h>
#include <sys/stat.h>


#define MAX_BUF_SIZE 256
#define SERVER_HOST  "localhost"
#define SERVER_PORT  1234
#define QUEUE_LENGTH 5

#define ENDOFSTREAM "-----ENDOFSTREAM-----"
#define FILEOK "-----FILEOK-----"

extern struct sockaddr_in server_addr, client_addr;
extern char line[MAX_BUF_SIZE], buf[MAX_BUF_SIZE], response[MAX_BUF_SIZE];
extern char cmd[32], pathname[224];
extern int server_socket, client_socket;
extern int result, length, nbytes;

int init_server();

int accept_new_connection();

int server_mkdir();

int server_rmdir();

int server_rm();

int server_cd();

int server_pwd();

int server_ls();

int server_get();

int server_put();

void filestat_stringify(struct stat *fs, char *filename, char *buf);
#endif