CC = gcc
CFLAGS =

shell: shell.o
	$(CC) $(CFLAGS) -o shell shell.o

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f shell.o shell
