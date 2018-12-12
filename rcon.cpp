#include <iostream>
#include <cstdlib>
#include <string>
#include "config.h"
#include "socket.h"
#include "rcon.h"
#if OS == 2
#include <unistd.h>
#endif
using namespace std;

RCON::RCON()
{
	live = false;
	running = true;
}

int RCON::connect(char server[], int port)
{
	socket.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket.connect(server, port) < 0)
		return -1;

	return 0;
}

int RCON::challenge(char password[])
{
	string buffer;
	int i = -1;

	buffer = "\xff\xff\xff\xff";
	buffer += "challenge rcon";

	if ((socket.write((char*)buffer.c_str()) == (-1)))
		return -1;

	while (i == -1)
		i = socket.read(response);
	response[i] = '\0';

	sscanf(response, "%*s %*s %s", chnum);
	strncpy(rconpass, password, strlen(password)+1);

	return 0;
}

int RCON::sendcmd(const char command[])
{
	string buffer;
	
	if (!strcmpi("quit", command))
		return 0;
	if (!strcmpi("rcon", command))
		return 0;

	// rcon string format is: rcon chnum "rconpass" command
	buffer = "\xff\xff\xff\xff";
	buffer += "rcon ";
	buffer += chnum;
	buffer += " \"";
	buffer += rconpass;
	buffer += "\" ";
	buffer += command;
	buffer += "\n";
	// ie. rcon 1112653200 "gentoo" sv_restart 1

	if (socket.write((char*)buffer.c_str()) == -1)
	{
		perror("Error writing RCON Command");
		return 1;
	}
	return 0;
}

void RCON::liveonthree()
{
	sendcmd("mp_freezetime 7");
	sendcmd("mp_friendlyfire 1");
    sendcmd("mp_startmoney 800");
	sendcmd("sv_alltalk 0");
    Sleep(1000);
	sendcmd("say [ eXeTeL CS Server Configuration Completed ]");
	Sleep(1000);
	sendcmd("say [ eXeTeL CS Server : Now Restarting Rounds ]");
	Sleep(1000);
	sendcmd("say [ eXeTeL : Get Ready for Live on Three ]");
	Sleep(1000);
	sendcmd("sv_restart 1");
	Sleep(1000);
	sendcmd("sv_restart 1");
	Sleep(1000);
	sendcmd("sv_restart 3");
	Sleep(3500);
	sendcmd("say [ eXeTeL : Game is Live ! ]");
	Sleep(500);
	sendcmd("say [ eXeTeL : Good Luck, Have Fun ! ]");

	return;
}

void RCON::close()
{
	socket.close();
}

