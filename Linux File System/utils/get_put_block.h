#ifndef _GET_PUT_BLOCK_H
#define _GET_PUT_BLOCK_H

/*-----------------------------------------
Function: get_block
Use: loads data from block 'block' from 
	 device 'dev.' Stores results in 'buf' 
Throws errors?: -native
-----------------------------------------*/
int get_block(int dev, int block, char buf[]);

/*-----------------------------------------
Function: put_block
Use: writes data in 'buf' to block 'block'
	 on device 'dev' 
Throws errors?: -native
-----------------------------------------*/
int put_block(int dev, int block, char buf[]);

#endif