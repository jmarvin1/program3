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
    char  inputFile [BUFFER];
    
    printf("Enter the file to download\n");
    scanf("%s\0", inputFile);
    
    char fSize [BUFFER];
    sprintf(fSize, "%d", strlen(inputFile));
    char action [BUFFER];

    strcat(action, "DWLD ");
    strcat(action, fSize);
    strcat(action, " ");
    strcat(action, inputFile);

    printf("sending string: %s\n", action);

    int sizeSent;
    if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    {
        perror("Error sending message\n");
        close(s);
        exit(1);
    }
    int rSize;
    char rBuffer[BUFFER];
    if ((rSize = recv(s, rBuffer, BUFFER, 0)) < 0) 
    {
        perror("Error receiving message after DWLD sent\n");
        close(s);
        exit(1);
    }


    return 0;

}


int upld(int s)
{
    size_t sizeInput;
    char uploadFile [BUFFER];
    
    printf("Enter the file to upload\n");
    
    scanf("%s\0", uploadFile); 
    
    printf("yeet: %s\n", uploadFile);
    if( access( uploadFile, F_OK ) != 0 ) 
    {
        printf("file does not exist\n");
        exit(1);
    } 
    
    FILE *fp = NULL;
    fp = fopen(uploadFile, "r");
    
    if (fp == NULL)
    {
        fprintf(stderr, "cannot open file\n");
        close(s);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    long fSize = ftell(fp);
    fseek(fp, 0, SEEK_SET); 
    
    char *buf = malloc(fSize + 1);
    
    size_t bRead = fread(buf, fSize, 1, fp);
    if (bRead > 0) {
        printf("read was successful\n"); 
        buf[fSize] = '\0';
    }
    else {
        printf("read is wrong\n");
        exit(1);
    }
  
    int sizeSent;
    
    char action [BUFFER] = "UPLD\0";
    if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    {
        perror("Error sending action\n");
        close(s);
        free(buf);
        fclose(fp);
        exit(1);
    }

    char sizeName [BUFFER];
    sprintf(sizeName, "%d\0", strlen(uploadFile));
    strcat(sizeName, ",");
    strcat(sizeName, uploadFile);
    strcat(sizeName, "\0");
    if ((sizeSent = send(s, sizeName, strlen(sizeName), 0)) < 0)
    {
        perror("Error sending sizeName\n");
        close(s);
        free(buf);
        fclose(fp);
        exit(1);
    }

    int rSize;
    char rBuffer[BUFFER];
    if ((rSize = recv(s, rBuffer, BUFFER, 0)) < 0) 
    {
        perror("Error receiving ACK about UPLD fileSize\n");
        close(s);
        exit(1);
    }

    if (strcmp(rBuffer, "letsgo") == 0)
    { 
        char sizeFile [BUFFER];
        sprintf(sizeFile, "%d\0", fSize + 1);
        if ((sizeSent = send(s, sizeFile, strlen(sizeFile), 0)) < 0)
        {
            perror("Error sending sizeFile\n");
            close(s);
            free(buf);
            fclose(fp);
            exit(1);
        }
        if ((sizeSent = send(s, buf, fSize + 1, 0)) < 0)
        {
            perror("Error sending UPLD file\n");
            close(s);
            free(buf);
            fclose(fp);
            exit(1);
        }
    }

    char rBuffer1[BUFFER];
    if ((rSize = recv(s, rBuffer1, BUFFER, 0)) < 0) 
    {
        perror("Error receiving ACK about UPLD file\n");
        close(s);
        exit(1);
    }

    printf("We did it!\n%s\n", rBuffer1);

    free(buf);
    fclose(fp);
    return 1;
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
    int yo = 0;
    while (1) 
    {
        printf("Enter a command to perform:\n\t");
        printf("DWLD: Download\n\tUPLD: Upload\n\tLIST: List\n\tMDIR: ");
        printf("Make Directory\n\tRDIR: Remove Directory\n\tCDIR: ");
        printf("Change Directory\n\tDELF: Delete File\n\tQUIT: Exit\n");
        
        size_t sizeInput;
        char inputAction [BUFFER];
        scanf("%s\0", inputAction);
        if (strcmp(inputAction, "DWLD") == 0)
        {
            printf("DWLD\n");
            dwld(s);
        }
        else if (strcmp(inputAction, "UPLD") == 0)
        {
            printf("UPLD\n");
            upld(s);
        } 
        else if (strcmp(inputAction, "LIST") == 0)
        {
            printf("LIST\n");
        } 
        else if (strcmp(inputAction, "MDIR") == 0)
        {
            printf("MDIR\n");
        } 
        else if (strcmp(inputAction, "RDIR") == 0)
        {
            printf("RDIR\n");
        } 
        else if (strcmp(inputAction, "CDIR") == 0)
        {
            printf("CDIR\n");
        } 
        else if (strcmp(inputAction, "DELF") == 0)
        {
            printf("DELF\n");
        } 
        else if (strcmp(inputAction, "QUIT") == 0)
        {
            close(s);
            return 0;
        } else {
            printf("error: improper action: try again\n");
        } 
    }
    close(s);
    return 0;    
}
