#include "myrcp.h"

int main(int argc, char *argv[])
{
	printf("\nSophie's cp -r file copier!\n");
	printf("Can copy REG(regular), LNK(shortcuts) and DIR(directory) files.\n");
	
	 if (argc < 3)
	 {
	 	//print usage and exit;
	 	printf("\n%s\n", argv[0]);
	 	printf("ERROR: not enough arguments. Exiting.\n");
	 	exit(1);
	 }
    
  return myrcp(argv[1], argv[2]);
}