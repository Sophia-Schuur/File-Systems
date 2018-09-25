#include "filesystemfunctions.c"

int main(void)
{
	
	initialize();
	
	while (run)
	{
		strcpy(line, "");
		strcpy(command, "");
		strcpy(pathname, "");
		//Get user input
		printf("\nEnter command: ");
		fgets(line, 128, stdin);  // get at most 128 chars from stdin
		line[strlen(line) - 1] = 0; // kill \n at end of line
		sscanf(line, "%s %s", command, pathname);
		//printf("Command: %s | Pathname: %s\n\n", command, pathname);


		int index = findCmd(command);

		//check to make sure necessary commands have filenames
		if (index == 1 || index == 2 || index == 6 || index == 7 || index == 8 || index == 9)
		{
			if ((strcmp(pathname, "") == 0))
			{
				printf("ERROR: No filename detected\n");
			}
		}
		char type = 'D';
		switch (index)
		{
		case 0:
			displayMenu();
			break;
		case 1:
			
			//path2node(pathname);
			mkdirFunction(type);
			break;
		case 2:
			rmdirFunction(type);
			break;
		case 3:
			lsFunction();
			break;
		case 4:
			cdFunction();//change directory
			break;
		case 5:
			pwdFunction();//display pathname
			break;
		case 6:
			creatFunction();
			break;
		case 7:
			rmFunction();
			break;
		case 8:
			reload();
			break;
		case 9:
			fp = fopen(pathname, "w+");
			save();
			break;
		case 10:
			quit();
			break;
		default:
			printf("Invalid command: %s\n", command);
			break;
		};
	}

	// mkdir /C - makes directory at root i.e root/c
	//mkdir C - makes folder at currewnt working direcotry i.e. root/A/b/C

	
	//system("PAUSE");
	return 0;
}