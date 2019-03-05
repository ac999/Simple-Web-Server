all:
	gcc -o serverTCP servTcpIt.c -Wall
clean:
	rm -f serverTCP
