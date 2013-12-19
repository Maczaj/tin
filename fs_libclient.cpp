#include<iostream>
#include "fs_libclient.h"
using namespace std;

int fs_open_server(char *adres_serwera){
	cout << "fs_open_server() called" << endl;
	return -10;
}

int fs_close_server(int srvhndl){
	cout << "fs_close_server() called" << endl;
	return -10;
}

int fs_open(int srvhndl, char *name, int flags){
	cout << "fs_open() called" << endl;
	return -10;
}

int fs_write(int srvhndl , int fd , void * buf , size_t len){
	cout << "fs_write() called" << endl;
	return -10;
}

int fs_read(int srvhndl , int fd , void * buf , size_t len){
	cout << "fs_read() called" << endl;
	return -10;
}

int fs_lseek(int srvhndl, int fd , long offset , int whence){
	cout << "fs_lseek() called" << endl;
	return -10;
}

int fs_close(int srvhndl, int fd){
	cout << "fs_close() called" << endl;
	return -10;
}

int fs_stat(int srvhndl , int fd, struct stat* buff){
	cout << "fs_stat() called" << endl;
	return -10;
}

int fs_lock(int srvhndl , int fd , int mode){
	cout << "fs_lock() called" << endl;
	return -10;
}