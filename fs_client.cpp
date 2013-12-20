#include <iostream>
#include <assert.h>
#include "fs_libclient.h"

#define MAX_SIZE 4096

using namespace std;

char * srvaddr;
int srvhndl;


void test1();


int main(int argc,char *argv[]){
	cout << "Client is running...." << endl;
	srvaddr = argv[0];
	srvhndl = fs_open_server(srvaddr);
	
	test1();
	test2();
	.
	.
	.
	.
	testn();
	
	
	fs_close_server(srvhndl);
	cout << "Test Suite completed" << endl;
	return 0;
}

/**
	test polegajacy na polaczeniu, utworzeniu pliku, zapisaniu czegos, rozlaczeniu, ponownym polaczeniu i sprawdzeniu czy odczytane bedzie rowne zapisanemu
*/
void test1(){
	char * fileName = "test1";
	
	cout << "Test case 1: Connecting, writing, disconnecting" << endl;
	
	int fd = fs_open(srvhndl , fileName , O_CREATE);	//zorientowac czy ta flaga tak wyglada czy inaczej
	
	assert( fd > 0 );
	
	fs_lock(srvhndl , fd , WRITE);
	
	char * data[MAX_SIZE];
	strcpy("pierwsza linijka testowego tekstu zobaczymy co z tego wyjdzie huehue" , data);
	fs_write(srvhndl , fd , (void *) data , strlen(data)); //dodac tutaj badanie wartosci zwracanej
	
	fs_close(srvhndl , fd);
	
	fs_open(srvhndl , fileName, O_READ); //moze inaczej ta flaga ma wygladac?
	char * readData[MAX_SIZE];
	fs_read(srvhndl , fd , (void *) readData, MAX_SIZE);
	
	assert( !strcmp( data, readData));	
}