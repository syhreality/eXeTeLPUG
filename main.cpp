#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "md5.h"
#include "pugbot.h"
using namespace std;

int main(int argc, char *argv[])
{
	char ircserver[64], ircnick[32], ircchannel[32], ircadminchannel[32];
    char gameserver[32], hostserver[32], rconpass[16], news[80];
    char password[32], passphrase[36] = "171852795695163624615230f364e59f";
	unsigned int ircport, gameport, hltvport, logport;

    MD5 context;
    ifstream ins;

    srand(time(0));
    cout << "[ eXeTeL : Pug Services ]" << endl;
    cout << "Enter passphrase: ";
    cin.getline(password, 31, '\n');
    cout << endl;

    context.update((unsigned char*)password, strlen(password));
    context.finalize();

    if (!strcmp(context.hex_digest(), passphrase))
    {
        ins.open("setup.cfg");
        ins >> ircserver;
        ins >> ircport;
        ins >> ircnick;
        ins >> ircchannel;
        ins >> ircadminchannel;
        ins >> gameserver;
        ins >> gameport;
        ins >> rconpass;
        ins >> hltvport;
        ins >> hostserver;
        ins >> logport;
        ins.ignore();
        ins.getline(news, sizeof(news)-1, '\n');
        ins.close();

        PUGBOT* pugbot;
        pugbot = new PUGBOT(ircserver,ircport,ircnick,ircchannel,ircadminchannel,gameserver,gameport,rconpass,hltvport,hostserver,logport,news);
        pugbot->setup();
        while (pugbot->run());
        delete pugbot;
    }
    else
    {
        cout << "You are not authorised to run this service." << endl;
        exit(1);
    }

	return 0;
}
