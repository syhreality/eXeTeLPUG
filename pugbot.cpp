#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include "extras.h"
#include "pugbot.h"
using namespace std;

PUGBOT::PUGBOT(char pircserver[], unsigned int pircport, char pircnick[], char pircchannel[], char pircadminchannel[], char pgameserver[], unsigned int pgameport, char prconpass[], unsigned int phltvport, char phostserver[], unsigned int plogport, char pnews[])
{
	strncpy(IRC::ircserver, pircserver, sizeof(IRC::ircserver)-1);
	IRC::ircport = pircport;
    strncpy(IRC::circnick, pircnick, sizeof(IRC::circnick)-1);
    strncpy(IRC::ircnick, pircnick, sizeof(IRC::ircnick)-1);
    strncpy(IRC::ircchannel, pircchannel, sizeof(IRC::ircchannel)-1);
	strncpy(IRC::ircadminchannel, pircadminchannel, sizeof(IRC::ircadminchannel)-1);
    strncpy(gameserver, pgameserver, sizeof(gameserver)-1);
    gameport = pgameport;
    strncpy(rconpass, prconpass, sizeof(rconpass)-1);
    hltvport = phltvport;
    strncpy(hostserver, phostserver, sizeof(hostserver)-1);
    logport = plogport;
	strncpy(news, pnews, sizeof(news)-1);
    memset(kickedname, '\0', sizeof(kickedname));
    memset(leaverid, '\0', sizeof(leaverid));
    memset(leavername, '\0', sizeof(leavername));
	CT_total = 0;
	T_total = 0;
	MR = 15;
	status = 0;
	enabled = true;
	half = false;
	lo3 = false;
    working = false;
	outs.open("requests.log", ios::app);
}

PUGBOT::~PUGBOT()
{
	char buffer[256];

	sprintf(buffer,"logaddress_del %s %d",hostserver, logport);
	RCON::sendcmd(buffer);
	if (hltvport)
        system("killall -9 hltv &");

	outs.close();
	LOG::close();
	RCON::close();
	IRC::quit("15,1[ 11,1e9,1X11,1e9,1T11,1e9,1L G11,1ames 9,1N11,1etwork1,1 9,1 // 0,1 http://games.exetel.com.au 15,1]\n");
}

void PUGBOT::setup()
{
    char buffer[256];
	//admin setup
	ADMIN::loadData();
	printf(" * ADMIN Setup Complete\n");
	//pug setup
	PUG::SetServer(gameserver,gameport);
	PUG::SetChan(ircchannel);
	printf(" * PUG Setup Complete\n");
	//irc setup
	IRC::connect(IRC::ircnick, IRC::ircserver, IRC::ircport);
	IRC::private_message("ChanServ", "vhost exetel- staff_egc_admin");
	Sleep(3000);
	IRC::join(IRC::ircchannel); // join the pug channel
	IRC::join(IRC::ircadminchannel); // join the pug admin channel
	printf(" * IRC Setup Complete\n");
	MAP::loadData();
	printf(" * MAP Setup Complete\n");
	//log setup
	LOG::connect(logport);
	printf(" * LOG Setup Complete\n");
	//rcon setup
	RCON::connect(gameserver, gameport);
	RCON::challenge(rconpass);
	sprintf(buffer,"logaddress_del %s %d",hostserver,logport);
	RCON::sendcmd(buffer);
	sprintf(buffer,"logaddress_add %s %d",hostserver,logport);
	RCON::sendcmd(buffer);
	printf(" * RCON Setup Complete\n");
    WARNING::loadData();
    printf(" * WARNING Setup Complete\n");

	ADMIN::printData();
	working = true;

	return;
}

bool PUGBOT::run()
{
	int len; // length of returned buffer

	if (IRC::check())
	{
		// there is irc crap to process
		len = IRC::read(buffer);
		buffer[len] = '\0';
		IRC::process(buffer);
		memset(buffer, '\0', sizeof(buffer));
	}

	if (LOG::check())
	{
		// there is log crap to process
		len = LOG::read(buffer);
		buffer[len] = '\0';
		LOG::process(buffer);

		if (strstr(buffer, "Server say \"[ eXeTeL : Game is Live ! ]"))
	    {
		    lo3 = false;
		    live = true;
	    }
		else if (inprogress) // only parse connects/disconnects and scores when the pug is running.
			actions(buffer);
		memset(buffer, '\0', sizeof(buffer));
	}

	return working;
}

void PUGBOT::actions(char buffer[]) // called if the pug is inprogress
{
    char parse[128], trigstat[160];
    
    if (strstr(buffer, "connected, address \"") || strstr(buffer, ">\" disconnected") || strstr(buffer, ">\" was kicked by \"Console\"")) // Connection parser
    {
        LOG::getname(buffer, parse);
        if (strstr(buffer, "connected, address \"")) // is this case a connection?
        {
            sprintf(trigstat, "9%s connected", parse);
            IRC::send_channel(PUG::chan, trigstat);
        }
        else if (strstr(buffer, ">\" was kicked by \"Console\"")) // a kick?
        {
            strncpy(kickedname, parse, 31); // store the kicked person's name
            sprintf(trigstat, "7%s was kicked from the server", parse);
            IRC::send_channel(PUG::chan, trigstat);
        }
        else // or a disconnection?
        {
            if (strcmp(kickedname, parse)) // if the disconnection name is different to the kicked name
                memset(kickedname, '\0', sizeof(kickedname)); // nullify the contents
            LOG::getsteamid(buffer, leaverid); // store the leaver's steamid for !banleaver
            strncpy(leavername, parse, 31);
			sprintf(trigstat, "4%s disconnected", parse);
            IRC::send_channel(PUG::chan, trigstat);
        }
    }
	else if (live && (strstr(buffer, "Team \"CT\" triggered") || strstr(buffer, "Team \"TERRORIST\" triggered"))) // Event parser
	{
		char *scorescan;
		char team[16];
		char trigger[24];
		char sendstring[64];
		CTscore = 0;
		Tscore = 0;

		// 02:10:17 og L 10/12/2004 - 02:26:31: Team "CT" triggered "Target_Saved" (CT "1") (T "0")
		scorescan = strstr(buffer, "Team \"");
		sscanf(scorescan, "%*s %s %*s %s (CT \"%d\") (T \"%d\")", team, trigger, &CTscore, &Tscore);

		if (!strcmp(team, "\"CT\""))
			if (!strcmp(trigger, "\"Bomb_Defused\""))
				sprintf(trigstat, "10Counter-Terrorists Win! (Bomb Defusal)");
			else if (!strcmp(trigger, "\"Target_Saved\""))
				sprintf(trigstat, "10Counter-Terrorists Win! (Target Saved)");
			else
				sprintf(trigstat, "10Counter-Terrorists Win!");
		else
			if (!strcmp(trigger, "\"Target_Bombed\""))
				sprintf(trigstat, "4Terrorists Win! (Target Bombed)");
			else
				sprintf(trigstat, "4Terrorists Win!");

		IRC::send_channel(PUG::chan, trigstat);
		memset(trigstat, '\0', sizeof(trigstat));

		if (!half && CTscore + Tscore < MR) // first half
		{
			sprintf(trigstat, "10Counter-Terrorists %d - %d 4Terrorists", CTscore, Tscore);
			IRC::send_channel(PUG::chan, trigstat);
		}
		else if (!half && CTscore + Tscore == MR) // end of first half
		{
			if (CTscore - Tscore >= MR-4) // score is at least 10-2 (11-4)
			{
				sprintf(trigstat, "Half Time: 10Counter-Terrorists currently caining the 4Terrorists %d to %d!", CTscore, Tscore);
				IRC::send_channel(PUG::chan, trigstat);
				sprintf(sendstring, "say Half Time: Counter-Terrorists currently leading %d to %d.", CTscore, Tscore); 
			}
			else if (CTscore > Tscore) // more like 7-5 (9-6)
			{
				sprintf(trigstat, "Half Time: 10Counter-Terrorists currently beating the 4Terrorists %d to %d!", CTscore, Tscore);
				IRC::send_channel(PUG::chan, trigstat);
				sprintf(sendstring, "say Half Time: Counter-Terrorists currently leading %d to %d.", CTscore, Tscore); 
			}
			else if (Tscore - CTscore >= MR-4) // score is at least 10-2 (11-4)
			{
				sprintf(trigstat, "Half Time: 4Terrorists currently obliterating the 10Counter-Terrorists %d to %d!", Tscore, CTscore);
				IRC::send_channel(PUG::chan, trigstat);
				sprintf(sendstring, "say Half Time: Terrorists currently leading %d to %d.", Tscore, CTscore); 
			}
			else if (Tscore > CTscore) // more like 7-5
			{
				sprintf(trigstat, "Half Time: 4Terrorists currently beating the 10Counter-Terrorists %d to %d!", Tscore, CTscore);
				IRC::send_channel(PUG::chan, trigstat);
				sprintf(sendstring, "say Half Time: Terrorists currently leading %d to %d.", Tscore, CTscore); 
			}
			else // draw has occured. we dont need to check for MR15 because that cannot have a draw at half time
			{
				IRC::send_channel(PUG::chan, "Half Time: 10Counter-Terrorists and 4Terrorists are currently tied up at 6 - 6!");
				sprintf(sendstring, "say Half Time: CT's and T's currently tied up at 6 - 6"); 
			}

			CT_total = CTscore;
			T_total = Tscore;
			CTscore = 0;
			Tscore = 0;

			live = false;
			half = true; // we go into second half, dont want to send first half formatted information to irc.
			RCON::sendcmd(sendstring);
			memset(sendstring, '\0', sizeof(sendstring));
			RCON::sendcmd("say Swap sides and !lo3 to begin the second half.");
		}
		else if (half) //second half
		{
			if (CTscore + T_total == MR+1) // CT has won 13 rounds (or 16)
			{
				sprintf(trigstat, "Full Time: 10Counter-Terrorists win over 4Terrorists %d - %d!", CTscore + T_total, Tscore + CT_total);
				IRC::send_channel(PUG::chan, trigstat);
				sprintf(sendstring, "say Full Time: CT's win over the T's %d to %d.", CTscore + T_total, Tscore + CT_total);
				RCON::sendcmd(sendstring);
				endpug();
			}
			else if (Tscore + CT_total == MR+1) // T has won 13 rounds (or 16)
			{
				sprintf(trigstat, "Full Time: 4Terrorists win over 10Counter-Terrorists  %d - %d!", Tscore + CT_total, CTscore + T_total);
				IRC::send_channel(PUG::chan, trigstat);
				sprintf(sendstring, "say Full Time: T's win over the CT's %d to %d.", Tscore + CT_total, CTscore + T_total);
				RCON::sendcmd(sendstring);
				endpug();
			}
			else if (CTscore + Tscore == MR) // it's a full time draw!
			{
				sprintf(trigstat, "Full Time: 10Counter-Terrorists and 4Terrorists  have drawn at %d - %d!", Tscore + CT_total, CTscore + T_total);
				IRC::send_channel(PUG::chan, trigstat);
				sprintf(sendstring, "say Full Time: CT's have drawn with the T's %d to %d.", CTscore + T_total, Tscore + CT_total);
				RCON::sendcmd(sendstring);
				endpug();
			}
			else // a won round in the second half
			{
				sprintf(trigstat, "4Terrorists %d - %d 10Counter-Terrorists", Tscore + CT_total, CTscore + T_total);
				IRC::send_channel(PUG::chan, trigstat);
			}
			memset(sendstring, '\0', sizeof(sendstring));
		}
	}
	memset(trigstat, '\0', sizeof(trigstat));
}


void PUGBOT::ontext(char chan[],char nick[],char vhost[],char text[]) //chan + nick + text #2
{
	char buffer[512];
	char temp;
    char *ptr = text;

	if (strcmpi(PUG::chan, chan))
	{
		if (!strcmp(IRC::GetMyName(), chan)) //priv msg, log it
		{
			if (!strcmp(GODVHOST, vhost))
			{
				char *ptr = text;
				if (strstr(text, "!say"))
				{
					ptr += 5;
					IRC::send_channel(PUG::chan, ptr);
				}
				if (strstr(text, "!join"))
				{
					ptr += 6;
					IRC::join(ptr);
				}
				else if (strstr(text, "!leave"))
				{
					ptr += 7;
					IRC::leave(ptr);
				}
				else if (strstr(text, "!rdyup"))
				{
					ptr += 7;
					SetMode(IRC::ircchannel, nick, "+o");
				}
				else
					outs << nick << "@" << vhost << ": " << text << endl;
			}
			else
				outs << nick << "@" << vhost << ": " << text << endl;
		}
		return;
	}

	// ----- HEAD ADMIN COMMANDS -----
	if (!strcmp(vhost,GODVHOST))
	{
		STRCOMP(text,"!addadmin") // !addadmin name steamid
            char name[20], steamid[20];
            ptr += 10;
            sscanf(ptr, "%s %s", name, steamid);
			if (ADMIN::addAdmin(name, steamid))
				IRC::send_channel(PUG::chan, "The admin was added to the database successfully.");
			else
				IRC::send_channel(PUG::chan, "Add was unsuccessful. Correct syntax is !addadmin name steamid");
			return;
		ESTRCOMP(text,"!addadmin")

		STRCOMP(text,"!deladmin") // !deladmin name
            char name[20];
            ptr += 10;
            sscanf(ptr, "%s", name);
            if (ADMIN::delAdmin(name))
                IRC::send_channel(PUG::chan, "The admin was removed from the database successfully.");
            else if (!strncmp(name, "STEAM_", 6))
                IRC::send_channel(PUG::chan, "Delete was unsuccessful. Correct syntax is !deladmin name");
            else
                IRC::send_channel(PUG::chan, "Delete was unsuccessful. The admin does not exist in the database.");
			return;
		ESTRCOMP(text,"!deladmin")

		STRCOMP(text,"!addmap") // !addmap de_name
            char map[20];
            ptr += 8;
            sscanf(ptr, "%s", map);
			if (MAP::addMap(map))
				IRC::send_channel(PUG::chan, "The map was added to the database successfully.");
			else
				IRC::send_channel(PUG::chan, "Add was unsuccessful. Correct syntax is !addmap de_name");
			return;
		ESTRCOMP(text,"!addmap")

		STRCOMP(text,"!delmap") // !delmap de_name
            char map[20];
            ptr += 8;
            sscanf(ptr, "%s", map);
            if (MAP::delMap(map))
                IRC::send_channel(PUG::chan, "The map was removed from the database successfully.");
            else if (!strncmp(map, "de_", 3))
                IRC::send_channel(PUG::chan, "Delete was unsuccessful. Correct syntax is !delmap de_name");
            else
                IRC::send_channel(PUG::chan, "Delete was unsuccessful. The map does not exist in the database.");
			return;
		ESTRCOMP(text,"!delmap")

		STRCOMP(text,"!news") // !news newsgohere
			ofstream outs;

            if (ptr != NULL)
            {
                ptr += 6;
                if (strlen(ptr) > sizeof(news)-1)
                    ptr[sizeof(news)] = '\0';
                if (!status)
                    sprintf(buffer, BOT_BEGIN, gameserver, gameport, ptr);
                else if (status == 1)
                    sprintf(buffer, BOT_DIFF, gameserver, gameport, ptr);
                else if (status == 2)
                    sprintf(buffer, BOT_INGAME, gameserver, gameport, ptr, PUG::GetMap(), Players[0].playername);
                else if (status == 3)
                    sprintf(buffer, BOT_START, gameserver, gameport, ptr);

                IRC::set_topic(PUG::chan, buffer);
                strncpy(news, ptr, sizeof(news)-1);

                outs.open("setup.cfg");
                outs << IRC::ircserver << endl;
                outs << IRC::ircport << endl;
                outs << IRC::ircnick << endl;
                outs << IRC::ircchannel << endl;
                outs << IRC::ircadminchannel << endl;
                outs << gameserver << endl;
                outs << gameport << endl;
                outs << rconpass << endl;
                outs << hltvport << endl;
                outs << hostserver << endl;
                outs << logport << endl;
                outs << news << endl;
                outs.close();
            }
			return;
		ESTRCOMP(text,"!news")

		STRCOMP(text,"!qtab")
			SetModes(IRC::ircchannel, "-v");
			working = false;
			return;
		ESTRCOMP(text,"!qtab")
	}

	// ----- ADMIN COMMANDS -----
	if (!strcmp(vhost,ADMINVHOST) || !strcmp(vhost,GODVHOST))
	{
		STRCOMP(text, "!admins")
			ADMIN::printData(buffer);
            IRC::send_notice(nick, buffer);
			return;
		ESTRCOMP(text, "!admins")

		STRCOMP(text,"!chat")
			ptr += 6;
			sprintf(buffer,"say %s: %s",nick,ptr);
			RCON::sendcmd(buffer);
			return;
		ESTRCOMP(text,"!chat")

		STRCOMP(text,"!details")
            char details[256];
            if (!inprogress)
            {
		        strcpy(details,PUG_NOGAME);
		        IRC::send_notice(nick, details);
		    }    
	        else
	        {
		        sprintf(details, PUG_INFO, ip, port, password);
		        IRC::send_notice(nick, details);
 				memset(details, '\0', sizeof(details));
				sprintf(details, PUG_ADMIN, adminpassword);
				IRC::send_notice(nick, details);
            }
            return;
		ESTRCOMP(text,"!details")

		STRCOMP(text,"!killpug")
			endpug();
			return;
		ESTRCOMP(text,"!killpug")

		STRCOMP(text,"!password")
			char modpass[6];
			PUG::ModifyPassword(modpass, nick);
			sprintf(buffer, "sv_password %s", modpass);
			RCON::sendcmd(buffer);
			memset(buffer, '\0', sizeof(buffer));
			sprintf(buffer, "say Password has been changed to \"%s\" on admin request.", modpass);
			RCON::sendcmd(buffer);
			return;
		ESTRCOMP(text,"!password")

        STRCOMP(text,"!rechallenge")
        	RCON::connect(gameserver, gameport);
        	RCON::challenge(rconpass);
        	sprintf(buffer,"logaddress_del %s %d",hostserver,logport);
        	RCON::sendcmd(buffer);
        	sprintf(buffer,"logaddress_add %s %d",hostserver,logport);
        	RCON::sendcmd(buffer);
            IRC::send_channel(PUG::chan, "Re-syncing with server complete.");
        ESTRCOMP(text,"!rechallenge")

        STRCOMP(text,"!banleaver")
            if (!banleaver())
                IRC::send_channel(PUG::chan, "The ban failed due to nobody having left the PUG.");
            return;
        ESTRCOMP(text,"!banleaver")

        STRCOMP(text,"!bl")
            if (!banleaver())
                IRC::send_channel(PUG::chan, "The ban failed due to nobody having left the PUG.");
            return;
        ESTRCOMP(text,"!bl")

        STRCOMP(text,"!ban")
            char ban[128], tid[16], steamid[32];
            int length, ret;

            ptr += 5;
            ret = sscanf(ptr, "%s %s", steamid, tid);

            if (!strncmp(steamid, "STEAM_", 6) && ret == 2)
            {
                WARNING::updateData(steamid, "IRC Banned");
                length = atoi(tid);
                sprintf(ban, "banid %d %s kick", length, steamid);
                RCON::sendcmd(ban);
                RCON::sendcmd("writeid");
                RCON::sendcmd("exec banned.cfg");
                if (!length)
                    sprintf(ban, "%s was successfully permanently banned.", steamid);
                else
                    sprintf(ban, "%s was successfully banned for %d minutes.", steamid, length);
                IRC::send_channel(PUG::chan, ban);
            }
            else
                IRC::send_channel(PUG::chan, "Ban was unsuccessful. Correct syntax is !ban steamid minutes");
			return;
        ESTRCOMP(text,"!ban")

		STRCOMP(text,"!unban")
			ptr += 7;
            if (WARNING::removeData(ptr))
            {
                sprintf(buffer, "removeid %s", ptr);
                RCON::sendcmd(buffer);
                RCON::sendcmd("writeid");
                RCON::sendcmd("exec banned.cfg");
                IRC::send_channel(PUG::chan, "STEAMID unbanned successfully.");
            }
            else
                IRC::send_channel(PUG::chan, "The STEAMID was not found in the database.");
			return;
		ESTRCOMP(text,"!unban")

		STRCOMP(text,"!enable")
			enabled = true;
			sprintf(buffer, BOT_BEGIN, gameserver, gameport, news);
			status = 0;
			IRC::set_topic(PUG::chan, buffer);
			return;
		ESTRCOMP(text,"!enable")

		STRCOMP(text,"!disable")
			enabled = false;
			sprintf(buffer, BOT_DIFF, gameserver, gameport, news);
			IRC::set_topic(PUG::chan, buffer);
			status = 1;
			PUG::KillPug();
			return;
		ESTRCOMP(text,"!disable")
	}

	if (!enabled)
		return; //disabled hence ignore

	// ----- NORMAL COMMANDS -----
	STRCOMP(text,"!pug")
		char *mapptr = NULL;
		if (!strcmpi(text, "!pug"))
			mapptr = stristr(text, "!pug") + 5; //extract start of map
		if (!PUG::mapset)
		{
			PUG::SetMap(mapptr); // set the map
			sprintf(buffer, BOT_START, gameserver, gameport, news);
			IRC::set_topic(PUG::chan, buffer);
			status = 3;
			PUG::AddPlayer(nick);
		}
		return;
	ESTRCOMP(text,"!pug")

	STRCOMP(text,"!join")
		char level[80];
		char *ptr = PUG::AddPlayer(nick);
		if (!strcmp(ptr, "START"))
		{
			char topic[512];

			SetModes(PUG::chan, "+v"); // voice the players in the channel

            sprintf(buffer, "%s[MR15]", IRC::ircnick);
            IRC::change_nick(buffer);
            strncpy(IRC::circnick, buffer, sizeof(IRC::circnick)-1);
            memset(buffer, '\0', sizeof(buffer));

			if (MAP::found(PUG::GetMap()))
			{
				sprintf(buffer, "map %s", PUG::GetMap());
				RCON::sendcmd(buffer);
				sprintf(buffer, BOT_INGAME, gameserver, gameport, news, PUG::GetMap(), Players[0].playername);
				sprintf(level, PUG_START, PUG::GetMap());
			}
			else
			{
				int x = rand() % 4;
				if (!x)
				{
					RCON::sendcmd("map de_dust2");
					sprintf(buffer, BOT_INGAME, gameserver, gameport, news, "de_dust2", Players[0].playername);
					sprintf(level, PUG_START, "de_dust2");
					PUG::SetMap("de_dust2");
				}
				else if (x == 1)
				{
					RCON::sendcmd("map de_inferno");
					sprintf(buffer, BOT_INGAME, gameserver, gameport, news, "de_inferno", Players[0].playername);
					sprintf(level, PUG_START, "de_inferno");
					PUG::SetMap("de_inferno");
				}
				else if (x == 2)
				{
					RCON::sendcmd("map de_nuke");
					sprintf(buffer, BOT_INGAME, gameserver, gameport, news, "de_nuke", Players[0].playername);
					sprintf(level, PUG_START, "de_nuke");
					PUG::SetMap("de_nuke");
				}
				else if (x == 3)
				{
					RCON::sendcmd("map de_train");
					sprintf(buffer, BOT_INGAME, gameserver, gameport, news, "de_train", Players[0].playername);
					sprintf(level, PUG_START, "de_train");
					PUG::SetMap("de_train");
				}
			}

			IRC::set_topic(PUG::chan, buffer);
			PUG::MakeTeams();
			printteams();
			PUG::StartPug();
            if (hltvport)
            {
                sprintf(buffer, "./hltv -port %d +connect %s:%d +serverpassword %s &", hltvport, gameserver, gameport, PUG::password);
                system(buffer);
            }
			memset(buffer, '\0', sizeof(buffer));
			IRC::send_channel(PUG::chan, level);
			status = 2;
		}
		return;
	ESTRCOMP(text,"!join")

	STRCOMP(text,"!leave")
		if (PUG::RemovePlayer(nick) && !mapset)
		{
			sprintf(buffer, BOT_BEGIN, gameserver, gameport, news);
			IRC::set_topic(PUG::chan, buffer);
			status = 0;
		}
		return;
	ESTRCOMP(text,"!leave")

	STRCOMP(text,"!players")
		char status[512];
		PUG::Status(status);
		IRC::send_channel(PUG::chan,status);
		return;
	ESTRCOMP(text,"!players")

	STRCOMP(text,"!endpug")
		if (!strcmp(nick, Players[0].playername))
			endpug();
		else
			IRC::send_notice(nick, PUG_NONADMIN);
		return;
	ESTRCOMP(text,"!endpug")

	STRCOMP(text,"!details")
		char details[256];
		if ((PUG::HasPlayer(nick) && inprogress) || !inprogress)
		{
			PUG::Details(details);
			IRC::send_notice(nick, details);
			if (!strcmp(nick, Players[0].playername))
			{
				memset(details, '\0', sizeof(details));
				sprintf(details, PUG_ADMIN, adminpassword);
				IRC::send_notice(nick, details);
			}
		}
		return;
	ESTRCOMP(text,"!details");

	STRCOMP(text,"!maps")
		MAP::printData(buffer);
        IRC::send_notice(nick, buffer);
		return;
	ESTRCOMP(text,"!maps");

	STRCOMP(text,"!about")
		IRC::private_message(nick, "9,1e0,1X9,1e0,1T9,1e0,1L Gaming Community Pugbot Project - Coded by Surreality and Vanja");
		return;
	ESTRCOMP(text,"!about");
}


void PUGBOT::endpug()
{
	PUG::inprogress = false; // set inprogress to false to stop disconnects being sent to channel
	RCON::sendcmd("say [ eXeTeL : Pug has been ended ! ]");
	RCON::sendcmd("say Thank you for supporting #eXeTeLPUG !");
	RCON::sendcmd("sv_password Vyj5dzow");
    if (hltvport)
    {
        system("killall -9 hltv &");
        system("gzip cstrike/*.dem &"); // compress the demo
        system("mv cstrike/*.dem.gz ../demos &"); // move the compressed file to the public dir
    }
    Sleep(3000);
	RCON::sendcmd("map de_dust2"); // change map and kick everybody

    memset(kickedname, '\0', sizeof(kickedname));
    memset(leaverid, '\0', sizeof(leaverid)); // remove last leaver for next pug
    memset(leavername, '\0', sizeof(leavername));
	status = 0;
	adminloggedin = false;
	half = false;
	live = false;
    lo3 = false;
	MR = 15;
	CTscore = 0;
	Tscore = 0;
	CT_total = 0;
	T_total = 0;

	PUG::KillPug();
	IRC::send_channel(PUG::chan, PUG_END);
    IRC::change_nick(IRC::ircnick);
    strncpy(IRC::circnick, IRC::ircnick, sizeof(IRC::circnick)-1);
	sprintf(buffer, BOT_BEGIN, gameserver, gameport, news);
	IRC::set_topic(PUG::chan, buffer);

	return;
}

void PUGBOT::printteams()
{
	strcpy(CT,"10CT: ");
	strcpy(T,"4T: ");
	PUG::AppendTeams(CT,T);
	IRC::send_channel(PUG::chan, CT);
	IRC::send_channel(PUG::chan, T);
	memset(CT, '\0', sizeof(CT));
	memset(T, '\0', sizeof(T));

	return;
}

void PUGBOT::SetMode(char chan[],char nick[], char mode[])
{
	char buffer[128];
	sprintf(buffer, "MODE %s %s %s\n", chan, mode, nick);
	IRC::raw_command(buffer);

	return;
}

void PUGBOT::SetModes(char chan[], char mode[])
{
	char buffer[256];
	char *ptr = &mode[1];

	sprintf(buffer, "MODE %s %s", chan, mode);
	for (int i = 0; i < 4; i++)
		strcat(buffer, ptr);
	strcat(buffer, " ");

	for (int i = 0; i < 5; i++)
	{
		strcat(buffer, Players[i].playername);
		strcat(buffer, " ");
	}
	strcat(buffer, "\n");
	IRC::raw_command(buffer);

	memset(buffer, '\0', sizeof(buffer));
	sprintf(buffer, "MODE %s %s", chan, mode);
	for (int i = 0; i < 4; i++)
		strcat(buffer, ptr);
	strcat(buffer, " ");

	for (int i = 5; i < 10; i++)
	{
		strcat(buffer, Players[i].playername);
		strcat(buffer, " ");
	}
	strcat(buffer, "\n");
	IRC::raw_command(buffer);

	return;
}

void PUGBOT::onstartpug(char adminpass[],char serverpass[])
{
	char buff[128];
	char map[32];

	adminloggedin = false;
	strcpy(adminpassword, adminpass);

	sprintf(buff, "sv_password %s", serverpass);
	RCON::sendcmd(buff);

	// set the map
	strncpy(map, PUG::GetMap(), sizeof(map));

	return;
}

//irc shit
void PUGBOT::onpart(char pnick[]) //chan + nick
{
	if (PUG::RemovePlayer(pnick) && !PUG::pcount)
	{
		sprintf(buffer, BOT_BEGIN, gameserver, gameport, news);
		IRC::set_topic(PUG::chan, buffer);
		status = 0;
	}
	return;
}

void PUGBOT::onjoin(char chan[], char nick[])
{
	IRC::send_notice(nick, NOTICEMSG);
	return;
}

void PUGBOT::onnamechange(char oldname[],char newname[]) //chan + nick
{
	PUG::ProcessNameChange(oldname, newname);
	return;
}

void PUGBOT::SendNotice(char nick[], char text[])
{
	//send notice to nick with contents thatext
	char notice[256];
	sprintf(notice,"NOTICE %s : %s\n",nick, text);
	IRC::raw_command(notice);
	return;
}

//rcon shit
void PUGBOT::ingamesay(char steamid[], char text[])
{
	char logincmd[32], message[1024];

	sprintf(logincmd,"!login %s",adminpassword); // password = !login adminpass
	if (strstr(text, logincmd))
	{
		if (!adminloggedin)
		{
			strcpy(adminsteam, steamid);
			RCON::sendcmd("say Admin logged on successfully.");
			adminloggedin = true;
		}
		return;
	}
	else if (stristr(text, "!about"))
		RCON::sendcmd("say eXeTeL Games Pugbot Project - Coded by Surreality and Vanja.");
	else if (stristr(text, "!score"))
	{
		if (!half)
		{
			sprintf(message, "say Counter-Terrorists %d - %d Terrorists", CTscore, Tscore);
			RCON::sendcmd(message);
		}
		else
		{
			sprintf(message, "say Terrorists %d - %d Counter-Terrorists", CT_total + Tscore, T_total + CTscore);
			RCON::sendcmd(message);
		}
	}
	else if (stristr(text, "!teams"))
	{
		strcpy(CT, "say CT: ");
		strcpy(T, "say T: ");
		PUG::AppendTeams(CT,T);
		RCON::sendcmd(CT);
		RCON::sendcmd(T);
		memset(CT, '\0', sizeof(CT));
		memset(T, '\0', sizeof(T));
	}
	else if (!strcmp(steamid,adminsteam) || ADMIN::found(steamid))
	{
		char *tmp;
		if (stristr(text, "!lo3") && !lo3)
		{
			lo3 = true;
			RCON::liveonthree();
			IRC::send_channel(PUG::chan, "The game has gone live!");
			IRC::send_channel(ircadminchannel, "The game has gone live!");
		}
		else if (stristr(text, "!endpug"))
			endpug();
		else if (stristr(text, "!restart"))
			RCON::sendcmd("sv_restart 1");
		else if (stristr(text, "!changelevel") || stristr(text, "!map"))
			changelevel(text);
		else if (stristr(text, "!at 0"))
		{
			RCON::sendcmd("sv_alltalk 0");
			RCON::sendcmd("say sv_alltalk set to 0.");
		}
		else if (stristr(text, "!at 1"))
		{
			RCON::sendcmd("sv_alltalk 1");
			RCON::sendcmd("say sv_alltalk set to 1.");
		}
		else if (stristr(text, "!ff 0"))
		{
			RCON::sendcmd("mp_friendlyfire 0");
			RCON::sendcmd("say mp_friendlyfire set to 0.");
		}
		else if (stristr(text, "!ff 1"))
		{
			RCON::sendcmd("mp_friendlyfire 1");
			RCON::sendcmd("say mp_friendlyfire set to 1.");
		}
		else if (stristr(text, "!ft 0"))
		{
			RCON::sendcmd("mp_freezetime 0");
			RCON::sendcmd("say mp_freezetime set to 0.");
		}
		else if (stristr(text, "!ft 1"))
		{
			RCON::sendcmd("mp_freezetime 7");
			RCON::sendcmd("say mp_freezetime set to 7.");
		}
		else if (stristr(text, "!mr12"))
		{
			if (!live && !half)
			{
                MR = 12;
                sprintf(message, "%s[MR12]", IRC::ircnick);
                IRC::change_nick(message);
                strncpy(IRC::circnick, message, sizeof(IRC::circnick)-1);
                RCON::sendcmd("say MR12 Ruleset Applied.");
				IRC::send_channel(PUG::chan, "MR12 Ruleset Applied.");
			}
			else
				RCON::sendcmd("say Cannot alter ruleset whilst live.");
		}
		else if (stristr(text, "!mr15"))
		{
			if (!live && !half)
			{
                MR = 15;
                sprintf(message, "%s[MR15]", IRC::ircnick);
                IRC::change_nick(message);
                strncpy(IRC::circnick, message, sizeof(IRC::circnick)-1);
                RCON::sendcmd("say MR15 Ruleset Applied.");
				IRC::send_channel(PUG::chan, "MR15 Ruleset Applied.");
			}
			else
				RCON::sendcmd("say Cannot alter ruleset whilst live.");
		}
		else if (stristr(text, "!get"))
		{
            RCON::sendcmd("say [ eXeTeL : Getting an Extra Player ]");
			if (PUG::password)
				sprintf(message, PUG_GET, gameserver, gameport, PUG::password, PUG::password);
			else
				sprintf(message, PUG_GET, gameserver, gameport, "", "<none>");

			IRC::send_channel(PUG::chan, message);
		}
		else if (stristr(text, "!replace"))
		{
            RCON::sendcmd("say [ eXeTeL : Getting a Replacement Player ]");
			if (PUG::password)
				sprintf(message, PUG_REPLACE, gameserver, gameport, PUG::password, PUG::password);
			else
				sprintf(message, PUG_REPLACE, gameserver, gameport, "", "<none>");

			IRC::send_channel(PUG::chan, message);
		}
		else if ((tmp = stristr(text, "!irc")))
		{
			char sendtoirc[128];
			sprintf(sendtoirc, "Ingame: %s", tmp+5);
			IRC::send_channel(PUG::chan, sendtoirc);
		}
		else if ((tmp = stristr(text, "!kick ")))
		{
            char kick[32];

            if (tmp != NULL) // prevent !kick without arg crashing bot
            {
                tmp += 6;
                if (tmp[0] == '#') // !kick #1337
                    sprintf(kick, "kick %s", tmp);
                else // !kick "Surreality"
                    sprintf(kick, "kick \"%s\"", tmp);
                RCON::sendcmd(kick);
            }
		}
        else if (stristr(text, "!banleaver") || stristr(text, "!bl")) // stops multiple !banleavers on one person
            banleaver();
		else if ((tmp = stristr(text, "!password")))
		{
			string pass = "sv_password ";
			tmp += 10;
			pass += tmp;
			RCON::sendcmd(pass.c_str());
			pass.erase();
			pass = "say Password has been changed to \"";
			pass += tmp;
			pass += "\" successfully.";
			RCON::sendcmd(pass.c_str());
			strncpy(PUG::password, tmp, sizeof(PUG::password)-1);
		}
        else if (stristr(text, "thetime"))
        {
            string time = "say ";
            thetime(message);
            time += message;
            RCON::sendcmd(time.c_str());
        }
	}
}

bool PUGBOT::banleaver()
{
    char ban[128];
    int warns;

    if (leaverid[0] == '\0')
        return false;

    if (!strcmp(kickedname, leavername)) // check for kicked then !banleaver
    {
        RCON::sendcmd("say Unable to ban a player kicked from the game.");
        return true;
    }

    warns = WARNING::updateData(leaverid, leavername);

    if (ADMIN::found(leaverid)) // check that it wasn't an admin
        RCON::sendcmd("say You cannot ban an administrator.");
    else
    {
        switch (warns)
        {
            case 0:
                break;
            case 1:
                sprintf(ban, "Warning: %s (%s) you will be banned if you leave another pug.", leavername, leaverid);
                break;
            case 2:
                sprintf(ban, "banid 180 %s kick", leaverid);
                RCON::sendcmd(ban);
                sprintf(ban, "Banning: %s (%s) for leaving the pug - 3 hours.", leavername, leaverid);
                break;
            case 3:
                sprintf(ban, "banid 1440 %s kick", leaverid);
                RCON::sendcmd(ban);
                sprintf(ban, "Banning: %s (%s) for leaving the pug - 1 day.", leavername, leaverid);
                break;
            case 4:
                sprintf(ban, "banid 4320 %s kick", leaverid);
                RCON::sendcmd(ban);
                sprintf(ban, "Banning: %s (%s) for leaving the pug - 3 days.", leavername, leaverid);
                break;
            case 5:
                sprintf(ban, "banid 10080 %s kick", leaverid);
                RCON::sendcmd(ban);
                sprintf(ban, "Banning: %s (%s) for leaving the pug - 1 week.", leavername, leaverid);
                break;
            default:
                sprintf(ban, "banid 0 %s kick", leaverid);
                RCON::sendcmd(ban);
                sprintf(ban, "Banning: %s (%s) is permanently banned for leaving pugs.", leavername, leaverid);
                break;
        }
        if (warns)
        {
            IRC::send_channel(PUG::chan, ban);
            RCON::sendcmd("writeid");
            RCON::sendcmd("exec banned.cfg");
        }
    }
    memset(leaverid, '\0', sizeof(leaverid)); // nullify!
    memset(leavername, '\0', sizeof(leavername));

    return true;
}

void PUGBOT::changelevel(char buffer[])
{
	char *ptr = strstr(buffer, "!changelevel");
	if (ptr == 0)
		ptr = strstr(buffer, "!map");

    char level[32];
    int j = 0;
	if (ptr != 0)
	{
		while (*ptr != ' ')
			*ptr++;
		*ptr++;
	}
	else
		ptr = buffer;

    while (isalnum(ptr[j]) || ptr[j] == '_')
    {
        level[j] = ptr[j];
        j++;
    }
    level[j] = '\0';

	if (MAP::found(level))
	{
		char change[64];
		char topic[512];
		half = false;
		CTscore = 0;
		Tscore = 0;
		CT_total = 0;
		T_total = 0;
		sprintf(change, "changelevel %s", level);
		RCON::sendcmd(change);
		sprintf(topic, BOT_INGAME, gameserver, gameport, news, level, Players[0].playername);
		IRC::set_topic(PUG::chan, topic);
	}
	else
		RCON::sendcmd("Invalid map selected.");

	return;
}
