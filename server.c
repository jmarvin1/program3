//James Marvin, Michael Burke, William Theisen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
//Ports we can use 41028, 41029, 41030
#define MAX_PENDING 10
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
	
	//create socket
	if(((s=socket(PF_INET, SOCK_STREAM,0))<0))
	{
		perror("error creating socket");
		exit(1);
	}
	
	//make socket reusable 
	int opt = 1;
    if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&opt,sizeof(int)))<0)
	{
		perror("error setting sock to reusable");
		exit(1);
	}
	
	//bind the socket to the specific address
	if(bind(s,(struct sockaddr *)&sin, sizeof(sin))<0)
	{
		perror("error binding socket");
		exit(1);
	}
	
	//start listening
	if((listen(s, MAX_PENDING))<0)	
	{
		perror("error listening");
		exit(1);
	}

	while(1)
	{
		//accept the client
		if((s2=accept(s, (struct sockaddr *)&sin, &len))<0)
		{
			perror("error accepting");
			exit(1);
		}
		
		if((len=recv(s2, buf, sizeof(buf), 0))==-1)
		{
			perror("error receiving message");
			exit(1);
		}
		if(len==0)
		{
			break;
		}
		printf("%s\n",buf);
		close(s2);
	}
		
}
