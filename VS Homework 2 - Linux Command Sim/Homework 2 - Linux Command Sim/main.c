//Sophia Schuur
//A windows friendly simulation of some basic Linux commands.
//Last Updated: 10/8/2018
//Edited for submission to Chief Architect Software Internship program

#include "FileSystemFunctions.c"
#include "FileSystemFunctions.h"
int main(void)
{
	
	initialize();
	printf("\nSophie's basic Linux command sim! Enter 'menu' for assistance with commands.");
	while (run)
	{
		setMain();
		
		int index = findCmd(command);
		int check = 0;

		//check to make sure necessary commands have filenames
		//if filename (pathname) absent, restart the main loop and ask for another command
		if (index == 1 || index == 2 || index == 6 || index == 7 /*|| index == 8 || index == 9*/)
		{
			if ((strcmp(pathname, "") == 0))
			{
				printf("ERROR: No filename detected.\n");
				check = 1;
			}
		}
		if (check != 1) //filename correct
		{
			CommandSwitch(index);
		}
		
	}

	//note to self:
	// mkdir /C - makes directory at root i.e root/c
	//mkdir C - makes folder at currewnt working direcotry i.e. root/A/b/C

	
	system("PAUSE");
	return 0;
}