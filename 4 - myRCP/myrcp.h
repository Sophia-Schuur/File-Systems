 #ifndef MYRCP_H
 #define MYRCP_H
 #include <stdio.h>       // for printf()
#include <stdlib.h>      // for exit()
#include <string.h>      // for strcpy(), strcmp(), etc.
#include <libgen.h>      // for basename(), dirname()
#include <fcntl.h>       // for open(), close(), read(), write()
#include <stdbool.h>
// for stat syscalls
#include <sys/stat.h>
#include <unistd.h>

// for opendir, readdir syscalls
#include <sys/types.h>
#include <dirent.h>

// #define true 1;
// #define false 0;

int cpf2f(char *f1, char *f2);
int cpf2d(char *f1, char *f2);
int cpd2d(char *f1, char *f2);
int myrcp(char *f1, char *f2);


 #endif