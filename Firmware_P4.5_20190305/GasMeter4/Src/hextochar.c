#include <stdio.h>
#include "string.h"
#include "stdlib.h"

/*����д��ĸת����Сд��ĸ*/  
static int tolower(int c)  
{  
    if (c >= 'A' && c <= 'Z')  
    {  
        return c + 'a' - 'A';  
    }  
    else  
    {  
        return c;  
    }  
}


static char htoi(char s[])  
{  
    int i = 0;  
    int n = 0;  
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X'))  
    {  
        i = 2;  
    }  
    else  
    {  
        i = 0;  
    }  
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i)  
    {  
        if (tolower(s[i]) > '9')  
        {  
            n = 16 * n + (10 + tolower(s[i]) - 'a');  
        }  
        else  
        {  
            n = 16 * n + (tolower(s[i]) - '0');  
        }  
    }  
    return n;  
}  

void HexToChar(char *src,char * des)
{
	int i,length = 0;
	char chStr[2];

	length = strlen(src);
	for( i =0 ; i < length/2 ; i++)
	{
		chStr[0] = src[i*2];
		chStr[1] = src[i*2+1];
		des[i] = htoi(chStr);
		chStr[0] = '\0';
		chStr[1] = '\0';
	}
	des[i] = '\0';
}
