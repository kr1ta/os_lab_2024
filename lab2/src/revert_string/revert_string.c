#include "revert_string.h"

void RevertString(char *str)
{
	char *start = str;          
    char *end = str;          

    while (*end != '\0') 
	{
        end++;
    }
    end--; 

    while (start < end) 
	{
        char temp = *start;
        *start = *end;
        *end = temp;

        start++;
        end--;
    }
}
