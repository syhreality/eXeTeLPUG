#include <iostream>
#include <ctime>
#include <sys/time.h>
#include <unistd.h>
#include "config.h"
#include "extras.h"

void thetime(char time[])
{
    char time_string[40];
    struct timeval tv;
    struct tm* ptm;

    gettimeofday(&tv, NULL);
    ptm = localtime(&tv.tv_sec);
    strftime(time_string, sizeof(time_string), "Server time: %d/%m/%Y - %H:%M:%S", ptm);
    strcpy(time, time_string);

    return;
}

char* stristr(const char *pszText, const char *pszSub) // case insensitive implementation of strstr
{
	if (!pszText || !pszSub || !*pszText || !*pszSub)
		return 0;
	int nLen = strlen(pszSub);
	const char test(toupper(*pszSub));
	while (*pszText)
	{
		if (toupper(*pszText) == test)
		{
			if (strnicmp(pszText,pszSub,nLen)==0)
				return (char*)pszText;
		}
		pszText++;
	}
	return 0;
}
