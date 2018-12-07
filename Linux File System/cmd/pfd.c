#include <stdio.h>
#include "../utils/type.h"

extern PROC *running;
extern OFT oft[NOFT];

int js_pfd(int argc, char *argv[])
{
	int i;
	OFT *ofp;

	printf("\n  fd   mode   offset   device   inode \n");
	printf("  --   ----   ------   ------   ----- \n");

	for(i = 0; i < NFD; i++)
    {
        ofp = running->fd[i];

        if(ofp == NULL)
        {
        	continue;
        }
        else if(ofp->refCount == 0)
        {
            continue;
        }

        printf("  %02x   ", i);

        switch(ofp->mode)
        {
        case 0:
            printf("RD");
            break;
        case 1:
            printf("WR");
            break;
        case 2:
            printf("RW");
            break;
        case 3:
            printf("AP");
            break;
        default:
            printf("--");
            break;
        }

        printf("%6d       %2d     %5d\n", ofp->offset, ofp->mptr->dev, ofp->mptr->ino);
    }
    return 0;
}