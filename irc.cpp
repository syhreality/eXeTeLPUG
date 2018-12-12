#include <cstdlib>
#include <string>
#include "irc.h"
#include "extras.h"

int IRC::connect(char nick[], char server[], int port)
{
    socket.socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socket.connect(server, port) < 0)
        return -1;
    else
    {
        char data[4096];

        sprintf(buffer, "NICK %s\r\n", nick);
        if (raw_command(buffer) < 0)
            return -1;    

		sprintf(myName, "%s", nick);
        sprintf(buffer, "USER %s cainor * :eXeTeL Services\r\n", nick);
        if (raw_command(buffer) < 0)
            return -1;
            
        while (IRC::check())
        {
            IRC::read(data);
            IRC::process(data);
        }

        return 0;
    }
}

int IRC::join(char channel[])
{
    sprintf(buffer, "JOIN %s\r\n", channel);
    if (raw_command(buffer) < 0)
        return -1;
        
    return 1;
}

int IRC::leave(char channel[])
{
    sprintf(buffer, "PART :%s\r\n", channel);
    if (raw_command(buffer) < 0)
        return -1;
        
    return 1;
}

int IRC::private_message(char to[], char message[])
{
    sprintf(buffer, "PRIVMSG %s :%s\r\n", to, message);
    if (raw_command(buffer) < 0)
        return -1;
        
    return 1;
}

int IRC::send_channel(char channel[], char message[])
{
    sprintf(buffer, "PRIVMSG %s :%s\r\n", channel, message);
    if (raw_command(buffer) < 0);
        return -1;
        
	return 1;
}

int IRC::send_notice(char nick[], char message[])
{
    sprintf(buffer ,"NOTICE %s : %s\r\n", nick, message);
    if (raw_command(buffer) < 0);
        return -1;

    return 1;
}

int IRC::set_topic(char channel[], char topic[])
{
	sprintf(buffer, "TOPIC %s :%s\r\n", channel, topic);
    if (raw_command(buffer) < 0)
        return -1;

	return 1;
}

int IRC::change_nick(char nick[])
{
    strncpy(myName, nick, sizeof(myName)-1);
    sprintf(buffer, ":%s NICK %s\r\n", circnick, nick);
    if (raw_command(buffer) < 0)
        return -1;

    return 1;
}

int IRC::raw_command(char command[])
{
	if (stristr(command, "www.")) // block url's, prevent glines
		return 0;

	sprintf(buffer, "%s\r\n", command);
    if (socket.write(buffer) < 0)
        return -1;

    return 1;
}

int IRC::read(char *input)
{
    int length = 0;
    length = socket.read(input);

    return length;
}

int IRC::quit(char message[])
{
    sprintf(buffer, "QUIT :%s\n", message);
    if (raw_command(buffer) < 0)
        return -1;

    if (socket.close() < 0)
        return -1;
        
	return 1;
}

bool IRC::check()
{
	//select call here
	return(socket.checkfordata());
}

int IRC::process(char message[])
{
	char *ptr;
	ptr = strtok(message, "\n");

	while (ptr != NULL)
	{
	    char *tmp;
	    tmp = strstr(ptr, "PING :");
	    if (tmp != NULL)
	    {
	        bool valid = true;
            char buffer[256];
	        int i = 0;

	        while((tmp[i] != '\n') && (tmp[i] != '\r'))
	        {
	            buffer[i] = tmp[i];
                i++;
                if (i > sizeof(buffer)-2)
                {
                    valid = false;
                    break;    
                }        
            }
            buffer[i] = '\0';
            if (valid)
            {
                buffer[1] = 'O';
                raw_command(buffer);
            }
        }

		int i = sscanf(ptr, ":%[^!\n\r ]!%s%s%[^\n\r]", szNICK, szUSER, szCOMMAND, szREST);

		if (i == 4)
		{
			if (strcmpi(szCOMMAND,"JOIN") == 0)
			{ 
				strcpy(szCHAN, szREST+2); 
			} //command = join, user = ident, nick = nick, rest = chan

			else if (strcmpi(szCOMMAND,"PART") == 0)
			{
				onpart(szNICK);
			} //command = part, user = ident, nick = nick, rest = chan

			else if (strcmpi(szCOMMAND,"PRIVMSG") == 0)
			{ 
				char *vhostptr = strstr(message,"@");
				char temp;
				int iPtr = 0;

				if (vhostptr != 0)
				{
					vhostptr++;
					while (vhostptr[iPtr] != ' ')
						iPtr++;
					temp = vhostptr[iPtr];
					vhostptr[iPtr] = '\0';
					strcpy(szVHOST, vhostptr);
					vhostptr[iPtr] = temp;
					// we have, vhost!
				}
				sscanf(szREST, "%s :", szCHAN);
				strcpy(szTEXT, szREST + 3 + strlen(szCHAN));
				ontext(szCHAN, szNICK, szVHOST, szTEXT);
			} //command = PRIVMSG, user = ident, nick = nick, rest = chan: message

			else if (!strcmpi(szCOMMAND,"KICK"))
			{
				char *kickptr = szREST + 2;
				char kick[32];
				int i = 0;

				while (*kickptr != ' ')
					*kickptr++;
				*kickptr++;

				while (kickptr[i] != ' ')
				{
					kick[i] = kickptr[i];
					i++;
				}
				kick[i] = '\0';

				if (!strcmp(kick, ircnick)) // bot has been kicked from channel
				{
					sprintf(buffer, "unbanme %s", ircchannel);
					private_message("ChanServ", buffer);
					Sleep(1000);
					join(ircchannel); // rejoin the channel
					join(ircchannel);
				}

				onpart(kick);
			}//command = mode, user = ident, nick = nick, rest = chan modes //does not catch whole list of modes/parameters

			else if (!strcmpi(szCOMMAND,"QUIT"))
			{
				onpart(szNICK);
			}

			else if (strcmpi(szCOMMAND,"NICK") == 0)
			{
				onnamechange(szNICK, szREST + 2);
			} //command nick, user = ident, rest = new nick, nick = old nick
		}
		ptr = strtok(NULL, "\n");
	}

	return 0;
}

