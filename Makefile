all: myftpd myftp

myftpd: 
	gcc server.c -o myftpd

myftp: 
	gcc client.c -o myftp

