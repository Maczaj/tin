all: client server

client: libclient.o fs_client.cpp fs_server.h
	g++ fs_client.cpp libclient.o -o client -lboost_serialization

server: server_web.o server_file.o fs_server.cpp
	g++ server_web.o server_file.o fs_server.cpp -o server -lpthread -lboost_serialization -std=c++11

server_file.o: fs_server_file.cpp fs_server_file.h
	g++ -c fs_server_file.cpp -o server_file.o -std=c++11

server_web.o: fs_server_web.cpp fs_server_web.h
	g++ -c fs_server_web.cpp -o server_web.o -std=c++11

libclient.o: fs_libclient.cpp fs_libclient.h
	g++ -c fs_libclient.cpp -o libclient.o

clean:
	rm *.o client server
