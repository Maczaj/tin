// #include <iostream>
#include "fs_libclient.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "fs_server.h"

using namespace std;

int fs_open_server(char* adres_serwera){
  cout << "fs_open_server() called" << endl;

  int sockd;
  struct sockaddr_in serv_name;
  int status;

  sockd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockd == -1)
  {
    perror("socket"); // DEBUG
    return 0;//-errno;
  }
  char * srvport = strstr(adres_serwera, ":") + 1;
  // strcpy(srvport, "12345");
  /* server address */
  serv_name.sin_family = AF_INET;
  char ip[16];
  memcpy( ip, adres_serwera, strstr(adres_serwera, ":") - adres_serwera);
  ip[strstr(adres_serwera, ":") - adres_serwera] = '\0';
  inet_aton(ip, &serv_name.sin_addr);
  serv_name.sin_port = htons(atoi((srvport)));

  /* connect to the server */
  status = connect(sockd, (struct sockaddr*)&serv_name, sizeof(serv_name));
  if (status == -1)
  {
    perror("connect"); // DEBUG
    return -1;//-errno;
  }
  return sockd;
}

int fs_close_server(int srvhndl){
  cout << "fs_close_server() called" << endl;
  int res = close(srvhndl);
  return res;
}

int fs_open(int srvhndl, char *name, int flags){
  cout << "fs_open() called" << endl;

  FS_c_open_fileT requestStruct;
  requestStruct.command = FILE_OPEN_REQ;
  requestStruct.name = *(new string(name));
  // requestStruct.name = name;
  requestStruct.flags = flags;

  //serialize struct to stringstream
  std::stringstream oss;
  {
    boost::archive::text_oarchive oa(oss);
    oa << requestStruct;
  }

  //prepare command+serializedStruct to char*
  std::string s = oss.str();
  const char* serializedStruct = s.c_str();
  char bufferToSend[80];
  memset(bufferToSend, 0, 80);
  bufferToSend[0] = requestStruct.command+48;
  strcat(bufferToSend, serializedStruct);

  // printf("stream >>%s<<, sizeof(stream) = %d\n", str, strlen(str));

  //send struct to server
  if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
    perror("writing on stream socket");

  //read response
  char buf[80];
  #define MAX_BUF 80
  memset(buf, 0, sizeof(buf));
  read(srvhndl, buf, MAX_BUF);
  cout << "Odpowiedz serwera " << buf << endl;

  return 0;
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
