#include <iostream>
#include "config.h"
#include "socket.h"

void SOCK::socket(int family, int type, int protocol)
{
	#if OS == 1
	WSADATA wsaData;
    WORD version;
    version = MAKEWORD(2, 0);
    WSAStartup(version, &wsaData);

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0 )
        WSACleanup();

    this->sockfd = (int)WSASocket(family, type, protocol, NULL, 0, 0);

    if (this->sockfd == INVALID_SOCKET)
        perror("WSASocket");

	#elif OS == 2
	this->sockfd = ::socket(family, type, protocol);
	if (this->sockfd < 0)
		perror("socket");
	#endif
}

int SOCK::connect(char host[], int port)
{
	hostent *h;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
	h = gethostbyname(host);
	if (!h)
	{
		printf("Could not get host.\n");
		exit(1);
	}
	memcpy(&sin.sin_addr, h->h_addr, h->h_length);

	#if OS == 1
    if (WSAConnect(sockfd, (struct sockaddr *)&sin, sizeof(sin), NULL, 0, 0, 0) != 0)
        return -1;
	return 1;

	#elif OS == 2
	if (::connect(sockfd, (sockaddr *)&sin, sizeof(sin)) < 0)
         return -1;
	return 1;
	#endif
}

int SOCK::write(char buffer[])
{
    int length = (int)strlen(buffer);

    if(send(this->sockfd, buffer, length, 0) < 0)
        return -1;
    return 1;

}

int SOCK::read(char *buffer)
{
    return(recv(this->sockfd, buffer, MTU, 0));
}

int SOCK::readfrom(char *buffer)
{
    int length = 0;

	#if OS == 1
	int namelen = sizeof(struct sockaddr_in);
	#elif OS == 2
	socklen_t namelen = sizeof(struct sockaddr_in);
	#endif

    length = recvfrom(this->sockfd, buffer, MTU, 0, (struct sockaddr *)&sin, &namelen);
    return length;
}

int SOCK::close()
{
	int ret;

	#if OS == 1
    ret = shutdown(this->sockfd, 2);
    WSACleanup();

	#elif OS == 2
	ret = ::close(this->sockfd);
    #endif

    return ret;
}

int SOCK::server(unsigned int port)
{
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;	// Since this socket is listening for connections,
	sin.sin_port = htons(port);
	return (::bind(sockfd, (LPSOCKADDR)&sin, sizeof(sockaddr_in)));
}

bool SOCK::checkfordata() 
{
	fd_set fds;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds); // adds sock to the file descriptor set
	select(sockfd+1, &fds, NULL, NULL, &tv);
	return((bool)FD_ISSET(sockfd, &fds));
}
