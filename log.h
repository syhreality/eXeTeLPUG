#ifndef LOG_H
#define LOG_H
#include "config.h"

class LOG
{
	public:
		int connect(int);
		int read(char *buffer);
		bool check();
		void process(char message[]);
		void changelevel(char[]);
		void close();
        int getip(char buffer[],char *ip);
        void getname(char buffer[],char *name);
		int getsteamid(char buffer[],char *steam);
		int gettext(char buffer[],char *text);
		virtual void ingamesay(char steamid[], char text[]) { return; }

	private:
		SOCK socket;
};

#endif
