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

int sendData(int client, char* data)
{ return send(client, data, strlen(data), 0); }

int recieveData(int client, char* buf, int len)
{ return recv(client, buf, len, 0); }

int download(int client)
{

    char meow[256];
    bzero(meow, sizeof(meow));
    printf("recieve 1\n");
    recieveData(client, meow, sizeof(meow));

    int i = 0;
    char* args[2], token = strtok(meow, ",");
    while (token != NULL)
        { args[i] = strdup(token); i++; token=strtok(NULL, ","); }

    return 1;
}

int upload(int client)
{
    char meow[256];
    bzero(meow, sizeof(meow));
    printf("recieve 1\n");
    recieveData(client, meow, sizeof(meow));

    int i = 0;


    char* args[2], token = strtok(meow, ",");
    while (token != NULL)
        { args[i] = strdup(token); i++; token=strtok(NULL, ","); }

    short fNameSize = ntohs(atoi(args[0]));

    printf("recieve 2\n");

    sendData(client, "ACK");

    /* recieveData(client, input[1]); */

    /* long fSize = ntohl(atoi(input[1])); */
    /* recieveData(client, input[2]); */

    /* FILE *fp = fopen(args[1], "w"); */
    /* if (fwrite(input[2], 1, fSize, fp) != fSize) { */
    /*     printf("ERROR: UPLD: write error\n"); */
    /*     exit(1); */
    /* } */
    /* return fclose(fp); */
    return 1;
}

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

        bzero(buf, strlen(buf));

        if (recieveData(s2, buf, sizeof(buf)) < 0) {
            printf("ERROR: recieving\n");
            exit(1);
        }

        if (!strcmp(buf, "DWLD")) { download(s2); }
        else if (!strcmp(buf, "UPLD")) { upload(s2); }

		close(s2);
	}
}
