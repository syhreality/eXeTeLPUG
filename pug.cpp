#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include "pug.h"
using namespace std;

PUG::PUG()
{
	for (int i = 0;i < 10;i++)
		Players[i].taken = false;
	inprogress = false;
	mapset = false;
	pcount = 0;
}

void PUG::SetChan(char pchan[])
{
	strcpy(chan, pchan);
	return;
}

void PUG::SetServer(char serverip[], unsigned long pport)
{
	strncpy(ip, serverip, sizeof(ip)-1);
	port = pport;
	return;
}

void PUG::MakeTeams()
{
	bool done = false;
	int count = 0;

	for (int i = 0;i < 10;i++)
		Players[i].team = false;
	while (!done)
	{
	//hand out 5 trues
		if (count == 5)
			done = true;
		else
		{
			int curr = (int)(rand()%10);
			if (Players[curr].team != true)
			{
				Players[curr].team = true;
				count++;
			}
		}
	}
	return;
}

void PUG::AppendTeams(char CT[], char T[])
{
	int j = 0, k = 0;
	for (int i = 0; i < 10; i++)
	{
		if (Players[i].team == true)
		{
			j++;
			strncat(CT,Players[i].playername,sizeof(Players[i].playername));
			if (j != 5)
				strcat(CT, ", ");
			else
				strcat(CT, ".");
		}
		else
		{
			k++;
			strncat(T,Players[i].playername,sizeof(Players[i].playername));
			if (k != 5)
				strcat(T, ", ");
			else
				strcat(T, ".");
		}
	}
}

int PUG::FindPlayer()
{
	for (int i = 0; i < 10; i++)
		if (Players[i].taken == false)
			return(i);
	return -1; //all slots taken
}

char *PUG::AddPlayer(char *nick)
{
	int num;

	if (pcount > 10) // quick hack to fix bad pcount values
		pcount = 0;

	if (pcount < 10)
	{
		if (inprogress)
			return(PUG_INPROGRESS);
		if (!mapset)
			return(PUG_EMPTY);
		if ((num = FindPlayer()) == -1)
			return false;

		for (int i = 0; i < 10; i++)
			if (!strcmp(nick,Players[i].playername))
				return("X");

		//SetMode(chan, nick, "+v");
		Players[num].taken = true;
		strncpy(Players[num].playername, nick, sizeof(Players[num].playername)-1);
		pcount++;

		if (pcount == PLAYERSIZE) // if there are 10 people ready, start the pug
			return("START");
	}
	else
		return(PUG_FULL);

	return("X");
}

bool PUG::RemovePlayer(char *nick)
{
	if (!inprogress) // not allowed to leave if live
	{
		if (!strcmp(nick, Players[0].playername)) // admin left game
		{
			//SetModes(chan, "-v");
			pcount = 0;
			mapset = false;
			for (int i = 0; i < 10; i++)
			{
				Players[i].taken = false;
				memset(Players[i].playername, '\0', sizeof(Players[i].playername));
			}
			return true;
		}
		else
		{
			for (int i = 0; i < 10; i++)
			{
				if (!strcmp(nick, Players[i].playername)) //names match
				{
					memset(Players[i].playername, '\0', sizeof(Players[i].playername));
					Players[i].taken = false;
					pcount--;
					//SetMode(chan, nick, "-v");
					if (pcount == 0)
						mapset = false;
					for(int j = 1; j < 9; j++)
					{
						if (Players[j].taken == true)
							continue;
						int temp = j+1;
						while (Players[temp].taken == false && temp < 10)
							temp++;
						if (temp >= 10)
							break;
						else
						{
							Players[j].taken = true;
							strcpy(Players[j].playername,Players[temp].playername);
							Players[temp].taken = false;
							memset(Players[temp].playername, '\0', sizeof(Players[temp].playername));
						}
					}
					return true;
				}
			}
		}
	}
	return false;
}

void PUG::ProcessNameChange(char old[], char noo[])
{
	for (int i = 0; i < 10; i++)
	{
		if (strcmp(old,Players[i].playername) == 0)
		{
			strcpy(Players[i].playername, noo);
			return;
		}
	}
	return;
}

void PUG::SetPasswords()
{
    int x, y;
    ifstream ins;

    x = int(50.0 * rand()/(RAND_MAX+1.0)) + 1;
    y = int(50.0 * rand()/(RAND_MAX+1.0)) + 1;

    if (x == y)
        y = x + 1;

    ins.open("words.txt");
    for (int i = 1; i < x; i++)
        ins >> password;
    ins.close();

    ins.open("words.txt");
    for (int i = 1; i < y; i++)
        ins >> adminpassword;
    ins.close();

	return;
}

void PUG::ModifyPassword(char modpass[], char nick[])
{
	char buffer[128];
	int x;
	ifstream ins;

	x = int(50.0 * rand()/(RAND_MAX+1.0)) + 1;
	
	ins.open("words.txt");
	for (int i = 1; i < x; i++)
		ins >> modpass;
	ins.close();

	sprintf(buffer, PUG_SET, modpass);
	SendNotice(nick, buffer);

	strncpy(password, modpass, sizeof(password)-1);

	return;
}

void PUG::StartPug()
{
	char buff[512];
	char send[32];

	inprogress = true;
	memset(adminpassword, '\0', sizeof(adminpassword));
	SetPasswords();
	onstartpug(adminpassword, password);

    sprintf(buff, PUG_INFO, ip, port, password);
    sprintf(send, "+%s", chan);
    SendNotice(send, buff);
    sprintf(buff,PUG_ADMIN,adminpassword);
    SendNotice(Players[0].playername, buff);
    
	return;
}

bool PUG::HasPlayer(char nick[])
{
	for (int i = 0; i< 10;i++)
		if (!strcmpi(Players[i].playername,nick))
			return true;
	return false;
}

void PUG::KillPug()
{
	SetModes(chan, "-v");
	for (int i = 0; i < 10; i++)
	{
		Players[i].taken = false;
		memset(Players[i].playername, '\0', sizeof(Players[i].playername));
	}
    pcount = 0;
	inprogress = false;
	mapset = false;

	return;
}

void PUG::Details(char *dets)
{
	if (!inprogress)
		strcpy(dets,PUG_NOGAME);
	else
		sprintf(dets,PUG_INFO,ip,port,password);

	return;
}

void PUG::Status(char status[])
{
	if (pcount == 0)
		sprintf(status, PUG_NOGAME);
    else
    {
		if (pcount != 10)
			sprintf(status, "(4%d/910) Players: ", pcount);
		else
			sprintf(status, "(9%d/910) Players: ", pcount);

	    for (int i = 0; i < pcount; i++)
        {
            if (Players[i].taken)
			{
                strncat(status, Players[i].playername, sizeof(Players[i].playername));
				if (i != pcount-1)
					strcat(status, ", ");
				else
					strcat(status, ".");
			}
	    }
		strcat(status, "\n");
    }

    return;
}
