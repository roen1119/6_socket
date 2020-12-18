all: server client

CC = clang++

OPTIONS =  -O0 -std=c++17 -lpthread

server: server.cpp server.h
	$(CC) $(OPTION) -o server server.cpp

client: client.cpp client.h
	$(CC) $(OPTION) -o client client.cpp

clean
	-rm -f *.o, server, client