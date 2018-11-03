
#include "FileSystemFunctions.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

	root = malloc(sizeof(Node));
	//root->siblingPtr, root->parentPtr = root;
	root->childPtr = NULL;
	root->siblingPtr = NULL;
	root->parentPtr = NULL;
	strcpy(root->name, "root");
	root->type = 'D';

	cwd = root;
}

//empty globals, ask for command
void setMain()
{
	strcpy(line, "");
	strcpy(command, "");
	strcpy(pathname, "");
	bname[0] = '\0';
	dname[0] = '\0';
	pathname[0] = '\0';
	command[0] = '\0';
	line[0] = '\0';
	

	//Get user input
	printf("\n\nEnter command: ");
	fgets(line, 128, stdin);  // get at most 128 chars from stdin
	line[strlen(line) - 1] = 0; // kill \n at end of line
	sscanf(line, "%s %s", command, pathname);
}

//Display command specification
void displayMenu()
{
	printf("\n***********Commands available***********\n");
	printf("  mkdir [pathname]:		make a new directory. Try entering 'mkdir Hello'!\n");
	printf("  rmdir [pathname]:		remove directory if empty\n");
	printf("  cd [pathname]:		change CWD to pathname, or to root if 'cd %c' entered.\n", 92);
	printf("  ls:	 			list current directory contents\n");
	printf("  pwd:				print (absolute) pathname of CWD\n");
	printf("  creat [pathname]:		create a FILE node\n");
	printf("  rm [pathname]:		remove a FILE node\n");
	printf("  save:	 			save the current file system tree to outfile.txt, then terminate\n");
	printf("  reload:			construct file system tree from outfile.txt\n");
	printf("  quit:				terminate program\n");
}

//switch statement for finding command index
void CommandSwitch(int index)
{
	//set initial filetype to D for CheckFileType
	char type = 'D';
	switch (index)
	{
	case 0: //display meu
		displayMenu();
		break;
	case 1: //make new directory
		mkdirFunction(type);
		break;
	case 2: //remove a directory
		rmdirFunction(type);
		break;
	case 3: //list all elements of cwd
		lsFunction();
		break;
	case 4: //change directory
		cdFunction();
		break;
	case 5: //display pathname
		pwdFunction();
		break;
	case 6: //create a file
		creatFunction();
		break;
	case 7: //remove a file
		rmFunction();
		break;
	case 8: //load tree from file
		reload();
		break;
	case 9: //save tree to file
		//fp = fopen(pathname, "w+");
		save(pathname, root);
		break;
	case 10: //exit program
		quit();
		break;
	default:
		printf("Invalid command: %s\n", command);
		break;
	};
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

//allocate space for a new node
Node * NewNode(char *checkFileType)
{
	Node *NewNode = malloc(sizeof(Node));
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
//makes sure all elements in a given absolute pathname exist
int Search(char * DirectoryPathName, char checkFileType)
{
	Node * last = start;
	//get to parent node where directory is supposed to be placed.
		//if i write mkdir /A/B/C, it will set start to B (i.e. C's parent).
	while (DirectoryPathName)
	{
		start = start->childPtr;
		if (start == NULL)
		{
			printf("ERROR: %s Directory does not exist", DirectoryPathName);
			start = last;
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
					printf("ERROR: %s is a file", DirectoryPathName);
					return 1;
				}
				break;
				if (start->type == 'D') //directory, not file
				{
					printf("ERROR: %s is a directory", DirectoryPathName);
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
	char *DirectoryPathName = NULL;
	//set the directory name and basename
	checkAbsoluteRelative();
	PathnameToDirBasename();
	

	//gets rid of the first slash in the dname and sets DirectoryPathName
	//if (pathname[0] == '/') //absolute
	//{
		DirectoryPathName = strtok(dname, "/");
	//}
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

	//allocate new directory space
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

	//filetype check
	if (checkFileType == 'D')
	{
		printf("[!] New Directory '%s' created", bname);
	}
	else if (checkFileType == 'F')
	{
		printf("[!] New File '%s' created", bname);
	}
}

//make a new file
void creatFunction()
{
	char checkFileType = 'F';
	mkdirFunction(checkFileType);
}

//navigate through items
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
		if (start == NULL)
		{
			printf("ERROR: %s does not exist", DirectoryPathName);
			return;
		}
		if (start->type == 'F')
		{
			printf("ERROR: %s is a file", start->name);
			return;
		}


		//if the cwd and base name are not empty, set cwd to its child
		if ((cwd->name, bname) != 0)
		{
			cwd = start->childPtr;
		}
		cwd = start;
	}
	printf("[!] Directory changed to '%s'", cwd->name);
}

//filetype check for remove 
int removeCheckFileType(char checkFileType, Node * node)
{
	if (checkFileType == 'F' && node->type == 'D')
	{
		printf("ERROR: %s is a directory", bname);
		return -1;
	}
	else if (checkFileType == 'D' && node->type == 'F')
	{
		printf("ERROR: %s is a file", bname);
		return -1;
	}
}

//remove a directory
void rmdirFunction(char checkFileType)
{
	int check = 0, checkExist = 0;
	//set the directory name and basename
	PathnameToDirBasename();
	checkAbsoluteRelative();

	//gets rid of the first slash in the dname and sets DirectoryPathName
	char *DirectoryPathName = strtok(dname, "/");

	//if a directory of an absolute pathname does not exist, exit the function
	/*if ((start->name, dname[0]) == "/")
	{*/
		checkExist = Search(DirectoryPathName, checkFileType);
		if (checkExist == 1)
			return;

		/*checkExist = Search(DirectoryPathName, checkFileType);
		if (checkExist == 1)
			return;*/
	//}

	//char *DirectoryPathName = NULL;
	//PathnameToDirBasename();
	//check = checkAbsoluteRelative();

	//if (check == 0)
	//{
	//	if (strcmp(dname, "") == 0)
	//	{
	//		DirectoryPathName = strtok(dname, "/");
	//		//DirectoryPathName = bname;
	//		//strcpy(DirectoryPathName, bname);
	//	}
	//	else if (dname != NULL)
	//	{
	//		DirectoryPathName = strtok(pathname, "/");
	//	}
	//	//if a directory of an absolute pathname does not exist, exit the function
	//	/*if ((start->name, DirectoryPathName) == "/")
	//	{*/
	//		int checkExist = Search(DirectoryPathName, checkFileType);
	//		if (checkExist == 1)
	//			return;
	//	//}

		//check if node exists in start's child's list of siblings
		Node *temp = start->childPtr;
		int found = 0;	//check for making sure node even exists
		while (temp != NULL)
		{
			if (strcmp(temp->name, bname) == 0)
			{
				found = 0;
				break;
			}
			else
			{
				temp = temp->siblingPtr;
				found = 1;
			}
		}
		//did we find the node? 
		if (found == 1)
		{
			printf("ERROR: Cannot delete. %s does not exist.", bname);
			return;
		}

		//check if the found directory has children. if it does we cannot delete it
		temp = temp->childPtr;

		if (temp != NULL) //check if empty
		{
			printf("ERROR: Directory is not empty");
			return;
		}


		temp = start->childPtr;

		//if the node desired to be removed is the child of the start, free it.		
		if (strcmp(temp->name, bname) == 0)
		{
			//check filetypes match
			int check = removeCheckFileType(checkFileType, temp);
			if (check < 0)
				return;

			printf("[!] item %s removed.", start->childPtr->name);

			//set start->childptr to the sibling of the removed node.
			start->childPtr = temp->siblingPtr;
			free(temp->childPtr);
			return;
		}

		//if not, start checkin siblings!
		Node * sibling = start->childPtr->siblingPtr;
		start = start->childPtr;
		Node *last = start;
		while (1)
		{

			//find node we are wanting to delete
			if (strcmp(sibling->name, bname) == 0)
			{
				//check filetypes match
				int check = removeCheckFileType(checkFileType, sibling);
				if (check < 0)
					return;

				//check if the node has siblings
				if (sibling->siblingPtr != NULL)//sibling present
				{
					printf("[!] Item %s removed.", sibling->name);
					start->siblingPtr = sibling->siblingPtr;
					free(sibling);
					//free(start);
					return;
				}
				else if (sibling->siblingPtr == NULL)//no more siblings
				{
					printf("[!] Item %s removed.", sibling->name);
					start->siblingPtr = NULL;
					free(sibling);
					//free(start);
					return;
				}
			}
			else
			{
				//last holds the previous sibling. I.E. will hold A in A, B
				last = sibling;
				sibling = sibling->siblingPtr;
				start = last;
			}
		}
	//}
}

//remove a file
void rmFunction()
{
	char checkFileType = 'F';
	rmdirFunction(checkFileType);
}

//print working directory
//prints iteratively as opposed to recursively
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
}

//get strings to save to file
char *FileGetString()
{
	Node *temp = malloc(sizeof(Node));
	Node *temp2 = malloc(sizeof(Node));
	int flag = 0;
	char string1[128] = "";
	char string2[128] = "";
	temp = cwd;
	temp2 = cwd;

	while (temp->parentPtr != NULL)
	{
		strcpy(string1, temp->name);

		if (flag == 1)
		{
			strcat(string1, "/");
		}
		strcat(string1, string2);
		strcpy(string2, string1);
		temp = temp->parentPtr;
		flag = 1;
	}
	//check if the name is root. If root, write a "/" instead of "root" so we can use the pathname upon reload
	if (strcmp(temp->name, "root") == 0)
	{
		strcpy(string1, "/");
	}
	else
	{
		strcpy(string1, temp->name);
	}

	//C string manipulation.....
	strcat(string1, string2);
	strcpy(string2, string1);
	if (temp2->type == 'D')
	{
		CopyStringCleanup(string1, string2, "D");
	}
	else
	{
		CopyStringCleanup(string1, string2, "F");
	}
	//printf(fp, "%s", string1);
	//printf(fp, "\n");
	return string1;
}

//save tree to file
void save()
{
	Node *currentNode = malloc(sizeof(Node));
	char tempString[128];
	int flag = 0, checkBroken = 0;
	FILE *fp = fopen("outfile.txt", "w+");

	currentNode = cwd;
	cwd = root;

	while (cwd != NULL)
	{
		if (cwd->childPtr != NULL && flag == 0)
		{
			strcpy(tempString, FileGetString());
			fprintf(fp, "%s\n", tempString);
			cwd = cwd->childPtr;
		}
		else if (cwd->siblingPtr != NULL)
		{
			if (flag == 0)
			{
				strcpy(tempString, FileGetString());
				fprintf(fp, "%s\n", tempString);
			}
			cwd = cwd->siblingPtr;
			flag = 0;
		}
		else
		{
			if (flag == 0)
			{
				strcpy(tempString, FileGetString());
				fprintf(fp, "%s\n", tempString);
			}
			cwd = cwd->parentPtr;
			flag = 1;
		}
	}
	fclose(fp);
	cwd = currentNode;
	printf("Saving to outfile.txt...");
	quit();
}

//load tree from file
void reload()
{
	FILE *f = fopen("outfile.txt", "r");
	char tempString[128];
	int flag = 0, checkBroken = 0;

	while (!feof(f))
	{
		fscanf(f, "%s", tempString);

		//exit if empty
		if (strcmp(tempString, "") == 0)
		{
			return;
		}
		//if filetype is directory, make new directory 
		if (strcmp(tempString, "D") == 0)
		{
			fscanf(f, "%s", tempString);
			if (strcmp(tempString, "/") != 0)
			{
				strcpy(pathname, tempString);
				mkdirFunction('D');
			}
		}

		// if filetype is file, create new file
		else if (strcmp(tempString, "F") == 0)
		{
			fscanf(f, "%s", tempString);
			strcpy(pathname, tempString);
			creatFunction();
		}
		printf("\n");
	}
	fclose(f);
}

//cleanup of string manipulation
void CopyStringCleanup(char string1[], char string2[], char FileType[])
{
	strcpy(string2, FileType);
	strcat(string2, " ");
	strcat(string2, string1);
	strcpy(string1, string2);
}

//terminate program
void quit()
{
	run = 0;
	printf("\n\nEnding Sim\n");
}