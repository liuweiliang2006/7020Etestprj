#include<stdio.h>
#include "string.h"
#include "stdlib.h"

/*将大写字母转换成小写字母*/  
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


static void htoi(char s[],char chstr)  
{  
    int i = 0;  
    int n = 0;  
	int data;

	data = chstr / 16;
	if(data<10)
	{
		s[0] = data +'0';
	}
	else if(data>=0xa && data <=0xf)
	{
		s[0] = data - 0xa+'A';
	}

	data = chstr % 16;
	if(data<10)
	{
		s[1] = data +'0';
	}
	else if(data>=0xa && data <=0xf)
	{
		s[1] = data - 0xa+'A';
	}
}  

void CharToHex(char *src,char * des)
{
	int i,length = 0;
	char chStr[2];

	length = strlen(src);
	for( i =0 ; i < length ; i++)
	{
		htoi(chStr,src[i]);
		des[i*2] = chStr[0];
		des[i*2+1] = chStr[1];
	}
	des[length*2] = '\0';
}

