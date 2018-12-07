#ifndef _FILE_H_
#define _FILE_H_

#include "type.h"

int duplicate_oft(MINODE *mip);

OFT *get_oft();

int open_file(MINODE *mip, int mode);

int myread(int fd, char *buf, int nbytes);

int creat_file(MINODE *parent_mip, char *name);

int mywrite(int fd, char *buf, int nbytes);

#endif
