//James Marvin, Michael Burke, William Thiesen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define BUFFER 8128

int dwld(int s)
{
    size_t sizeInput;
    char * inputFile;
    
    printf("Enter the file to download\n");
    int bytes = getline(&inputFile, &sizeInput, stdin);
    
    char * fSize;
    
    sprintf(fSize, "%d", bytes);

    int sizeSent;

    char action [BUFFER];
    printf("string copy\n");
    strcpy(action, "DWLD ");
    strcpy(action, fSize);
    strcpy(action, " ");
    strcpy(action, inputFile);

    printf("sending string: %s\n", inputFile);


    //sprintf(message, "%s %d %s", action, sizeof(inputFile), inputFile);
    printf("message:\n%s", message);
    char * action = "DWLD,10,test.txt\0";
    int sizeSent;
    if ((sizeSent = send(s, action, BUFFER, 0)) < 0)
    {
        perror("Error sending message\n");
        close(s);
        exit(1);
    }
    return 0;

}

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
		fprintf(stderr,"Inappropriate number of arguments. Need 2 arguments\n");
		exit(1);
	}
	
	//prepare hostname 
	hp = gethostbyname(host);
	if(!hp)
	{
		fprintf(stderr, "Can't find host\n");
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
		perror("error creating socket\n");
		exit(1);
	}
	
	//connet the socket to the server
	if(connect(s, (struct sockaddr *)&sin, sizeof(sin))<0)
	{
		perror("error connecting socket to server\n");
		close(s);
		exit(1);
	}

    while (1) 
    {
        printf("Enter a command to perform:\n\t");
        printf("DWLD: Download\n\tUPLD: Upload\n\tLIST: List\n\tMDIR: ");
        printf("Make Directory\n\tRDIR: Remove Directory\n\tCDIR: ");
        printf("Change Directory\n\tDELF: Delete File\n\tQUIT: Exit\n");
        
        size_t sizeInput;
        char * inputAction;
        int bytes = getline(&inputAction, &sizeInput, stdin);
        
        if (!bytes) {
            fprintf(stderr, "Error reading input\n");
            exit(1);
        }
        else if (bytes != 5) {
            printf("Incorrect action: try again\n");
        }
        else if (strcmp(inputAction, "DWLD\n") == 0)
        {
            printf("DWLD\n");
            dwld(s);
        }
        else if (strcmp(inputAction, "UPLD\n") == 0)
        {
            printf("UPLD\n");
        } 
        else if (strcmp(inputAction, "LIST\n") == 0)
        {
            printf("LIST\n");
        } 
        else if (strcmp(inputAction, "MDIR\n") == 0)
        {
            printf("MDIR\n");
        } 
        else if (strcmp(inputAction, "RDIR\n") == 0)
        {
            printf("RDIR\n");
        } 
        else if (strcmp(inputAction, "CDIR\n") == 0)
        {
            printf("CDIR\n");
        } 
        else if (strcmp(inputAction, "DELF\n") == 0)
        {
            printf("DELF\n");
        } 
        else if (strcmp(inputAction, "QUIT\n") == 0)
        {
            printf("QUIT\n");
            return 0;
        } else {
            printf("error: improper action: try again\n");
        } 

        printf("Action: %s\n", inputAction);
        
        char *message = "DWLD 10 test.txt ";
        int sizeSent;
        if ((sizeSent = send(s, message, sizeof(message), 0)) < 0)
        {
            perror("Error sending message\n");
            close(s);
            exit(1);
        }
       return 1; 
    }	
}
