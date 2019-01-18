#include "mysh.h"

int main (int argc, char **argv, char* const envp[])
{
    printf("\nNOTE: Press ctrl + c or type 'exit' to exit at any time.\n");
    char* inputString = NULL;
    char** inputArray = NULL;
  
    GetSetPath();
    GetSetHome();

    while(1)
    {

        inputString = getInput();
        printf("Entered: %s\n\n", inputString);
        inputArray = parseInput(inputString);
        Execute(inputArray, envp);
    }
	return 0;
}