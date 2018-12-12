#ifndef PUG_H
#define PUG_H
#include "config.h"

struct player
{
  bool taken;
  char playername[32];
  bool team; // false = team one, true = team two
};

class PUG
{
	public:
		PUG();
		void SetChan(char pchan[]);
		void SetServer(char serverip[], unsigned long pport);
		void SetPasswords();
		void ModifyPassword(char[],char[]);
		bool HasPlayer(char[]);
		void Details(char *dets);
		void SetMap(char pmap[]) { strncpy(map,pmap,sizeof(map)-1); mapset = true; return; } // should extract the real map name somehow, ie. stop at space
		char *GetMap() { return(map); }
		int FindPlayer();
		char *AddPlayer(char *nick);
		bool RemovePlayer(char *nick);
		void ProcessNameChange(char oldname[], char newname[]);
		void Status(char[]);
		void KillPug();
		void StartPug();
		void MakeTeams();
		void AppendTeams(char CT[], char T[]);
		
		virtual void SendNotice(char nick[], char text[]) { return; } //nick + text
		virtual void SetMode(char chan[],char nick[],char mode[]) { return; }
		virtual void SetModes(char chan[],char mode[]) { return; }
		virtual void onstartpug(char adminpass[],char serverpass[]) { return; } // called on start pug

		char chan[32];
		char password[16];
		char adminpassword[6];		
	protected:
		player Players[10];
		unsigned long port;
		int pcount;
		bool inprogress, mapset;
		char ip[32], map[32];
};				    

#define PLAYERSIZE 10 // how many people are needed in the pug to start it?
#define PUG_START "000,1[ 9,1eXeTeL 14,1:0,1 Starting Pug on %s 9,1]"
#define PUG_END "000,1[ 9,1eXeTeL 14,1:0,1 Ending Pug 9,1]"
#define PUG_INPROGRESS "000,1[ 9,1#eXeTeL 14,1:0,1 Pug in progress 9,1]"
#define PUG_EMPTY "000,1[ 9,1eXeTeL 14,1:0,1 Cannot !join - type !pug <level> 9,1]"
#define PUG_FULL "0,1[ 9,1eXeTeL 14,1:0,1 Pug is full 9,1]"
#define PUG_JOIN "0,1[ 9,1eXeTeL 14,1:0,1 Pug is already started - type !join 9,1]"
#define PUG_NOGAME "000,1[ 9,1eXeTeL 14,1:0,1 No game is in progress 9,1]"
#define PUG_NONADMIN "0,1[ 9,1eXeTeL 14,1:0,1 You are not the admin. 9,1]"
#define PUG_INFO "000,1[ 9,1eXeTeL 14,1:0,1 Pug is starting @ cs://%s:%d/%s 9,1]"
#define PUG_ADMIN "000,1[ 9,1eXeTeL 14,1:0,1 You are the Administrator, !login password is \"%s\" 9,1]"
#define PUG_SET "000,1[ 9,1eXeTeL 14,1:0,1  Password has been changed to \"%s\" 9,1]"
#define PUG_GET "0,1[ 9,1eXeTeL0,1PUG 9,1] 14,1-0,1 [ 9,1Ingame 14,1:0,1 Player Needed! 9,1] 14,1- 0,1[ 9,1Launcher 14,1: 0,1cs://%s:%d/%s 9,1] 14,1- 0,1[ 9,1Password 14,1: 0,1%s 9,1]"
#define PUG_REPLACE "0,1[ 9,1eXeTeL0,1PUG 9,1] 14,1-0,1 [ 9,1Ingame 14,1:0,1 Replacement Needed! 9,1] 14,1- 0,1[ 9,1Launcher 14,1: 0,1cs://%s:%d/%s 9,1] 14,1- 0,1[ 9,1Password 14,1: 0,1%s 9,1]"

#endif
