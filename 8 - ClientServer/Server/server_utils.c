#include "server_utils.h"

int init_server()
{
	printf("- - - - - - - - - - - - - - INIT SERVER - - - - - - - - - - - - - -\n"); 

	printf("[1]: create a TCP stream socket\n");

	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(server_socket < 0)
	{
		printf("[ERROR]: socket call failed\n");
		return -1;
	}  

	printf("[2]: fill server_addr struct with host IP and PORT NUMBER information\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT); 

	printf("[3]: bind the socket to server address\n");


	result = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int));

	if(result < 0)
	{
		printf("[ERROR]: setsockopt failed\n");
		return -1;
	}

	result = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

	if(result < 0)
	{
		printf("[ERROR]: bind failed\n");
		return -1;
	}

	printf("Hostname: %s Port: %d\n", SERVER_HOST, SERVER_PORT);
	printf("\nserver is listening...\n");

	listen(server_socket, QUEUE_LENGTH); 

	printf("- - - - - - - - - - - - - INIT SERVER DONE - - - - - - - - - - - - -\n");
	return 0;
}

int accept_new_connection()
{
	printf("server: accepting new connection\n");

	length = sizeof(client_addr);
	client_socket = accept(server_socket, (struct sockadd *)&client_addr, &length);


	
	if(client_socket < 0)
	{
		printf("server accept error\n");
		return -1;
	}
	
	printf("server: accepted a client connection from\n");
	printf("-----------------------------------------------\n");
	printf("Client: IP = %s  port= %d \n",inet_ntoa(client_addr.sin_addr),                                       
		ntohs(client_addr.sin_port));
	printf("-----------------------------------------------\n");		
}

int server_mkdir()
{
	result = mkdir(pathname, 0755);
	if(result < 0)
	{
		strcpy(response, strerror(errno));
	}
	else
	{
		strcpy(response, "mkdir ok");
	}
}

int server_rmdir()
{
	result = rmdir(pathname);
	if(result < 0)
	{
		strcpy(response, strerror(errno));
	}
	else
	{
		strcpy(response, "rmdir ok");
	}
}

int server_rm()
{
	result = unlink(pathname);
	if(result < 0)
	{
		strcpy(response, strerror(errno));
	}
	else
	{
		strcpy(response, "rm ok");
	}
}

int server_cd()
{
	result = chdir(pathname);
	if(result < 0)
	{
		strcpy(response, strerror(errno));
	}
	else
	{
		strcpy(response, "cd ok");
	}
}

int server_pwd()
{
	char buf[MAX_BUF_SIZE];
	getcwd(buf, MAX_BUF_SIZE);

	if(buf == NULL)
	{
		strcpy(response, strerror(errno));
	}
	else
	{
		strcpy(response, buf);
	}
}

int server_ls()
{

	//set path to cwd if no arg for ls
	char path[MAX_BUF_SIZE];
	struct stat fs;
	char buf[MAX_BUF_SIZE];

	if(pathname[0] == 0)
	{
		getcwd(path, MAX_BUF_SIZE);
		if(path == NULL)
		{
			strcpy(response, strerror(errno));
			return -1;
		}
	}
	else
	{
		strcpy(path, pathname);
	}

	if(lstat(path, &fs) < 0)
	{
		strcpy(response, strerror(errno));
		return -1;
	}

	if(S_ISDIR(fs.st_mode))
	{
		server_ls_dir(path);
	}
	else
	{
		filestat_stringify(&fs, basename(path), buf);
		nbytes = write(client_socket, buf, MAX_BUF_SIZE);
	}

	nbytes = write(client_socket, ENDOFSTREAM, MAX_BUF_SIZE);
	return 0;
}

int server_ls_dir(char *pathname)
{
	DIR *src;
	struct dirent *dp;
	struct stat fs;
	char path[MAX_BUF_SIZE], buf[MAX_BUF_SIZE];

	if(!(src = opendir(pathname)))
	{
		strcat(response, strerror(errno));
		printf("%s\n", response);
		return -1;
	}

	while((dp = readdir(src)))
	{
		if(strncmp(dp->d_name, ".", strlen(".")) != 0 &&
			strncmp(dp->d_name, "..", strlen("..")) != 0)
		{
			strcpy(path, pathname);
			strcat(path, "/");
			strcat(path, dp->d_name);

			if(stat(path, &fs) < 0)
			{
				strcpy(response, strerror(errno));
				printf("%s\n", response);
				return -1;				
			}

			filestat_stringify(&fs, dp->d_name, buf);

			nbytes = write(client_socket, buf, MAX_BUF_SIZE);
		}
	}
}

int server_get()
{
	FILE *fp;
	char buf[MAX_BUF_SIZE];

	fp = fopen(pathname, "rb");

	if(fp == NULL)
	{
		strcpy(response, strerror(errno));
		printf("%s\n", response);
		return -1;
	}

	//let client know file is ok
	nbytes = write(client_socket, FILEOK, MAX_BUF_SIZE);

	while(fgets(buf, MAX_BUF_SIZE, fp))
	{
		nbytes = write(client_socket, buf, MAX_BUF_SIZE);
		buf[0] = 0;
	}
	nbytes = write(client_socket, ENDOFSTREAM, MAX_BUF_SIZE);
	
	strcpy(response, "get file ok\n");
	return 0;
}

int server_put()
{
	char writebuf[256];

	nbytes = read(client_socket, writebuf, MAX_BUF_SIZE);
	FILE *fp;

	if(strcmp(writebuf, FILEOK) != 0)
	{
		return -1;
	}

	fp = fopen(pathname, "wb+");
	if(fp == NULL)
	{
		strcpy(response, strerror(errno));
		return -1;
	}

	nbytes = read(client_socket, writebuf, MAX_BUF_SIZE);
	

	while((strcmp(writebuf, ENDOFSTREAM)) != NULL)
	{
		fprintf(fp, writebuf);
		nbytes = read(client_socket, writebuf, MAX_BUF_SIZE);
	}
	fclose(fp);

	strcpy(response, "put file ok\n");
	return 0;

}

void filestat_stringify(struct stat *fs, char *filename, char *buf)
{
	char int_box[MAX_BUF_SIZE];
	struct tm lt;

	//permissions
	strcpy(buf, (S_ISDIR(fs->st_mode)) ? "d" : "-");
    strcat(buf, (fs->st_mode & S_IRUSR) ? "r" : "-");
    strcat(buf, (fs->st_mode & S_IWUSR) ? "w" : "-");
    strcat(buf, (fs->st_mode & S_IXUSR) ? "x" : "-");
    strcat(buf, (fs->st_mode & S_IRGRP) ? "r" : "-");
    strcat(buf, (fs->st_mode & S_IWGRP) ? "w" : "-");
    strcat(buf, (fs->st_mode & S_IXGRP) ? "x" : "-");
    strcat(buf, (fs->st_mode & S_IROTH) ? "r" : "-");
    strcat(buf, (fs->st_mode & S_IWOTH) ? "w" : "-");
   	strcat(buf, (fs->st_mode & S_IXOTH) ? "x" : "-");

   	//nifty space
   	strcat(buf, " ");

   	//link;
   	sprintf(int_box, "%d", fs->st_nlink);	
   	strcat(buf, int_box);
   	int_box[0] = 0;

   	//nifty space
   	strcat(buf, " ");

   	//nifty space
   	strcat(buf, " ");

   	//gid;
   	sprintf(int_box, "%d", fs->st_gid);	
   	strcat(buf, int_box);
   	int_box[0] = 0;

   	//nifty space
   	strcat(buf, " ");

   	//uid;
   	sprintf(int_box, "%d", fs->st_uid);	
   	strcat(buf, int_box);
   	int_box[0] = 0;

   	//nifty space
   	strcat(buf, " ");

   	//size;
   	sprintf(int_box, "%d", fs->st_size);	
   	strcat(buf, int_box);
   	int_box[0] = 0;

   	//nifty space
   	strcat(buf, " ");

   	//date;
   	localtime_r(&fs->st_mtime, &lt);
   	strftime(int_box, MAX_BUF_SIZE, "%b %d %y %H:%M", &lt);
   	strcat(buf, int_box);

   	//nifty space
   	strcat(buf, " ");

   	//name
   	strcat(buf, filename);


   	return buf;

}