#ifndef PUGBOT_H
#define PUGBOT_H
#include <fstream>
#include "socket.h"
#include "admin.h"
#include "config.h"
#include "irc.h"
#include "log.h"
#include "map.h"
#include "rcon.h"
#include "pug.h"
#include "warning.h"
using namespace std;

#define ADMINVHOST "eXeTeL.on.EnterTheGame.Com"
#define GODVHOST "104.4.233.220.exetel.com.au"
#define NOTICEMSG "15,1[ 11,1e9,1X11,1e9,1T11,1e9,1L G11,1ames 9,1C11,1ommunity1,1 9,1 // 0,1 http://games.exetel.com.au 15,1]"
#define STRCOMP(a,b) temp = a[strlen(b)]; a[strlen(b)] = '\0'; if(strcmpi(a,b)==0) {
#define ESTRCOMP(a,b) } a[strlen(b)] = temp;

class PUGBOT : ADMIN, IRC, LOG, MAP, RCON, PUG, WARNING
{
    public:
        PUGBOT(char pircserver[], unsigned int pircport, char pircnick[], char pircchannel[], char pircadminchannel[], char pgameserver[], unsigned int pgameport, char prconpass[], unsigned int phltvport, char phostserver[], unsigned int plogport, char news[]);
		~PUGBOT();

		//helper functions
        void setup();
		bool run();
		void actions(char[]);
		void printteams();
		void endpug();

       	//irc fill in
		void ontext(char chan[],char nick[],char vhost[],char text[]);
		void onpart(char pnick[]);
		void onjoin(char chan[],char nick[]);
		void onnamechange(char old[],char noo[]);

		//pug fill in
		void SetMode(char chan[],char nick[],char mode[]);
		void SetModes(char chan[],char mode[]);
		void SendNotice(char nick[],char thatext[]);
		void onstartpug(char adminpass[],char serverpass[]);

		//log fill in
		void ingamesay(char steamid[], char text[]);
        bool banleaver();
		void changelevel(char[]);

    private:
		bool enabled, half, lo3, working;
        char buffer[2048];
        char gameserver[32], rconpass[16], hostserver[32], news[80], CT[128], T[128];
        char kickedname[32], leaverid[20], leavername[32];
        int ircport, gameport, hltvport, logport, MR, CTscore, Tscore, CT_total, T_total, status;
		ofstream outs;

		//ADMIN STUFF
		bool adminloggedin;
		char adminsteam[32];
		char adminpassword[6];
};

#define BOT_BEGIN "0,1[ 9,1eXeTeL0,1PUG 9,1] 14,1-0,1 [ 9,1Server 14,1:0,1 %s:%d 9,1] 14,1- 0,1[ 9,1News 14,1: 0,1%s 9,1] 14,1- 0,1[ 9,1Status 14,1: 0,1Type !pug <level> to start a game. 9,1]"
#define BOT_DIFF "0,1[ 9,1eXeTeL0,1PUG 9,1] 14,1-0,1 [ 9,1Server 14,1:0,1 %s:%d 9,1] 14,1- 0,1[ 9,1News 14,1: 0,1%s 9,1] 14,1- 0,1[ 9,1Status 14,1: 0,1Bot is disabled due to technical difficulties. 9,1]"
#define BOT_INGAME "0,1[ 9,1eXeTeL0,1PUG 9,1] 14,1-0,1 [ 9,1Server 14,1:0,1 %s:%d 9,1] 14,1- 0,1[ 9,1News 14,1: 0,1%s 9,1] 14,1- 0,1[ 9,1Status 14,1: 0,1Ingame on %s, administered by %s. 9,1]"
#define BOT_START "0,1[ 9,1eXeTeL0,1PUG 9,1] 14,1-0,1 [ 9,1Server 14,1:0,1 %s:%d 9,1] 14,1- 0,1[ 9,1News 14,1: 0,1%s 9,1] 14,1- 0,1[ 9,1Status 14,1: 0,1A pug has been started. Type !join to play. 9,1]"

#endif
