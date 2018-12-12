// Windows	1 - No Windows support at current time, do not use.
// Unix		2
#define OS 2

#if OS == 2
#define Sleep(x) sleep(x/1000)
#define strcmpi(x,y) strcasecmp(x,y)
#define strnicmp(x,y,z) strncasecmp(x,y,z)
#define LPSOCKADDR struct sockaddr*
#endif
