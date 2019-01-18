#include "mysh.h"


 char* getInput()
 {
    char tokenBuf[1024];
    char *temp = NULL;
    int tokenLength = -1;

    printf("\nEnter a command: ");

    //get line
    fgets(tokenBuf, 1024, stdin);
    tokenLength = strlen(tokenBuf);
    temp = (char*)malloc(sizeof(char)*tokenLength);

    strcpy(temp, tokenBuf);

    if(temp[tokenLength-1] == '\n')
    {
        temp[tokenLength-1] = 0;
    }
    return temp;
 }


 char** parseInput(char* input)
 {
    int count = 0;
    char *token = NULL, *temp = NULL;;
    
    char** inputArr = (char**)malloc(sizeof(char*)*20);

    token = strtok(input, " ");
    while(token)
    {
        temp = (char *)malloc(sizeof(char)*strlen(token));
        strcpy(temp, token);
        inputArr[count] = temp;
        count++;
        token = strtok(NULL, " ");
    }
    inputArr[count] = NULL;
    return inputArr;
 }


 void Execute(char** input, char* const envp[])
 {
    char *temp;
    int i = 0;

    if(strcmp(input[0], "cd") == 0)
    {
        cd(input[1]);
    }
    else if(strcmp(input[0], "exit") == 0)
    {
        exitProgram();
    }
    else
    {
        forkChild(input, envp);
    }

    //free array
    while(input[i])
    {
        free(input[i++]);
    }
    free(input);
 }


 void cd(char* pathname)
 {
    if(pathname)
    {
        chdir(pathname);
    }
    else
    {
        chdir(homePath);
    }
 }


 void otherCommand(char** input, char* const envp[])
 {
    char first[128];
    char new[1024];
    int i = 0;
    strcpy(first, input[0]);

   //Check each bin folder
    while(commandPath[i]) 
    {
        printf("path[%d] = %s\n", i, commandPath[i]);
        strcpy(new, commandPath[i]);
        strcat(new, "/");
        strcat(new, first);
        execve(new, input, envp);
        i++;
    }
    printf("ERROR: Command %s not found\n", input[0]);
 }


 void GetSetPath()
 {
    char *token;
    int i = 0;
    char *name = "PATH";
    char *temp = getenv(name);
    token = strtok(temp, ":");
    while(token != NULL)
    {
        commandPath[i++] = token;
        token = strtok(NULL,":");
    }
    commandPath[i] = NULL;
    i = 0;
}


 void GetSetHome()
 {
    int j = 0, i = 0;
    const char* name = "HOME";
    char *temp = getenv(name);
    homePath = (char*)malloc(sizeof(char)*strlen(temp));
    while(temp[i])
    {
       homePath[j++] = temp[i++];
     }
     homePath[j] = 0;
 }


 int forkChild(char** input, char* const envp[])
 {
    int temp = 0, check = 0, isPipe = 0, i = 0;
    char** head = NULL;
    char** tail = NULL;

    head = (char**)malloc(sizeof(char*)*20);
    tail = (char**)malloc(sizeof(char*)*20);


    while(input[i])
    {
        printf("myargv[%d] = %s\n", i, input[i]);
        i++;
    }
    printf("myargv[%d] = %s\n\n", i, input[i]);
    i=0;

    temp = fork();

    if(temp < 0)
    {
        printf ("ERROR: Could not fork\n");
        exit(1);
    }

    if(temp)
    {   
        temp = wait(&check);
    }
    else
    {
        isPipe = CheckIfPipe(input, head, tail);
        if(isPipe)
        {
            NewPipe(head, tail, envp);
        }
        else
        {
            runProgram(input, envp);
        }
    }
    free(head);
    free(tail);
    return check;
 }


 char* isRedirect(char** input, int* set)
 {
    int i = 1;
    char* temp = NULL;
    char* temp2 = NULL;

    while(!(*set) && input[i])
    {
        if(!(strcmp(input[i], "<")))
        {
            *set = 1;
        }
        else if(!(strcmp(input[i], ">"))) 
        {
            *set = 2;
        }
        else if(!(strcmp(input[i], ">>"))) 
        {
            *set = 3;
        }
        if(*set)
        {
            //reset input
            temp = input[i+1];          
            temp2 = input[i];
            input[i] = NULL;
            input[i+1] = NULL;
            free(temp2);
        }
        i++;
    }
    return temp;
 }


 void redirect(char* file, int set)
 {
    switch(set)
    {
        case 1: close(0);
            open(file, O_RDONLY);
            break;

        case 2: close(1);
            open(file, O_WRONLY|O_CREAT, 0644);
            break;

        case 3: close(1);
            open(file, O_APPEND|O_WRONLY|O_CREAT, 0644);
            break;

        default: printf("ERROR: File not found\n");
            break;
    }
 }


int CheckIfPipe(char** input, char** head, char** tail)
{
    int i = 0, j = 0, found = 0;
    char *temp;
    while(!found && input[i])
    {
        if(!strcmp(input[i], "|")) 
        {
            found = 1;
            temp = input[i];
            input[i++] = NULL;
            free(temp);

            while(input[i]) 
            {
                tail[j++] = input[i++];
            }

            tail[j] = NULL;
            input[i] = NULL;
            j = 0, i = 0;
            
            while(input[i])
            {
                head[j++] = input[i++];
            }
            head[j] = NULL;
            free(input);
        }
        i++;
    }
    return found;
}


void NewPipe(char** head, char** tail, char* const envp[])
{
   int pd[2];
   pipe(pd);

   if(fork() == 0)
   {
        close(pd[0]);
        dup2(pd[1], 1);
        close(pd[1]);
        runProgram(head, envp);
    }
    else 
    {
        close(pd[1]);
        dup2(pd[0], 0);
        close(pd[0]);
        runProgram(tail, envp);
    }
}


void runProgram(char** input, char* const envp[])
{
    int set = 0;
    char* file = NULL;
    
    file = isRedirect(input, &set);
    if(set)
    {
        redirect(file, set);
    }
    otherCommand(input, envp);
}

 void exitProgram()
 {
    exit(1);
 }