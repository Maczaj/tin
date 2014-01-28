#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <assert.h>
#include "fs_libclient.h"

//=================== CONSTANTS ================//

#define DATA_FOR_WRITING "some test data for writing and reading"
#define FILE_NAME "test_file"

using namespace std;

char * srvaddr;
int srvhndl;


//============== TEST CASES ======================//
void test1();
void test2();
void test3();
void test4();
void test5();

//=================== MAIN ========================//
int main(int argc, char *argv[]){
  if( argc == 0 ) {
    cout << "Server address not specified, aborting execution " << endl;
    return 2;
  }
  srvaddr = argv[1];
  char * result = strstr ( srvaddr , ":" );

  if( result == NULL || (result + 1) == '\0'){
    cout << "Port not specified, exiting now... " << endl;
    return 1;
  }

  cout << "Client is running...." << endl;

  srvhndl = fs_open_server(srvaddr);

  if (srvhndl == -1)
    return -1;

  test1();
  test2();
  test3();
  test4();
  test5();

  fs_close_server(srvhndl);
  cout << "Test Suite completed" << endl;
  return 0;
}

/**
  test fs_open
*/
void test1(){
  char * fileName = (char*)FILE_NAME;

  cout << "Test case 1: Creating new file and opening non-existing file..." << endl;

  //utworzenie
  int fd = fs_open(srvhndl , fileName , CREATE);  //zorientowac czy ta flaga tak wyglada czy inaczej
  assert( fd > 0 );

  fs_close(srvhndl, fd);

  fileName = (char*)"blaknglowegawg"; //takiego na pewno nie ma ;P
  //teraz otwarcie go do odczytu co spowoduje blad ze wzgledu na to ze nie istnieje
  fd = fs_open(srvhndl , fileName, READ);
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
  assert ( result == NO_SUCH_FILE_ERROR );

  int fd = fs_open (srvhndl , (char*)FILE_NAME , WRITE );
  assert (fd > 0 );

  //pisanie do niezablokowanego pliku
  result = fs_write (srvhndl , fd , (void *) DATA_FOR_WRITING , strlen(DATA_FOR_WRITING));
  assert( result == NO_LOCK_ERROR ) ;

  result = fs_lock ( srvhndl , fd , WRITE_LOCK );
  assert ( result == 0 );

  result = fs_write (srvhndl , fd , (void *) DATA_FOR_WRITING , strlen(DATA_FOR_WRITING));
  assert( result == 0 );

  fs_close( srvhndl , fd );
  cout << "OK" << endl;
}

/**
  test fs_read
*/
void test3(){
  cout << "Test case 3: reading from non-existing file, opening file for reading without locking and reading from file...";

  char buffer[200];

  //najpierw czytanie z nieistniejacego
  int result = fs_read ( srvhndl , 999999999 , (void *) buffer , strlen(DATA_FOR_WRITING) );
  assert (result == NO_SUCH_FILE_ERROR );

  int fd = fs_open ( srvhndl , (char*)FILE_NAME , READ );
  assert ( fd > 0 );

  result = fs_read ( srvhndl , fd , (void *) buffer , strlen(DATA_FOR_WRITING) );
  assert ( result == NO_LOCK_ERROR ) ;

  result = fs_lock ( srvhndl , fd , READ_LOCK );
  assert ( result == 0 );

  result = fs_read ( srvhndl , fd , (void *) buffer , strlen(DATA_FOR_WRITING) );
  assert( !strcmp( buffer , DATA_FOR_WRITING ) );

  fs_close(srvhndl , fd) ;
  cout << "OK" << endl;
}

/**
  test fs_lseek
*/
void test4(){
  cout << "Test case 4: Opening file, reading first characters, moving back to beginning and reading again...";
  char buffer[50];
  char second_buffer[50];
  memset(buffer, 0, 50);
  memset(second_buffer, 0, 50);

  int fd = fs_open (srvhndl, (char*)FILE_NAME, READ );
  assert (fd > 0);
  int result = fs_lock( srvhndl, fd , READ_LOCK );
  assert (result == 0) ;

  int read_amount = 5;
  result = fs_read ( srvhndl , fd , (void *) buffer , read_amount );
  assert ( result == 0 ) ;

  result = fs_lseek ( srvhndl , fd , -read_amount , SEEK_CUR ) ;
  assert (result == 0 );
  result = fs_read ( srvhndl , fd , (void *) second_buffer , read_amount );
  assert ( result == 0 );
  cout << "MMMMMMMMMMMMMMMMMM ." << buffer << ". ." << second_buffer << "." << endl;
  assert ( !strcmp ( buffer, second_buffer ) );

  fs_close (srvhndl, fd) ;
  cout << "OK" << endl;
}

/**
  test fs_lock
*/
void test5(){
  cout << "Test case 5a: Opening file, locking for reading , locking for writing, closing...";

  int fd = fs_open (srvhndl, (char*)FILE_NAME, READ );
  assert ( fd > 0 );

  int result = fs_lock( srvhndl, fd , READ_LOCK );
  assert (result == 0 ) ;
  result = fs_lock ( srvhndl , fd , WRITE_LOCK );
  assert ( result == LOCKING_NOT_POSSIBLE_ERROR );

  fs_close( srvhndl, fd) ;
  cout << "OK" << endl;

  cout << "Test case 5b: Opening file, locking for writing, locking for reading, closing...";

  fd = fs_open (srvhndl, (char*)FILE_NAME, READ );
  assert ( fd > 0 ) ;
  result = fs_lock ( srvhndl , fd , WRITE_LOCK );
  assert ( result == 0 ) ;

  result = fs_lock ( srvhndl , fd , READ_LOCK );
  assert ( result == LOCKING_NOT_POSSIBLE_ERROR );

  fs_close( srvhndl , fd );

  cout << "OK";
}
