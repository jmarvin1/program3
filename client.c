//James Marvin, Michael Burke, William Thiesen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <inttypes.h>
#include <ctype.h>
#define BUFFER 8128

/*unsigned int convert(char *st) {
  char *x;
  for (x = st ; *x ; x++) {
      if (!isdigit(*x))
        return 0L;
    }
  return (strtoul(st, 0L, 10));
}
*/
int dwld(int s)
{
    char  inputFile [BUFFER];   
    printf("Enter the file to download\n");
    scanf("%s", inputFile);
    
    int sizeSent;
    char action [BUFFER] = "DWLD\0";
    if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    {
        perror("Error sending action DWLD\n");
        close(s);
        exit(1);
    }

    char sizeName [BUFFER];
    short int tmpShort = (short int)strlen(inputFile);
    uint16_t sizeOfName = htons(tmpShort);

    sprintf(sizeName, "%" PRIu16, sizeOfName);
    strcat(sizeName, ",");
    strcat(sizeName, inputFile);
    if ((sizeSent = send(s, sizeName, strlen(sizeName), 0)) < 0)
    {
        perror("Error sending sizeName\n");
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

    long fSize = ntohl(atoi(rBuffer));
    if (fSize < 0)
    {
        exit(1);
    }
    char fBuffer[(int)fSize];
    if ((rSize = recv(s, fBuffer, fSize, 0)) < 0) 
    {
        perror("Error receiving message after DWLD sent\n");
        close(s);
        exit(1);
    }

    FILE *fp = fopen(inputFile, "w");
    if (fwrite(fBuffer, 1, fSize, fp) != fSize) {
        perror("ERROR: UPLD: write error\n");
        close(s);
        fclose(fp);
        exit(1);
    }
    fclose(fp);

    return 0;
}


int upld(int s)
{
    char uploadFile [BUFFER];
    
    printf("Enter the file to upload\n");
    
    scanf("%s", uploadFile); 
    
    printf("yeet: %s\n", uploadFile);
    if( access( uploadFile, F_OK ) != 0 ) 
    {
        printf("file does not exist\n");
        exit(1);
    } 
    
    FILE *fp = fopen(uploadFile, "r");
    
    if (fp == NULL)
    {
        fprintf(stderr, "cannot open file\n");
        close(s);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    int fSize = ftell(fp);
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
    short int tmpShort = (short int)strlen(uploadFile);
    uint16_t sizeOfName = htons(tmpShort);
    
    sprintf(sizeName, "%" PRIu16, sizeOfName);
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
    if ((rSize = recv(s, rBuffer, BUFFER, 0)) <= 0) 
    {
        perror("Error receiving First ack\n");
        close(s);
        printf("size of received: %d\n", rSize);
        printf("From Server: %s\n", rBuffer);
        exit(1);
    }
    if (strcmp(rBuffer, "ACK") == 0)
    { 
        char sizeFile [BUFFER];
        
        int tmpInt = (int)(strlen(buf));
        uint32_t sizeOfFile = htonl(tmpInt);
        sprintf(sizeFile, "%" PRIu32, sizeOfFile);
        printf("%" PRIu32 " yeet\n", sizeOfFile);
        
        if ((sizeSent = send(s, sizeFile, strlen(sizeFile), 0)) < 0)
        {
            perror("Error sending sizeFile\n");
            close(s);
            free(buf);
            fclose(fp);
            exit(1);
        }
        if ((sizeSent = send(s, buf, strlen(buf), 0)) < 0)
        {
            perror("Error sending UPLD file\n");
            close(s);
            free(buf);
            fclose(fp);
            exit(1);
        }
    }

    char rBuffer1[BUFFER];
    if ((rSize = recv(s, rBuffer1, BUFFER, 0)) <= 0) 
    {
        perror("Error receiving ACK about UPLD file\n");
        close(s);
        exit(1);
    }

    if (strcmp(rBuffer1, "ACK") != 0)
    {
        fprintf(stderr, "Server side failure uploading file\n");
        free(buf);
        fclose(fp);
        close(s);
        exit(1);
    }

    printf("We did it!\n%s\n", rBuffer1);

    free(buf);
    fclose(fp);
    return 0;
}

int list(int s)
{
    int sizeSent;
    char action [BUFFER] = "LIST\0";
    if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    {
        perror("Error sending action LIST\n");
        close(s);
        exit(1);
    }

    int rSize;
    char rBuffer[BUFFER];
    if ((rSize = recv(s, rBuffer, BUFFER, 0)) <= 0) 
    {
        perror("Error receiving size of incoming directory listing\n");
        close(s);
        exit(1);
    }

    uint32_t sizeOfList = ntohl(atoi(rBuffer));
    char lBuffer[(int)sizeOfList]; 
    if ((rSize = recv(s, lBuffer, strlen(lBuffer), 0)) <= 0) 
    {
        perror("Error receiving size of incoming directory listing\n");
        close(s);
        exit(1);
    }

    printf("%s\n", lBuffer);
    return 0;
}

int delf(int s)
{
	int sizeSent;
	char action [BUFFER] = "DELF\0";
    	if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    	{
        	perror("Error sending action DELF\n");
        	close(s);
        	exit(1);
    	}
	char deleteFile[BUFFER];
	printf("Enter the file to be deleted:\n");
	scanf("%s",deleteFile);
	
	
	short int filenameLength = (short int)strlen(deleteFile);
	uint16_t sizeOfName = htons(filenameLength);
    	char sizeName[BUFFER];
    	sprintf(sizeName, "%" PRIu16, sizeOfName);
    	strcat(sizeName, ",");
    	strcat(sizeName, deleteFile);
    	strcat(sizeName, "\0");
    
    	if ((sizeSent = send(s, sizeName, strlen(sizeName), 0)) < 0)
    	{
        	perror("Error sending sizeName\n");
        	close(s);
        	exit(1);
    	}

	int rSize;
    	char rBuffer[BUFFER];
    	if ((rSize = recv(s, rBuffer, BUFFER, 0)) <= 0) 
    	{
        	perror("Error receiving whether or not file exists\n");
        	close(s);
        	exit(1);
    	}
	
	if(strcmp(rBuffer, "1")==0)
	{
		//file exists 
		char request[BUFFER];
		printf("The file exists. Are you sure you want to delete the file?(yes,no)\n");
		scanf("%s",request);
		//sending confirmation
		if(strcmp(request,"no")==0)
		{
			printf("Delete abandoned by user\n");
			close(s);
			return 0;
		}
		else if(strcmp(request,"yes")==0)
		{
			if((sizeSent= send(s, "yes\0", strlen("yes\0"),0))<=0)
			{
				perror("Error sending the delete confirmation");
				close(s);
			}
			
		}
	}
	else
	{
		//file does not exist
		printf("File does not exist");
		close(s);
		return 0;
	}

	int ackSize;
	char ackBuffer[BUFFER];
	if(( ackSize = recv(s, ackBuffer, BUFFER, 0))<0)
	{
		perror("error receiving final deletion acknowledgement");
		close(s);
		exit(1);
	}

	if(strcmp(ackBuffer,"1")==0)
	{
		//file was deleted 
		printf("File was successfully deleted!\n");
		return 0;
	}
	else
	{
		printf("File could not be deleted\n");
		return 0;
	}


}		
  
 	

int main(int argc, char * argv[])
{
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	int s;
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
        
        char inputAction [BUFFER];
        scanf("%s", inputAction);
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
	    delf(s);
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
