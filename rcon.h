#ifndef RCON_H
#define RCON_H
#include "config.h"
#include "socket.h"

class RCON
{
	public:
		RCON();
		int connect(char[], int);
		int challenge(char[]);
		int sendcmd(const char[]);
		void liveonthree();
		void close();
	protected:
		bool live;
	private:
		SOCK socket;
		char response[1500];
		bool running;
		char chnum[32];
		char rconpass[16];
};

#endif
