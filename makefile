all: client server

client: libclient.o
	g++ -ansi -pedantic -Wall -Wextra fs_client.cpp libclient.o -o client

server: server_web.o server_file.o
	g++  server_web.o server_file.o fs_server.cpp -o server

server_file.o:
	g++ -ansi -pedantic -Wall -Wextra -c fs_server_file.cpp -o server_file.o

server_web.o:
	g++ -ansi -pedantic -Wall -Wextra -c fs_server_web.cpp -o server_web.o

libclient.o:
	g++ -ansi -pedantic -Wall -Wextra -c fs_libclient.cpp -o libclient.o

clean:
	rm *.o client server