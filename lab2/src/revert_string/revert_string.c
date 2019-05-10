#include "revert_string.h"
#include <string.h>

void RevertString(char *str)
{
	int lstr = strlen(str) - 1;
	char* temp = str;
	char ct;
	while(lstr > 0)
	{
		ct = *temp;
		*temp  = *(temp + lstr);
		*(temp + lstr) = ct;
		temp++;
		lstr -= 2;
	}
	return;
}

