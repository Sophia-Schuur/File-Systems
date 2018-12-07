#ifndef _PARSE_H_
#define _PARSE_H_


/*-----------------------------------------
Function: parse 
Use: tokenizes 'line' by the value of 
	 'delimiters' and dynamically stores
	 result in 'buf.' 
	 *DOES NOT AFFECT INPUT*
Throws errors?: -native
-----------------------------------------*/
int parse(const char *input, char *delimiters, char ***buf);

/*-----------------------------------------
Function: free_array 
Use: properly frees dynamically allocated
	 two-dimensional char array
Throws errors?: 
-----------------------------------------*/
void free_array(char **arr);

#endif