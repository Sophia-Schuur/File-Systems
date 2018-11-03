#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#pragma once
#pragma warning(disable : 4996)


//NODE STRUCT//
typedef struct node
{
	char name[64];
	char type;
	struct node *childPtr, *siblingPtr, *parentPtr;

}Node;

///////////////

//GLOBAL VARIABLES//
Node *root = NULL, *cwd = NULL, /**curr = NULL,*/ *start = NULL;
char line[128];                   // for gettting user input line
char command[16], pathname[64];   // for command and pathname strings
char dname[64], bname[64];        // for dirname and basename 
char *name[100];                  // token string pointers 
int  n;                           // number of token strings in pathname 
FILE *fp;
int run = 1;

//Command Table/////////////////0       1		2	   3	4	   5		6	  7        8	   9		10   11
extern char *cmdTable[] = { "menu", "mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "quit", 0 };
//int(*fptr[])(char *) = { (int(*)())mkdir, rmdir, ls, cd, pwd, creat, rm, reload, save, quit};
/////////////////


//return the command table index (see above table)
int findCmd(char *command)
{
	int i = 0;
	while (cmdTable[i]) {
		if (!strcmp(command, cmdTable[i]))
		{
			return i; // found command: return index i
		}

		i++;
	}
	return -1; //not found: return -1
}


//Clear global variables, set pointers to null,  set the current working directory to root
void initialize()
{
	strcpy(line, "");
	strcpy(command, "");
	strcpy(pathname, "");

	root = (struct node *)malloc(sizeof(struct node));
	//root->siblingPtr, root->parentPtr = root;
	root->childPtr = NULL;
	root->siblingPtr = NULL;
	root->parentPtr = NULL;
	strcpy(root->name, "root");
	root->type = 'D';

	cwd = root;
}


//Display command specification
void displayMenu()
{
	printf("Commands available:\n");
	printf("mkdir pathname:		make a new directory\n");
	printf("rmdir pathname:		remove directory if empty\n");
	printf("cd [pathname]:		change CWD to pathname, or to / if no pathname\n");
	printf("ls [pathname]:	 	list directory contents (pathname or CWD)\n");
	printf("pwd:			print (absolute) pathname of CWD\n");
	printf("creat pathname:		create a FILE node\n");
	printf("rm pathname:		remove a FILE node\n");
	printf("save filename:	 	save the current file system tree in a file\n");
	printf("reload filename:	construct file system tree from a file\n");
	printf("quit:			save the file system tree, then terminate\n\n");
}

//sets dirname and basename of given pathname
void PathnameToDirBasename()
{
	char *pathString = strtok(pathname, "/");
	char *pathLast;
	while (pathString)
	{
		pathLast = pathString;
		pathString = strtok(NULL, "/");

		if (pathString)
		{
			strcat(dname, pathLast);
			strcat(dname, "/");
		}
		else
		{
			strcpy(bname, pathLast);
		}
	}

	//printf("Dirname: %s | Basename: %s\n", dname, bname);
}

//allocate space for a new directory type node
Node * NewNode(char *checkFileType)
{
	Node *NewNode = (struct node *)malloc(sizeof(struct node));
	strcpy(NewNode->name, bname);
	if (checkFileType == 'D')
	{
		NewNode->type = 'D';
	}
	else if (checkFileType == 'F')
	{
		NewNode->type = 'F';
	}
	NewNode->parentPtr = start;
	NewNode->siblingPtr = NULL;
	NewNode->childPtr = NULL;
	return NewNode;
}

//checks whether or not pathname is absolute or relative
int checkAbsoluteRelative()
{
	//if pathname starts with a '/', absolute. remove the '/' and set start to root
	if (pathname[0] == '/') //absolute
	{
		char *temp = strdup(dname);
		strcpy(dname, "/");
		strcat(dname, temp);
		start = root;
	}
	//relative. Set start to the current working directory
	else
	{
		start = cwd;
	}

	//check if input of cd is /. if it is, set cwd to root and return 1 for the check.
	if (strcmp(pathname, "/") == 0)
	{
		cwd = root;
		return 1;
	}
	else
	{
		//if not, return 0
		return 0;
	}
}

//set start to the cwd
int Search(char * DirectoryPathName, char checkFileType)
{

	//get to parent node where directory is supposed to be placed.
		//if i write mkdir /A/B/C, it will set start to B (i.e. C's parent).
	while (DirectoryPathName)
	{
		start = start->childPtr;
		if (start == NULL)
		{
			printf("ERROR: %s Directory does not exist\n", DirectoryPathName);
			//return a 1 if it doesnt exist 
			return 1;
		}

		while (start != NULL)
		{
			//break out of the loop if we find the desired directory. 
			if (strcmp(start->name, DirectoryPathName) == 0)
			{
				//check if directory is a file 
				if (start->type == 'F' && checkFileType == 'D') //file, not directory
				{
					printf("ERROR: %s is a file\n", DirectoryPathName);
					return 1;
				}
				break;
				if (start->type == 'D') //directory, not file
				{
					printf("ERROR: %s is a directory\n", DirectoryPathName);
					return 1;
				}
				break;
			}
			//traverse to next sibling if directory is not found
			else
			{
				start = start->siblingPtr;
			}

		}
		//need to get next directory pathname between the slashes
		DirectoryPathName = strtok(NULL, "/");
	}

}

//make a new directory
void mkdirFunction(char checkFileType)
{
	//set the directory name and basename
	PathnameToDirBasename();
	checkAbsoluteRelative();

	//gets rid of the first slash in the dname and sets 
	char *DirectoryPathName = strtok(dname, "/");

	//if a directory of an absolute pathname does not exist, exit the function
	int checkExist = Search(DirectoryPathName, checkFileType);
	if (checkExist == 1)
		return;


	//checks for duplicate directory names
	Node *ChildCheckDuplicates = start->childPtr;
	while (ChildCheckDuplicates != NULL)
	{
		if (strcmp(ChildCheckDuplicates->name, bname) == 0)
		{
			printf("ERROR: %s Directory already exists", bname);
			return;
		}
		ChildCheckDuplicates = ChildCheckDuplicates->siblingPtr;
	}

	//allocate new directory soace
	Node * newD = NewNode(checkFileType);

	//if start is childless, set its child to the new directory
	if (start->childPtr == NULL)
	{
		start->childPtr = newD;
	}

	//if start has children, we have to set the new directory to the next one of child's siblings
	else
	{
		start = start->childPtr;
		while (start->siblingPtr != NULL)
		{
			start = start->siblingPtr;
		}
		//when we find that the child has no more siblings, set the directory to the next sibling
		start->siblingPtr = newD;
	}
	if (checkFileType == 'D')
	{
		printf("[!] New Directory '%s' created\n", bname);
	}
	else if (checkFileType == 'F')
	{
		printf("[!] New File '%s' created\n", bname);
	}
}

//make a new file
void creatFunction()
{
	char checkFileType = 'F';
	mkdirFunction(checkFileType);
}

//navigate items
void cdFunction()
{

	int check = 0, checkExist = 0;
	PathnameToDirBasename();
	check = checkAbsoluteRelative();

	if (check == 0)
	{
		char *DirectoryPathName = strtok(pathname, "/");

		//if a directory of an absolute pathname does not exist, exit the function
		int checkExist = Search(DirectoryPathName, NULL);
		if (checkExist == 1)
			return;
		if(start == NULL)
		{
			printf("ERROR: %s does not exist\n", DirectoryPathName);
			return;
		}
		if (start->type == 'F')
		{
			printf("ERROR: %s is a file\n", start->name);
			return;
		}

		//if the cwd and base name are not empty, set cwd to its child
		if ((cwd->name, bname) != 0)
		{
			cwd = start->childPtr;
		}
		cwd = start;
	}
	printf("[!] Directory changed to '%s'\n", cwd->name);
}

//remove a directory
void rmdirFunction(char checkFileType)
{
	int check = 0, checkExist = 0;
	char *DirectoryPathName = NULL;
	PathnameToDirBasename();
	check = checkAbsoluteRelative();

	if (check == 0)
	{
		if (strcmp(dname, "") == 0)
		{
			DirectoryPathName = strtok(dname, "/");
			//DirectoryPathName = bname;
			//strcpy(DirectoryPathName, bname);
		}
		else if (dname != NULL)
		{
			DirectoryPathName = strtok(pathname, "/");
		}
		//if a directory of an absolute pathname does not exist, exit the function
		int checkExist = Search(DirectoryPathName, checkFileType);
		if (checkExist == 1)
			return;


		//check if node exists in start's child's list of siblings
		Node *foo = start->childPtr; //go to basename
		if (foo == NULL) {
			printf("ERROR: Cannot delete. %s does not exist.", bname);
			return;
		}

		while (foo != NULL)
		{
			if (strcmp(foo->name, bname) == 0)
			{
				break;
			}
			else
			{
				foo = foo->siblingPtr;
				break;
			}

			printf("ERROR: Cannot delete. %s does not exist.", bname);
			return;
		}

		// foo = start->childPtr; //go to basename
		// if (foo != NULL) //check if empty
		// {
		// 	printf("ERROR: Directory is not empty.\n");
		// 	return;
		// }
		

		Node * temp = start->childPtr;

		//if the node desired to be removed is the child of the start, free it.		
		if (strcmp(temp->name, bname) == 0)
		{
			if (checkFileType == 'F' && temp->type == 'D')
			{
				printf("ERROR: %s is a directory", bname);
				return;
			}
			else if (checkFileType == 'D' && temp->type == 'F')
			{
				printf("ERROR: %s is a file", bname);
				return;
			}

			printf("[!] item %s removed.", start->childPtr->name);

			//set start->childptr to the sibling of the removed node.
			start->childPtr = temp->siblingPtr;
			free(temp->childPtr);
			return;
		}

		Node * sibling = start->childPtr->siblingPtr;
		start = start->childPtr;
		Node *last = start;
		while (1)
		{

			//find node we are wanting to delete
			if (strcmp(sibling->name, bname) == 0)
			{
				//check if the node has siblings

				if (sibling->siblingPtr != NULL)//sibling present
				{
					if (checkFileType == 'F' && sibling->type == 'D')
					{
						printf("ERROR: %s is a directory", bname);
						return;
					}
					else if (checkFileType == 'D' && sibling->type == 'F')
					{
						printf("ERROR: %s is a file", bname);
						return;
					}
					printf("[!] Item %s removed.", sibling->name);
					start->siblingPtr = sibling->siblingPtr;
					free(sibling);
					//free(start);
					return;
				}
				else if (sibling->siblingPtr == NULL)//no more siblings
				{
					if (checkFileType == 'F' && sibling->type == 'D')
					{
						printf("ERROR: %s is a directory", bname);
						return;
					}
					else if (checkFileType == 'D' && sibling->type == 'F')
					{
						printf("ERROR: %s is a file", bname);
						return;
					}
					printf("[!] Item %s removed.", sibling->name);
					start->siblingPtr = NULL;
					free(sibling);
					//free(start);
					return;
				}

			}
			else
			{
				//last holds the previous sibling. I.E. will hold A in A/B
				last = sibling;
				sibling = sibling->siblingPtr;
				start = last;
			}
		}
	}
}

//remove a file
void rmFunction()
{
	char checkFileType = 'F';
	rmdirFunction(checkFileType);
}

//print working directory
void pwdFunction()
{
	int i = 0;
	Node * curr = cwd;
	char nodes[100][100];

	//traverse from cwd to root, save names into array
	while ((strcmp(curr->name, "root") != 0))
	{
		strcpy(nodes[i], curr->name);
		curr = curr->parentPtr;
		i++;
	}
	//print out array
	i = i - 1;
	printf("root");
	while (i >= 0)
	{
		printf("/%s", nodes[i]);
		i--;
	}
	printf("\n");
}

//list contents of directory 
void lsFunction()
{
	Node *curr = cwd->childPtr;
	while (curr != NULL)
	{
		printf("[ %c  %s ]  ", curr->type, curr->name);
		curr = curr->siblingPtr;
	}
	printf("\n");
}

//check if the file is NULL or otherwise absent
int checkFileBroken()
{
	if (strcmp(pathname, "") == 0)
	{
		printf("ERROR: Empty Filename\n");
		return 0;
	}
	if (fp == NULL)
	{
		printf("ERROR: File is empty\n");
		return 1;
	}
}

char stringGlobal[];
//get strings to save to file
void FileGetString()
{
	// Node *foo = (struct node *)malloc(sizeof(struct node));
	// Node *foo2 = (struct node *)malloc(sizeof(struct node));
	int flag = 0;
	 char string1[128] = "";
	char string2[128] = "";
	Node *foo = cwd;
	Node *foo2 = cwd;

	while (foo->parentPtr != NULL)
	{
		strcpy(string1, foo->name);

		if (flag == 1)
		{
			strcat(string1, "/");
		}
		strcat(string1, string2);
		strcpy(string2, string1);
		foo = foo->parentPtr;
		flag = 1;
	}
	//check if the name is root. If root, write a "/" instead of "root" so we can use the pathname
	if (strcmp(foo->name, "root") == 0)
	{
		strcpy(string1, "/");
	}
	else
	{
		strcpy(string1, foo->name);
	}

	strcat(string1, string2);
	strcpy(string2, string1);

	//not creating seg fault
	if (foo2->type == 'D')
	{
		strcpy(string2, "D");
		strcat(string2, " ");
		strcat(string2, string1);
		strcpy(string1, string2);
	}
	else
	{
		strcpy(string2, "F");
		strcat(string2, " ");
		strcat(string2, string1);
		strcpy(string1, string2);
	}
	// printf(fp, "%s", string1);
	// printf(fp, "\n");
	strcpy(stringGlobal, string1);
	//return string1;
}

//save tree to file
void save()
{
	Node *currentNode = (struct node *)malloc(sizeof(struct node));

	char string1[128];
	int flag = 0, checkBroken = 0;
	fp = fopen("outfile.txt", "w+");
	//fprintf(fp, "Hello world");

	//check if the file is NULL or otherwise absent
	checkBroken = checkFileBroken();
	if (checkBroken == 1)
		return;

	currentNode = cwd;
	cwd = root;

	while (cwd != NULL)
	{
		if (cwd->childPtr != NULL && flag == 0)
		{
			strcpy(stringGlobal, "");
				FileGetString();
				strcpy(string1, stringGlobal);
			//strcpy(string1, FileGetString());
			fprintf(fp, "%s\n", string1);
			cwd = cwd->childPtr;
		}
		else if (cwd->siblingPtr != NULL)
		{
			if (flag == 0)
			{
				//strcpy(string1, FileGetString());
				strcpy(stringGlobal, "");
				FileGetString();
				strcpy(string1, stringGlobal);
				fprintf(fp, "%s\n", string1);
			}
			cwd = cwd->siblingPtr;
			flag = 0;
		}
		else
		{
			if (flag == 0)
			{				
				//strcpy(string1, FileGetString());
				//string1 = FileGetString();
				strcpy(stringGlobal, "");
				FileGetString();
				strcpy(string1, stringGlobal);
				fprintf(fp, "%s\n", string1);
			}
			cwd = cwd->parentPtr;
			flag = 1;
		 }
	}
	printf("Saved to %s\n", pathname);
	fclose(fp);
	
	cwd = currentNode;
}



//load tree from file
void reload()
{
	fp = fopen(pathname, "r");
	char tempString[128];
	int flag = 0, checkBroken = 0;
	
	checkBroken = checkFileBroken();
	if (checkBroken == 1)
		return;

	while (!feof(fp))
	{
		fscanf(fp, "%s", tempString);
		if (strcmp(tempString, "") == 0)
		{
			return;
		}
		if (strcmp(tempString, "D") == 0)
		{
			fscanf(fp, "%s", tempString);
			if (strcmp(tempString, "/") != 0)
			{
				strcpy(pathname, tempString);
				mkdirFunction('D');
			}
		}
		else if (strcmp(tempString, "F") == 0)
		{
			fscanf(fp, "%s", tempString);
			strcpy(pathname, tempString);
			creatFunction();
		}
		printf("\n");
	}
	fclose(fp);
}

void quit()
{
	run = 0;
	printf("\nEnding Sim\n");
}