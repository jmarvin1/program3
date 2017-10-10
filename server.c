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

enum action {
    DWLD,
    UPLD,
    DELF,
    LIST,
    MDIR,
    RDIR,
    CDIR,
    QUIT
};

int download(char* command[])
{
    char* args[4];
    int i = 0;
    char* token = strtok(command[1], ",");
    while (token != NULL)
        {args[i] = strdup(token); i++; token=strtok(NULL, ",");}

    return 1;
}

int upload(char*  command[])
{
    char* args[4];
    int i = 0;
    char* token = strtok(command[1], ",");
    while (token != NULL)
        {args[i] = strdup(token); i++; token=strtok(NULL, ",");}

    short fNameSize = ntohs(atoi(args[0]));
    long fSize = ntohl(atoi(command[2]));

    FILE *fp = fopen(args[1], "w");
    if (fwrite(command[3], 1, fSize, fp) != fSize) {
        printf("ERROR: UPLD: write error\n");
    }
    fclose(fp);

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

        int i;
        char* command[10];
        int fSize = 0;

        for (i = 0; i > -1; i++) {
            bzero(buf, strlen(buf));

            if((len=recv(s2, buf, sizeof(buf), 0))==-1) {
                perror("ERROR: receiving message");
                exit(1);
            }
            if(len==0) {
                break;
            }

            command[i] = strdup(buf);

            if (i == 1 || i == 3) {
                if ((send(s2, "ACK", strlen("ACK"), 0)) < 0) {
                    printf("ERROR: send\n");
                    exit(1);
                }
            }

            if (!strcmp(command[0], "DWLD") && i == 3)
                { download(command); break; }
            else if (!strcmp(command[0], "UPLD") && i == 3)
                { upload(command); break; }
        }
		close(s2);
	}
}
