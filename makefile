all: client server

client: libclient.o fs_client.cpp
	g++ fs_client.cpp libclient.o -o client -g -O0 -lboost_serialization 

server: server_web.o server_file.o fs_server.cpp
	g++ server_web.o server_file.o fs_server.cpp -o server -g -O0 -DBOOST_LOG_DYN_LINK -lboost_log -lboost_system -lpthread -lboost_serialization -std=c++11

server_file.o: fs_server_file.cpp
	g++ -c fs_server_file.cpp -o server_file.o -std=c++11 -g -O0 -DBOOST_LOG_DYN_LINK -lboost_log -lboost_system

server_web.o: fs_server_web.cpp
	g++ -c fs_server_web.cpp -o server_web.o -std=c++11 -g -O0 -DBOOST_LOG_DYN_LINK -lboost_log -lboost_system

libclient.o: fs_libclient.cpp
	g++ -c fs_libclient.cpp -o libclient.o -g -O0

clean:
	rm *.o client server
