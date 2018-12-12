#include <iostream>
#include <cstdlib>
#include <string>
#include "socket.h"
#include "log.h"
using namespace std;

int LOG::connect(int port)
{
	socket.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (socket.server(port) < 0)
		return -1;
	return 0;
}

void LOG::process(char message[]) // FIXME: this should read from buffer, but all processing should be done in class PUGBOT
{
	char steam[20];
	char text[128];
	//L 07/05/2004 - 19:03:59: "vanlje-emi2<6><STEAM_ID_PENDING><>" connected, address "202.89.164.51:27005"
	if (getsteamid(message,steam) == -1)
		return;
	if (gettext(message,text) == -1)
		return;
	ingamesay(steam,text);
	return;
}

int LOG::read(char *buffer)
{
	//read from socket here
	return(socket.read(buffer));
}

bool LOG::check()
{
	//select call here
	return(socket.checkfordata());
}

void LOG::close()
{
	socket.close();
}

int LOG::getip(char buffer[], char *ip)
{
    int j = 0;
    char *ptr = strstr(buffer, "connected, address \"");
    if (ptr != NULL)
    {
        ptr += 20;
        while (ptr[j] != ':' && ptr[j] != '"')
        {
            ip[j] = ptr[j];
            j++;
        }
        ip[j] = '\0';
        return j;
    }
    return -1;
}

void LOG::getname(char buffer[], char *name)
{
    int i = 0, j = 0;

    while (buffer[i] != '"') // eat characters up until first '"'
        i++;
    i++;

    while (buffer[i] != '"') // store characters up until last '"'
        name[j++] = buffer[i++];
    name[j] = '\0';

    i = strlen(name);
    j = 0;

    while (j != 3) // backtrack
    {
        if (name[i] == '<')
            j++;
        i--;
    }
    name[++i] = '\0';
}

int LOG::getsteamid(char buffer[], char *steam)
{
	int j = 0;
	char *ptr = strstr(buffer, "STEAM_");
	if (ptr != NULL)
	{
		while (ptr[j] != '>' && ptr[j] != '"')
		{
			steam[j] = ptr[j];
			j++;
		}
		steam[j] = '\0';
		return j;
	}
	return -1;
}

int LOG::gettext(char buffer[],char *text)
{
	int j = 0, k = 0;
	char *ptr = strstr(buffer, "say \"");
	if (ptr == NULL)
	{
		return -1;
	}

	for(; ptr[k] != '\"'; k++);
		k++;
	while ((ptr[k] != '\"') && (ptr[k] != ';')) //no cheekyness with ;s
	{
		text[j] = ptr[k];
		j++;
		k++;
	}
	text[j] = '\0';
	return j;
}
