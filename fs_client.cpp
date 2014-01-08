#include <iostream>
#include <assert.h>
#include "fs_libclient.h"

//=================== CONSTANTS ================//
#define MAX_SIZE 4096
#define NO_SUCH_FILE_ERROR -200999
#define NO_LOCK_ERROR -200998
#define DATA_FOR_WRITING "some test data for writing and reading"
#define FILE_NAME "test_file"

using namespace std;

char * srvaddr;
int srvhndl;


//============== TEST CASES ======================//
void test1();
void test2();
void test3();

//=================== MAIN ========================//
int main(int argc,char *argv[]){
	cout << "Client is running...." << endl;
	srvaddr = argv[0];
	srvhndl = fs_open_server(srvaddr);
	
	test1();
	test2();
	test3();
	.
	.
	.
	testn();
	
	
	fs_close_server(srvhndl);
	cout << "Test Suite completed" << endl;
	return 0;
}

/**
	test fs_open 
*/
void test1(){
	char * fileName = FILE_NAME;
	
	cout << "Test case 1: Creating new file and opening non-existing file..." ;
	
	//utworzenie
	int fd = fs_open(srvhndl , fileName , O_CREATE);	//zorientowac czy ta flaga tak wyglada czy inaczej
	assert( fd > 0 );
	
	fs_close(srvhndl, fd);
	
	fileName = "blaknglowegawg"; //takiego na pewno nie ma ;P
	//teraz otwarcie go do odczytu co spowoduje blad ze wzgledu na to ze nie istnieje
	fd = fs_open(srvhndl , fileName, O_READ);
	assert ( fd == NO_SUCH_FILE_ERROR);	
	
	cout << "OK" << endl;
}

/**
	test fs_write, przy okazji uzywa tez fs_lock
*/
void test2(){
	cout << "Test case 2: Writing to non-existing file, opening file for writing without locking and writing to a file...";

	//jako ze aplikacja dopiero rusza, zapewne nie ma pliku o takim deskryptorze 
	int result = fs_write( srvhndl , 999999999 , (void *) DATA_FOR_WRITING , strlen(DATA_FOR_WRITING)); 
	assert ( result == NO_SUCH_FILE_ERROR ):
	
	int fd = fs_open (srvhndl , FILE_NAME , O_WRITE );
	assert (fd > 0 );
	
	//pisanie do niezablokowanego pliku
	result = fs_write (srvhndl , fd , (void *) DATA_FOR_WRITING , strlen(DATA_FOR_WRITING));
	assert( result == NO_LOCK_ERROR ) ;
	
	result = fs_lock ( srvhndl , fd , WRITE );
	assert ( result == 0 );
	
	result = fs_write (srvhndl , fd , (void *) DATA_FOR_WRITING , strlen(DATA_FOR_WRITING));
	assert( result == srtlen (DATA_FOR_WRITING) );

	fs_close( srvhndl , fd );
	cout << "OK" << endl;
}

/**
	test fs_read
*/
void test3(){
	cout << "Test case 3: reading from non-existing file, opening file for reading without locking and reading from file" << endl;
	
	//4 kB na bufor, i tak az za, ale....
	char buffer[4096];
	
	//najpierw czytanie z nieistniejacego
	int result = fs_read ( srvhndl , 999999999 , (void *) buffer , strlen(DATA_FOR_WRITING) ):
	assert (result == NO_SUCH_FILE_ERROR );
	
	int fd = fs_open ( srvhndl , FILE_NAME , O_READ );
	assert ( fd > 0 );
	
	result = fs_read ( srvhndl , fd , (void *) buffer , strlen(DATA_FOR_WRITING) ):
	assert ( result == NO_LOCK_ERROR ) ;
	
	result = fs_lock ( srvhndl , fd , READ );
	assert ( result == 0 );
	
	result = fs_read ( srvhndl , fd , (void *) buffer , strlen(DATA_FOR_WRITING) ):
	assert( !strcmp( buffer , DATA_FOR_WRITING ) );
	
	fs_close(srvhndl , fd) ;
	cout << "OK" << endl;
}

