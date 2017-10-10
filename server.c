//James Marvin, Michael Burke, William Theisen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdint.h>
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
		int filesize;

		char* fname;
		
	
		int i;
		printf("%s\n",buf);
		char *str = strdup(buf);
		char *token;
		char *command;
		int wc=0;
		int32_t fsize;
		while ((token = strsep(&str, ",")) != NULL)
		{
			wc++;
			if(wc==1)
				command=token;
			else if(wc==2)
				filesize=atoi(token);
			else if(wc==3)
				fname=token;	
					
		}

		printf("%s\n",command);
		printf("%d\n",filesize);
		printf("%s\n",fname);

		if(strcmp(command,"DWLD")==0)
		{
			//check to see if file exists
			if(access(fname, F_OK) !=-1)
			{
				//file exists
				struct stat st;
				stat(fname, &st);
				fsize = st.st_size;
			}
			else
			{
				fsize = -1;
			}
			char* message;
			sprintf(message,"%d\n",fsize);
			//send file size to the client 
			 	
		}						
					
		close(s2);
	}
		
}
