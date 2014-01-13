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

// concat command, as char, at index 0 and serializedStruct from index 1
char* concatCommandAndSerializedStruct(const char* outputSerializedStruct, FS_cmdT command) {
  char *bufferToSend = new char[strlen(outputSerializedStruct)];
  memset(bufferToSend, 0, strlen(outputSerializedStruct));
  bufferToSend[0] = command + 48;
  strcat(bufferToSend, outputSerializedStruct);
  return bufferToSend;
}

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

  //prepare struct to serialize
  FS_c_close_srvrT requestStruct;
  requestStruct.command = CLOSE_SRV_REQ;

  //serialize struct to stringstream
  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << requestStruct;
  }

  char* bufferToSend = concatCommandAndSerializedStruct(outputStringStream.str().c_str(), requestStruct.command);

  //send struct to server
  if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
    perror("writing on stream socket");

  delete [] bufferToSend;

  int res = close(srvhndl);
  return res;
}

int fs_open(int srvhndl, char *name, int flags){
  cout << "fs_open() called" << endl;

  // pack request struct
  FS_c_open_fileT requestStruct;
  requestStruct.command = FILE_OPEN_REQ;
  requestStruct.name = *(new string(name));
  requestStruct.flags = flags;

  // serialize struct to stringstream
  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << requestStruct;
  }

  char* bufferToSend = concatCommandAndSerializedStruct(outputStringStream.str().c_str(), requestStruct.command);

  // send request
  if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
    perror("writing on stream socket");

  delete [] bufferToSend;

  //read response
  const int MAX_BUF = 4096;
  char *inputSerializedStruct = new char [MAX_BUF];
  memset(inputSerializedStruct, 0, MAX_BUF);

  read(srvhndl, inputSerializedStruct, MAX_BUF);

  // deserialize response from string to struct
  FS_s_open_fileT response;
  {
      std::string string(inputSerializedStruct);
      std::stringstream inputStringStream(string);
      // create and open an archive for input
      boost::archive::text_iarchive ia(inputStringStream);
      // read class state from archive
      ia >> response;
  }
  delete [] inputSerializedStruct;

  // unpack request struct

  return response.fd;
}

int fs_write(int srvhndl , int fd , void * buf , size_t len){
  cout << "fs_write() called" << endl;

  // pack request struct
  FS_c_write_fileT requestStruct;
  requestStruct.command = FILE_WRITE_REQ;
  requestStruct.fd = fd;
  requestStruct.len = len;
  requestStruct.data = buf;

  // serialize struct to stringstream
  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << requestStruct;
  }

  char* bufferToSend = concatCommandAndSerializedStruct(outputStringStream.str().c_str(), requestStruct.command);

  // send request
  if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
    perror("writing on stream socket");

  delete [] bufferToSend;

  //read response
  const int MAX_BUF = 4096;
  char *inputSerializedStruct = new char [MAX_BUF];
  memset(inputSerializedStruct, 0, MAX_BUF);

  read(srvhndl, inputSerializedStruct, MAX_BUF);

  // deserialize response from string to struct
  FS_s_write_fileT response;
  {
      std::string string(inputSerializedStruct);
      std::stringstream inputStringStream(string);
      boost::archive::text_iarchive ia(inputStringStream);
      ia >> response;
  }
  delete [] inputSerializedStruct;

  cout << "response:" << endl;
  cout << response.status << endl;
  cout << response.written_len << endl;

  // unpack request struct

  // return response.;

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
