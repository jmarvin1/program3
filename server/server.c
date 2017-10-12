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
#include <inttypes.h>
#include <dirent.h>
#include <errno.h>
//Ports we can use 41028, 41029, 41030
#define MAX_PENDING 10
#define BUF_SIZE 8128

int sendData(int client, char* data, int len)
{ return send(client, data, len, 0); }

int recieveData(int client, char* buf, int len)
{ return recv(client, buf, len, 0); }

void octalToString(mode_t octalNum, char* resultString)
{
    char octalString[4];
    sprintf(octalString, "%3o", octalNum&0777);
    int permNum = atoi(octalString);

    char* perms[3];

    int i;
    for (i=0; i < 3; i++) {
        switch(permNum % 10) {
            case 0: perms[i] = "---"; break;
            case 1: perms[i] = "--x"; break;
            case 2: perms[i] = "-w-"; break;
            case 3: perms[i] = "-wx"; break;
            case 4: perms[i] = "r--"; break;
            case 5: perms[i] = "r-x"; break;
            case 6: perms[i] = "rw-"; break;
            case 7: perms[i] = "rwx"; break;
        }
        permNum /= 10;
    } 
    
    strcpy(resultString, perms[2]);
    strcat(resultString, perms[1]);
    strcat(resultString, perms[0]);
}

int download(int client)
{
    char fileNameSize[256];
    bzero(fileNameSize, sizeof(fileNameSize));
    recieveData(client, fileNameSize, sizeof(fileNameSize));

    printf("%s\n", fileNameSize);

    int i = 0;
    char* args[2];
    char* token = strtok(fileNameSize, ",");
    while (token != NULL)
        { args[i] = strdup(token); i++; token=strtok(NULL, ","); }

    printf("%s\n", args[1]);
    short fNameSize = ntohs(atoi(args[0]));

    printf("checking if file exists\n");
    if (access(args[1], F_OK) == -1 ) {
        printf("ERROR: file doesn't exist\n");
        char failed[BUF_SIZE];
        sendData(client, "-1", strlen("-1"));
        return 1;
    }

    printf("opening file\n");
    FILE *fp = fopen(args[1], "r");

    fseek(fp, 0, SEEK_END);
    uint32_t fSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char sizeFile[BUF_SIZE];
    
    sprintf(sizeFile, "%" PRIu32, htonl(fSize));
    printf("sending file size\n");
    printf("file size: %s", sizeFile);
    printf("size if file size: %d\n", strlen(sizeFile));    
    sendData(client, sizeFile, strlen(sizeFile));

    //char fileData[fSize];
    
    char *fileData = malloc(fSize);
    
    printf("reading from file\n");
    fread(fileData, fSize, 1, fp);
    //printf("File: %s\n", fileData);

    printf("sending\n");
    sendData(client, fileData, fSize);
    free(fileData);
    return fclose(fp);
}

int upload(int client)
{
    char fileNameSize[256];
    bzero(fileNameSize, sizeof(fileNameSize));
    recieveData(client, fileNameSize, sizeof(fileNameSize));
    printf("file name and size: %s\n", fileNameSize);

    int i = 0;
    char* args[2];
    char* token = strtok(fileNameSize, ",");
    while (token != NULL)
        { args[i] = strdup(token); i++; token=strtok(NULL, ","); }

    short fNameSize = ntohs(atoi(args[0]));

    sendData(client, "ACK", strlen("ACK"));

    char fileSize[256];
    bzero(fileSize, sizeof(fileSize));
    recieveData(client, fileSize, sizeof(fileSize));

    long fSize = ntohl(atoi(fileSize));

    char fileData[fSize];
    bzero(fileData, strlen(fileData));
    recieveData(client, fileData, fSize);

    printf("File name: %s\n", args[1]);
    FILE *fp = fopen(args[1], "w");
    if (fwrite(fileData, 1, fSize, fp) != fSize) {
        printf("ERROR: UPLD: write error\n");
        exit(1);
    }

    sendData(client, "ACK", strlen("ACK"));

    return fclose(fp);
}

int deleteFile(int client)
{   
    char fileNameSize[256];
    bzero(fileNameSize, sizeof(fileNameSize));
    recieveData(client, fileNameSize, sizeof(fileNameSize));

    int i = 0;
    char* args[2];
    char* token = strtok(fileNameSize, ",");
    while (token != NULL)
        { args[i] = strdup(token); i++; token=strtok(NULL, ","); }

    short fNameSize = ntohs(atoi(args[0]));

    if (access(args[1], F_OK) == -1 ) {
        printf("ERROR: file doesn't exist\n");
        sendData(client, "-1", strlen("-1"));
        return 1;
    } else {
        sendData(client, "1", strlen("1"));
    }

    char deleteConfirm[256];
    bzero(deleteConfirm, sizeof(deleteConfirm));
    recieveData(client, deleteConfirm, sizeof(deleteConfirm));

    if (!strcmp(deleteConfirm, "Yes")) {
        if (remove(args[1]) == -1) {
            printf("ERROR: error deleting file\n");
            sendData(client, "-1", strlen("-1"));
            return 1;
        }
        sendData(client, "1", strlen("1"));
    }

    return 1;
}

int list(int client)
{
    DIR *dp;
    struct dirent *ep;
    char currDir[BUF_SIZE];

    getcwd(currDir, sizeof(currDir));

    dp = opendir(currDir);

    char* dirEnts[BUF_SIZE];
    uint32_t ents = 0;

    if (dp != NULL) {
        struct stat fileStat;
        char permString[9];
        while ((ep = readdir(dp))) {
            if (stat(ep->d_name, &fileStat) < 0) { return 1; }

            bzero(permString, sizeof(permString));
            octalToString(fileStat.st_mode, permString);

            char fileInfoString[strlen(ep->d_name) + 11];
            bzero(fileInfoString, sizeof(fileInfoString));
           
            strcpy(fileInfoString, permString);
            strcat(fileInfoString, " ");
            strcat(fileInfoString, ep->d_name);
            
            dirEnts[ents] = strdup(fileInfoString);
            ents++; 
        }
        closedir(dp);
    } else {
        printf("Directory open failed\n");
        return 1;
    }
   
    char dirSizeBuffer[BUF_SIZE];
    bzero(dirSizeBuffer, sizeof(dirSizeBuffer));
    sprintf(dirSizeBuffer, "%" PRIu32, htonl(ents)); 
    sendData(client, dirSizeBuffer, strlen(dirSizeBuffer));

    int i;
    for (i = 0; i < ents; i++) {
        printf("%s\n", dirEnts[i]);
        sendData(client, dirEnts[i], strlen(dirEnts[i]));
    }

    return 1;
}

int createDir(int client)
{
    char fileNameSize[256];
    bzero(fileNameSize, sizeof(fileNameSize));
    recieveData(client, fileNameSize, sizeof(fileNameSize));

    int i = 0;
    char* args[2];
    char* token = strtok(fileNameSize, ",");
    while (token != NULL)
        { args[i] = strdup(token); i++; token=strtok(NULL, ","); }

    short fNameSize = ntohs(atoi(args[0]));

    if (access(args[1], F_OK) == 0) {
        printf("ERROR: file already exists\n");
        sendData(client, "-2", strlen("-2"));
        return 1;
    }
    
    if (mkdir(args[1], 0666) == -1) {
        printf("ERROR: error creating directory\n");
        sendData(client, "-1", strlen("-1"));
        return 1;
    }

    sendData(client, "1", strlen("1"));

    return 1;
}

int deleteDir(int client)
{   
    char fileNameSize[256];
    bzero(fileNameSize, sizeof(fileNameSize));
    recieveData(client, fileNameSize, sizeof(fileNameSize));

    int i = 0;
    char* args[2];
    char* token = strtok(fileNameSize, ",");
    while (token != NULL)
        { args[i] = strdup(token); i++; token=strtok(NULL, ","); }

    short fNameSize = ntohs(atoi(args[0]));

    DIR* dir = opendir(args[1]);
    if (ENOENT == errno) {
        printf("ERROR: directory does not exist\n");
        sendData(client, "-1", strlen("-1"));
        return 1;
    } else {
        sendData(client, "1", strlen("1"));
    }
    closedir(dir);
   
    char deleteConfirm[256];
    bzero(deleteConfirm, sizeof(deleteConfirm));
    recieveData(client, deleteConfirm, sizeof(deleteConfirm));

    if (!strcmp(deleteConfirm, "Yes")) {
        if (rmdir(args[1]) == -1) {
            printf("ERROR: error deleting directory\n");
            printf("ERROR: is the directory empty?\n");
            sendData(client, "-1", strlen("-1"));
            return 1;
        }
        sendData(client, "1", strlen("1"));
    }

    return 1;
}

int changeDir(int client)
{
    char fileNameSize[256];
    bzero(fileNameSize, sizeof(fileNameSize));
    recieveData(client, fileNameSize, sizeof(fileNameSize));

    int i = 0;
    char* args[2];
    char* token = strtok(fileNameSize, ",");
    while (token != NULL)
        { args[i] = strdup(token); i++; token=strtok(NULL, ","); }

    short fNameSize = ntohs(atoi(args[0]));

    DIR* dir = opendir(args[1]);
    if (errno == ENOENT) {
        printf("ERROR: directory does not exist\n");
        sendData(client, "-2", strlen("-2"));
        return 1;
    }
    closedir(dir);

    if (chdir(args[1]) == -1) {
        printf("ERROR: error changing directory\n");
        sendData(client, "-1", strlen("-1"));
        return 1;
    }

    return 1;
}

int main(int argc, char *argv[])
{
	//create data structure
	struct sockaddr_in sin;
	char buf[BUF_SIZE];
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
        if((s2=accept(s, (struct sockaddr *)&sin, &len))<0)
        {
            perror("error accepting");
            exit(1);
        }
        
    
        while(1)
        {         
            bzero(buf, strlen(buf));
            if (recieveData(s2, buf, sizeof(buf)) < 0) {
                printf("ERROR: recieving\n");
                exit(1);
            }

            printf("%s\n", buf);

            if (!strcmp(buf, "DWLD")) { download(s2); }
            else if (!strcmp(buf, "UPLD")) { upload(s2); }
            else if (!strcmp(buf, "DELF")) { deleteFile(s2); }
            else if (!strcmp(buf, "LIST")) { list(s2); }
            else if (!strcmp(buf, "MDIR")) { createDir(s2); }
            else if (!strcmp(buf, "RDIR")) { deleteDir(s2); }
            else if (!strcmp(buf, "CDIR")) { changeDir(s2); }
            else if (!strcmp(buf, "QUIT")) { break; }
	    }

        close(s2);
	}
}
