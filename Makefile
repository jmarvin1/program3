all:
	gcc server.c -Wall -o myftpd
	gcc client.c -Wall -o myftp

clean:
	rm myftpd myftp
