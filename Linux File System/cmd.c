#include "cmd.h"

#include "utils/error_manager.h"
#include "utils/error_manager.h"

#include <string.h>

typedef struct cmd
{
	char *name;
	int (*cmd)(int, char**);
}CMD;

static const CMD commands[] = 
{
    { "ls"      , &js_ls      },
    { "cd"      , &js_cd      },
    { "pwd"     , &js_pwd     },
    { "mkdir"   , &js_mkdir   },
    { "rmdir"   , &js_rmdir   },
    { "creat"   , &js_creat   },
    { "link"    , &js_link    },
    { "unlink"  , &js_unlink  },
    { "symlink" , &js_symlink },
    { "chmod"   , &js_chmod   },
    { "chown"   , &js_chown   },
    { "stat"    , &js_stat    },
    { "touch"   , &js_touch   },
    { "open"    , &js_open    },
    { "pfd"     , &js_pfd     },
    { "close"   , &js_close   },
    { "lseek"   , &js_lseek   },
    { "cat"     , &js_cat     },
    { "cp"      , &js_cp      }
};

int invalid_command(int argc, char* argv[])
{
    set_error("invalid command\n");
    return 0;
}

int (*get_cmd(char *cmd))(int, char**)
{
	int i;
	for(i = 0; i < COMMAND_COUNT; i++)
	{
		if(strcmp(cmd, commands[i].name) == 0)
		{
			return commands[i].cmd;
		}
	}
	return &invalid_command;
}