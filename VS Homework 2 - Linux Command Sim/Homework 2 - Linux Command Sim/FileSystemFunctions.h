#ifndef MYFILESYSTEMFUNCTIONS_H
#define MYFILESYSTEMFUNCTIONS_H

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

//GLOBAL VARIABLES//
Node *root = NULL, *cwd = NULL, /**curr = NULL,*/ *start = NULL;
char line[128];                   // for gettting user input line
char command[16], pathname[64];   // for command and pathname strings
char dname[64], bname[64];        // for dirname and basename 
char *name[100];                  // token string pointers 
int  n;                           // number of token strings in pathname 
//FILE *fp;
int run = 1;

//Command Table/////////////////
extern char *cmdTable[] = { "menu", "mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "quit", 0 };
//int(*fptr[])(char *) = { (int(*)())mkdir, rmdir, ls, cd, pwd, creat, rm, reload, save, quit};
/////////////////

//return the command table index (see above table)
int findCmd(char *command);

//Clear global variables, set pointers to null,  set the current working directory to root
void initialize();

//empty globals, ask for command
void setMain();

//Display command specification
void displayMenu();

//switch statement for finding command index
void CommandSwitch(int index);

//sets dirname and basename of given pathname
void PathnameToDirBasename();

//allocate space for a new node
Node * NewNode(char *checkFileType);

//checks whether or not pathname is absolute or relative
int checkAbsoluteRelative();

//set start to the cwd
//makes sure all elements in a given absolute pathname exist
int Search(char * DirectoryPathName, char checkFileType);

//make a new directory
void mkdirFunction(char checkFileType);

//make a new file
void creatFunction();

//navigate through items
void cdFunction();

//filetype check for remove 
int removeCheckFileType(char checkFileType, Node * node);

//remove a directory
void rmdirFunction(char checkFileType);

//remove a file
void rmFunction();

//print working directory
//prints iteratively as opposed to recursively
void pwdFunction();

//list contents of directory 
void lsFunction();

//get strings to save to file
char *FileGetString();

//save tree to file
void save();

//load tree from file
void reload();

//cleanup of string manipulation
void CopyStringCleanup(char string1[], char string2[], char FileType[]);

//terminate program
void quit();

#endif // !MYFILESYSTEMFUNCTIONS_H