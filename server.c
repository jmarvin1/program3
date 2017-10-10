//James Marvin, Michael Burke, William Theisen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
//Ports we can use 41028, 41029, 41030
#define MAX_PENDING 2
#define BUFFER 8128
int main(int argc, char *argv[])
{
	//create data structure
	struct sockaddr_in sin;
	char buf[BUFFER];
	int len;
	int s, s2;
	int port;
	//handle arguments
	if(argc==2)
	{
		port = atoi(argv[1]);
	}
	else
	{
		fprintf(stderr, "Innappropriate amount of arguments");
		exit(1);
	}	
	//prepare address
	bzero((char*)&sin, sizeof(sin));
	sin.sin_family= AF_INET;
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_port=htons(port);
	
	
	
}
