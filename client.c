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
#include <sys/time.h> 

#define BUFFER 8128

//download function
int dwld(int s)
{
    //get desired file name
    char  inputFile [BUFFER];   
    printf("Enter the file to download\n");
    scanf("%s", inputFile);
    //send action
    int sizeSent;
    char action [BUFFER] = "DWLD\0";
    if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    {
        perror("Error sending action DWLD\n");
        close(s);
        exit(1);
    }
    //get name and size in appropriate format
    char sizeName [BUFFER];
    short int tmpShort = (short int)strlen(inputFile);
    uint16_t sizeOfName = htons(tmpShort);

    sprintf(sizeName, "%" PRIu16, sizeOfName);
    strcat(sizeName, ",");
    strcat(sizeName, inputFile);
    //send name and size
    if ((sizeSent = send(s, sizeName, strlen(sizeName), 0)) < 0)
    {
        perror("Error sending sizeName\n");
        close(s);
        exit(1);
    }
    //receive file size 
    int rSize;
    char rBuffer[BUFFER]; 
    if ((rSize = recv(s, rBuffer, BUFFER, 0)) < 0) 
    {
        perror("Error receiving the size of file to be downloaded\n");
        close(s);
        exit(1);
    }
    //create appropriate buffer size  
    long fSize = ntohl(atoi(rBuffer));
    if (atoi(rBuffer) == -1)
    {
        printf("File %s does not exist\n", inputFile);
        return 1;
    }
    printf("fSize: %d\n", (int)fSize);
    char *fBuffer = malloc(fSize);
    if (!fBuffer) {
        printf("problem with malloc\n");
        close(s);
        exit(1);
    }
    struct timeval st, et;
    // start timer
    gettimeofday(&st,NULL);
    
    if ((rSize = recv(s, fBuffer, fSize, 0)) < 0) 
    {
        perror("Error receiving the DWLD file\n");
        free(fBuffer);
        close(s);
        exit(1);
    }

    // stop timer
    gettimeofday(&et,NULL);
    double elapsed = (((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec));
    printf("%d bytes transferred in %f microseconds or %f seconds : %f MegaBytes/sec\n", (int)fSize, elapsed, elapsed / 1000000, (double)rSize / elapsed);

    printf("opening file to write to\n");
    FILE *fp = fopen(inputFile, "w");
    if (fwrite(fBuffer, 1, fSize, fp) < 0) {
        perror("ERROR: DWL: writing download file to client\n");
        free(fBuffer);
        close(s);
        fclose(fp);
        exit(1);
    }
    fclose(fp);
    free(fBuffer);
    return 0;
}

//upload function 
int upld(int s)
{
    //get desired file to upload
    char uploadFile [BUFFER];
    
    printf("Enter the file to upload\n");
    
    scanf("%s", uploadFile); 
   //check existence of the file 
    if( access( uploadFile, F_OK ) != 0 ) 
    {
        printf("file does not exist\n");
        exit(1);
    } 
    
    FILE *fp = fopen(uploadFile, "r");
    //get contents of file
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
  //send upld action
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
    //get name and size in appr format
    char sizeName [BUFFER];
    short int tmpShort = (short int)strlen(uploadFile);
    uint16_t sizeOfName = htons(tmpShort);
    
    sprintf(sizeName, "%" PRIu16, sizeOfName);
    strcat(sizeName, ",");
    strcat(sizeName, uploadFile);
    strcat(sizeName, "\0");
    //send name and size
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
//LIST function
int list(int s)
{
    //Send the action command to the server
    int sizeSent;
    char action [BUFFER] = "LIST\0";
    if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    {
        perror("Error sending action LIST\n");
        close(s);
        exit(1);
    }
    
    //receive the size of directory  
    int rSize;
    char rBuffer[BUFFER];
    if ((rSize = recv(s, rBuffer, BUFFER, 0)) < 0) 
    {
        perror("Error receiving size of incoming directory listing\n");
        close(s);
        exit(1);
    }
    //get list
    int i;
    int lSize;
    int sSize;
    uint32_t sizeOfList = ntohl(atoi(rBuffer));
    char lBuffer[BUFFER];
    //printf("%d size of list\n",sizeOfList);
    for(i=0; i<sizeOfList; i++)
    { 
        //receive entry from directory
        bzero(lBuffer, BUFFER);
        if ((lSize = recv(s, lBuffer, sizeof(lBuffer), 0)) < 0) 
    	{
        	perror("Error receiving  directory listing\n");
        	close(s);
       		exit(1);
    	}
        printf("%s\n",lBuffer);
        fflush(stdout);
        //send ack to server
        if ((sSize = send(s,"1",strlen("1"),0))<0)
        {
            perror("Error sending entry ack to server\n");
            close(s);
            exit(1);
        }
    } 
    bzero(rBuffer, sizeof(rBuffer));
    //printf("END OF LIST!!!\n\n");
    
    return 0;
}
//Delete function
int delf(int s)
{
	//send delete action
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
	
	//get name and size in appr format
	short int filenameLength = (short int)strlen(deleteFile);
	uint16_t sizeOfName = htons(filenameLength);
    	char sizeName[BUFFER];
    	sprintf(sizeName, "%" PRIu16, sizeOfName);
    	strcat(sizeName, ",");
    	strcat(sizeName, deleteFile);
    	strcat(sizeName, "\0");
    //send name and size
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
        	perror("Error receiving whether or not file exists\n");
        	close(s);
        	exit(1);
    	}
	//check acknowledgement
	if(strcmp(rBuffer, "1")==0)
	{
		//file exists 
		char request[BUFFER];
		printf("The file exists. Are you sure you want to delete the file?(Y,N)\n");
		scanf("%s",request);
		//sending confirmation
		if(strcmp(request,"N")==0)
		{
			printf("Delete abandoned by user\n");
			close(s);
			return 0;
		}
		else if(strcmp(request,"Y")==0)
		{
			if((sizeSent= send(s, "Yes", strlen("Yes"),0))<=0)
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
    //check final ack
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
 //MDIR function 
int mdir(int s)
{
    //send action 
    int sizeSent;
	char action [BUFFER] = "MDIR\0";
    	if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    	{
        	perror("Error sending action MDIR\n");
        	close(s);
        	exit(1);
    	}
    //get name of dir    
	char dirName[BUFFER];
	printf("Enter the name of the directory to be created:\n");
	scanf("%s",dirName);
    //get in appr format 
    short int dirnameLength = (short int)strlen(dirName);
	uint16_t sizeOfName = htons(dirnameLength);
    char sizeName[BUFFER];
    sprintf(sizeName, "%" PRIu16, sizeOfName);
    strcat(sizeName, ",");
    strcat(sizeName, dirName);
    strcat(sizeName, "\0");
    //send size and name of dir
    if ((sizeSent = send(s, sizeName, strlen(sizeName), 0)) < 0)
    {
        perror("Error sending sizeName\n");
        close(s);
        exit(1);
    }
    //receive the confirmation from the server
    int cSize;
    char confirmBuff[BUFFER];

    if((cSize = recv(s,confirmBuff, BUFFER, 0))<0)
    {
       perror("Error receiving directory confirmation");
       close(s);
       exit(1);
    }
    //check ack
    if(strcmp(confirmBuff,"1")==0)
    {
        printf("Directory successfully made\n");
        return 0;
    }
    else if(strcmp(confirmBuff,"-2")==0)
    {
        printf("Directory already exists on the server\n");
        return 0;
    }
    else if(strcmp(confirmBuff, "-1")==0)
    {
        printf("Error making directory\n");
        return 0;
    }
    return 0;
}
//CDIR function
int cdir(int s)
{
    //send action 
    int sizeSent;
	char action [BUFFER] = "CDIR\0";
    	if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    	{
        	perror("Error sending action CDIR\n");
        	close(s);
        	exit(1);
    	}
    //get name of dir    
	char dirName[BUFFER];
	printf("Enter the name of the destination directory:\n");
	scanf("%s",dirName);
    //get in appr format 
    short int dirnameLength = (short int)strlen(dirName);
	uint16_t sizeOfName = htons(dirnameLength);
    char sizeName[BUFFER];
    sprintf(sizeName, "%" PRIu16, sizeOfName);
    strcat(sizeName, ",");
    strcat(sizeName, dirName);
    strcat(sizeName, "\0");
    //send size and name of dir
    if ((sizeSent = send(s, sizeName, strlen(sizeName), 0)) < 0)
    {
        perror("Error sending sizeName\n");
        close(s);
        exit(1);
    }
    //receive the confirmation from the server
    int cSize;
    char confirmBuff[BUFFER];

    if((cSize = recv(s,confirmBuff, BUFFER, 0))<0)
    {
       perror("Error receiving directory confirmation");
       close(s);
       exit(1);
    }

    if(strcmp(confirmBuff,"-1")==0)
    {
        printf("Error in changing directory\n");
        return 0;
    }
    else if(strcmp(confirmBuff,"1")==0)
    {
        //directory changed
        printf("Changed current directory\n"); 
	    return 0;
    }
    else if(strcmp(confirmBuff,"-2")==0)
    {
        printf("The directory does not exist on the server\n");
        return 0;
    }
    return 0;
}
//RDIR function
int rdir(int s)
{
    //send action 
    int sizeSent;
	char action [BUFFER] = "RDIR\0";
    	if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    	{
        	perror("Error sending action RDIR\n");
        	close(s);
        	exit(1);
    	}
    //get name of dir    
	char dirName[BUFFER];
	printf("Enter the name of the directory to be removed:\n");
	scanf("%s",dirName);
    //get in appr format 
    short int dirnameLength = (short int)strlen(dirName);
	uint16_t sizeOfName = htons(dirnameLength);
    char sizeName[BUFFER];
    sprintf(sizeName, "%" PRIu16, sizeOfName);
    strcat(sizeName, ",");
    strcat(sizeName, dirName);
    strcat(sizeName, "\0");
    //send size and name of dir
    if ((sizeSent = send(s, sizeName, strlen(sizeName), 0)) < 0)
    {
        perror("Error sending sizeName\n");
        close(s);
        exit(1);
    }
    //receive the confirmation from the server
    int cSize;
    char confirmBuff[BUFFER];

    if((cSize = recv(s,confirmBuff, BUFFER, 0))<0)
    {
       perror("Error receiving directory confirmation");
       close(s);
       exit(1);
    }
    //get ack
    if(strcmp(confirmBuff,"-1")==0)
    {
        printf("Directory does not exist\n");
        return 0;
    }
    else if(strcmp(confirmBuff,"1")==0)
    {
        //file exists 
		char request[BUFFER];
		printf("The file exists. Are you sure you want to delete the file?(Y,N)\n");
		scanf("%s",request);
		//sending confirmation
		if(strcmp(request,"N")==0)
		{
			printf("Delete abandoned by user\n");
			close(s);
			return 0;
		}
		else if(strcmp(request,"Y")==0)
		{
			if((sizeSent= send(s, "Yes", strlen("Yes"),0))<=0)
			{
				perror("Error sending the delete confirmation");
				close(s);
			}
			
		}
        return 0;
    }
    return 0;
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
    //prompts and function calls   
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
            list(s);
        } 
        else if (strcmp(inputAction, "MDIR") == 0)
        {
            printf("MDIR\n");
            mdir(s);
        } 
        else if (strcmp(inputAction, "RDIR") == 0)
        {
            printf("RDIR\n");
            rdir(s);
        } 
        else if (strcmp(inputAction, "CDIR") == 0)
        {
            printf("CDIR\n");
            cdir(s);
        } 
        else if (strcmp(inputAction, "DELF") == 0)
        {
            printf("DELF\n");
	        delf(s);
        } 
        else if (strcmp(inputAction, "QUIT") == 0)
        {
            int sizeSent;
	        char action [BUFFER] = "QUIT\0";
    	    if ((sizeSent = send(s, action, strlen(action), 0)) < 0)
    	    {
        	    perror("Error sending action MDIR\n");
        	    close(s);
        	    exit(1);
    	    }
            close(s);
            return 0;
        } else {
            printf("error: improper action: try again\n");
        } 
    }
    close(s);
    return 0;    
}
