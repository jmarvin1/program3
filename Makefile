all: 
	gcc server.c -o myftpd
	gcc client.c -o myftp

clean:
	rm myftpd myftp
