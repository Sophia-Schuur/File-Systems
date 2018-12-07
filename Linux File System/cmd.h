#ifndef _CMD_
#define _CMD_

#define COMMAND_COUNT 19

#include "utils/type.h"


int (*get_cmd(char *cmd))(int, char**);

int invalid_command(int argc, char *argv[]);


//level 1
int mount_root();

int js_ls(int argc, char *argv[]);
int js_cd(int argc, char *argv[]);
int js_pwd(int argc, char *argv[]);
int js_mkdir(int argc, char *argv[]);
int js_rmdir(int argc, char *argv[]);
int js_creat(int argc, char *argv[]);
int js_link(int argc, char *argv[]);
int js_unlink(int argc, char *argv[]);
int js_symlink(int argc, char *argv[]);
int js_chmod(int argc, char *argv[]);
int js_chown(int argc, char *argv[]);
int js_stat(int argc, char *argv[]);
int js_touch(int argc, char *argv[]);

//level2
int js_open(int argc, char *argv[]);
int js_close(int argc, char *argv[]);
int js_pfd(int argc, char *argv[]);
int js_lseek(int argc, char *argv[]);
int js_cat(int argc, char *argv[]);
int js_cp(int argc, char *argv[]);



#endif


