#include <iostream>
#include "fs_libclient.h"

using namespace std;

int fs_open_server(char *adres_serwera){
	cout << "fs_open_server() called" << endl;

  int sockd;
  struct sockaddr_in serv_name;
  int status;

  sockd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockd == -1)
  {
    return -errno;
  }

  /* server address */
  serv_name.sin_family = AF_INET;
  inet_aton(argv[1], &serv_name.sin_addr);
  serv_name.sin_port = htons(atoi(adres_serwera));

  /* connect to the server */
  status = connect(sockd, (struct sockaddr*)&serv_name, sizeof(serv_name));
  if (status == -1)
  {
    return -errno;
  }

	return sockd;
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
