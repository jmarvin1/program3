//James Marvin, Michael Burke, William Thiesen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define BUFFER 8128

int main(int argc, char * argv[])
{
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char buf[BUFFER];
	int s;
	int len;
	int port;
	//Handle arguments 
	if(argc==3)
	{
		host=argv[1];
		port= atoi(argv[2]);
	}
	else
	{
		fprintf(stderr,"Inappropriate number of arguments. Needs to be two");
		exit(1);
	}
	
	//prepare hostname 
	hp = gethostbyname(host);
	if(!hp)
	{
		fprintf(stderr, "Can't find host");
		exit(1);
	}

	//prepare address
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family= AF_INET;
	bcopy(hp->h_addr, (char*)&sin.sin_addr,hp->h_length);
	sin.sin_port = htons(port);

	//create a socket
	if((s=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		perror("error creating socket");
		exit(1);
	}
	
	//connet the socket to the server
	if(connect(s, (struct sockaddr *)&sin, sizeof(sin))<0)
	{
		perror("error connecting socket to server");
		close(s);
		exit(1);
	}

	
}

