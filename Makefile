CC=g++
CFLAGS=-Wall -O2

all: servidor cliente

servidor: servidor.c banco.h
	$(CC) $(CFLAGS) servidor.c -o servidor.exe -pthread

cliente: cliente.c banco.h
	$(CC) $(CFLAGS) cliente.c -o cliente.exe

clean:
	rm -f servidor.exe cliente.exe
