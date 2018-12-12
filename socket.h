#ifndef SOCK_H
#define SOCK_H
#include "config.h"
#if OS == 1
#include <winsock2.h>
#elif OS == 2
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#endif

#define MTU 1500

class SOCK
{
    public:
        void socket(int, int, int);
        int connect(char[], int);
        int write(char[]);
        int read(char*);
		int readfrom(char*); // #12
		int close();
		int server(unsigned int port);
		bool checkfordata();
    private:
        int sockfd;
        struct sockaddr_in sin;
};

#endif
