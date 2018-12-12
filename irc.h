#ifndef IRC_H
#define IRC_H
#include <fstream>
#include <iostream>
#include "config.h"
#include "socket.h"
using namespace std;

class IRC
{
    public:
        int connect(char[], char[], int);
        int join(char[]);
		int leave(char[]);
        int private_message(char[], char[]);
        int send_channel(char[], char[]);
        int send_notice(char[], char[]);
		int set_topic(char[], char[]);
        int change_nick(char[]);
        int raw_command(char[]);
        int read(char*);
		int quit(char message[]);
		int process(char message[]);
		bool check();

		char *GetMyName() { return myName; }
		virtual void ontext(char[],char[],char[], char[]) { return; } //chan + nick + vhost + text
		virtual void onpart(char[]) { return; }//nick
		virtual void onjoin(char chan[],char nick[]) { return; };
		virtual void onnamechange(char[],char[]) { return; } //old nick + new nick

	protected:
		char circnick[32], ircserver[32], ircnick[32], ircchannel[32], ircadminchannel[32];
		int ircport;

    private:
		SOCK socket;
        char buffer[512];
		char myName[128];
		char szNICK[128];
		char szUSER[128];
		char szCOMMAND[128];
		char szCHAN[128];
		char szTEXT[8192];
		char szREST[8192];
		char szVHOST[128];
};

#endif
