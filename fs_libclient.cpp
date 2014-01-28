#include "fs_libclient.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "fs_server.h"

#define MAX_SIZE 596

using namespace std;

// concat command, as char, at index 0 and serializedStruct from index 1
char* concatCommandAndSerializedStruct(const char* outputSerializedStruct, FS_cmdT command) {
  char *bufferToSend = new char[strlen(outputSerializedStruct)+2];
  memset(bufferToSend, 0, strlen(outputSerializedStruct)+2);
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
    perror("socket");
    return 0;
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
    perror("connect");
    return -1;
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
  cout << "HEJ TY: ." << bufferToSend << "." << endl;
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
    cout << "HEJ TY2: ." << bufferToSend << "." << endl;
  if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
    perror("writing on stream socket");

  delete [] bufferToSend;

  //read response
  char *inputSerializedStruct = new char [MAX_SIZE];
  memset(inputSerializedStruct, 0, MAX_SIZE);

  read(srvhndl, inputSerializedStruct, MAX_SIZE);
  cout << "response ." << inputSerializedStruct << "." << endl;
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

  FS_c_write_fileT requestStruct;
  requestStruct.command = FILE_WRITE_REQ;
  requestStruct.fd = fd;

  int bytesSent = 0;
  int bytesToSend = 0;
  const int MAX_BUF = MAX_SIZE+256;
  char *inputSerializedStruct = new char [MAX_BUF];
  char *bufPart;
  std::stringstream outputStringStream;
  int status = 0;

  const int packLength = MAX_SIZE;
  cout << len << endl;
  while (bytesSent < len) {
    bytesToSend = (len-bytesSent >= packLength ? packLength : len-bytesSent + 1) -1;
    // pack request struct
    requestStruct.len = bytesToSend;
    cout << "bytesToSend " << bytesToSend << endl;
    bufPart = new char [packLength];
    memset(bufPart, 0, packLength);

    memcpy(bufPart, ((char *)buf) + bytesSent, bytesToSend);

    bufPart[bytesToSend] = '\0';
    requestStruct.data = bufPart;

    // serialize struct to stringstream
    outputStringStream.str("");
    {
      boost::archive::text_oarchive oa(outputStringStream);
      oa << requestStruct;
    }

    char* bufferToSend =
        concatCommandAndSerializedStruct(outputStringStream.str().c_str(),
            requestStruct.command);

    // send request
      cout << "HEJ TYYYYYYYYYYYYY: ." << bufferToSend << "." << endl;
    if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
      perror("writing on stream socket");

    delete [] bufPart;
    bufPart = 0;
    //read response

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

    bytesSent += bytesToSend;

    status = response.status;

    delete [] bufferToSend;
    bufferToSend = 0;
  }

  delete [] inputSerializedStruct;
  inputSerializedStruct = 0;

  return status;
}

int fs_read(int srvhndl , int fd , void * buf , size_t len){
  cout << "fs_read() called" << endl;

  // pack request struct
  FS_c_read_fileT requestStruct;
  requestStruct.command = FILE_READ_REQ;
  requestStruct.fd = fd;

  int bytesReceived = 0;
  int bytesToReceive = 0;
  const int MAX_BUF = MAX_SIZE+256;
  char *inputSerializedStruct = new char [MAX_BUF];
  char *bufPart;
  std::stringstream outputStringStream;
  int status = 0;

  const int packLength = MAX_SIZE;
  cout << len << endl;
  while (bytesReceived < len) {
    cout << ">>>> " << bytesToReceive << " " << bytesReceived;
    bytesToReceive = (len-bytesReceived >= packLength ? packLength : len-bytesReceived + 1) -1;

    requestStruct.len = bytesToReceive;

    // serialize struct to stringstream
    std::stringstream outputStringStream;
    {
      boost::archive::text_oarchive oa(outputStringStream);
      oa << requestStruct;
    }

    char* bufferToSend = concatCommandAndSerializedStruct(outputStringStream.str().c_str(), requestStruct.command);

    // send request
      cout << "HEJ TY5: ." << bufferToSend << "." << endl;
    if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
      perror("writing on stream socket");

    delete [] bufferToSend;

    //read response
    char *inputSerializedStruct = new char [MAX_SIZE];
    memset(inputSerializedStruct, 0, MAX_SIZE);

    read(srvhndl, inputSerializedStruct, MAX_SIZE);
    cout << "response ." << inputSerializedStruct << "." << endl;
    // deserialize response from string to struct
    FS_s_read_fileT response;
    {
        std::string string(inputSerializedStruct);
        std::stringstream inputStringStream(string);
        // create and open an archive for input
        boost::archive::text_iarchive ia(inputStringStream);
        // read class state from archive
        ia >> response;
    }

    status = response.status;

    if (status < 0) {
      return status;
    }
    strcat((char*)buf, (char*)response.data);
    delete [] (unsigned char*)response.data;
    bytesReceived += bytesToReceive;
  }

  delete [] inputSerializedStruct;


  return status;
}

int fs_lseek(int srvhndl, int fd , long offset , int whence){
  cout << "fs_lseek() called" << endl;

  // pack request struct
  FS_c_lseek_fileT requestStruct;
  requestStruct.command = FILE_LSEEK_REQ;
  requestStruct.fd = fd;
  requestStruct.offset = offset;
  requestStruct.whence = whence;

  // serialize struct to stringstream
  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << requestStruct;
  }

  char* bufferToSend = concatCommandAndSerializedStruct(outputStringStream.str().c_str(), requestStruct.command);

  // send request
    cout << "HEJ TY2: ." << bufferToSend << "." << endl;
  if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
    perror("writing on stream socket");

  delete [] bufferToSend;

  //read response
  char *inputSerializedStruct = new char [MAX_SIZE];
  memset(inputSerializedStruct, 0, MAX_SIZE);

  read(srvhndl, inputSerializedStruct, MAX_SIZE);
  cout << "response ." << inputSerializedStruct << "." << endl;
  // deserialize response from string to struct
  FS_s_lseek_fileT response;
  {
      std::string string(inputSerializedStruct);
      std::stringstream inputStringStream(string);
      // create and open an archive for input
      boost::archive::text_iarchive ia(inputStringStream);
      // read class state from archive
      ia >> response;
  }
  delete [] inputSerializedStruct;

  return response.status;
}

int fs_close(int srvhndl, int fd){
  cout << "fs_close() called" << endl;

  // pack request struct
  FS_c_close_fileT requestStruct;
  requestStruct.command = FILE_CLOSE_REQ;
  requestStruct.fd = fd;

  // serialize struct to stringstream
  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << requestStruct;
  }

  char* bufferToSend = concatCommandAndSerializedStruct(outputStringStream.str().c_str(), requestStruct.command);

  // send request
    cout << "HEJ TY5: ." << bufferToSend << "." << endl;
  if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
    perror("writing on stream socket");

  delete [] bufferToSend;

  //read response
  char *inputSerializedStruct = new char [MAX_SIZE];
  memset(inputSerializedStruct, 0, MAX_SIZE);

  read(srvhndl, inputSerializedStruct, MAX_SIZE);
  cout << "response ." << inputSerializedStruct << "." << endl;
  // deserialize response from string to struct
  FS_s_close_fileT response;
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
  return response.status;
}

int fs_stat(int srvhndl , int fd, struct stat* buff){
  cout << "fs_stat() called" << endl;
  return -10;
}

int fs_lock(int srvhndl , int fd , int mode){
  cout << "fs_lock() called" << endl;

  // pack request struct
  FS_c_lock_fileT requestStruct;
  requestStruct.command = FILE_LOCK_REQ;
  requestStruct.fd = fd;
  requestStruct.lock_type = mode;

  // serialize struct to stringstream
  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << requestStruct;
  }

  char* bufferToSend = concatCommandAndSerializedStruct(outputStringStream.str().c_str(), requestStruct.command);

  // send request
    cout << "HEJ TY4: ." << bufferToSend << "." << endl;
  if (write(srvhndl, bufferToSend, strlen(bufferToSend)) == -1)
    perror("writing on stream socket");

  delete [] bufferToSend;

  //read response
  char *inputSerializedStruct = new char [MAX_SIZE];
  memset(inputSerializedStruct, 0, MAX_SIZE);

  read(srvhndl, inputSerializedStruct, MAX_SIZE);
  cout << "response ." << inputSerializedStruct << "." << endl;
  // deserialize response from string to struct
  FS_s_lock_fileT response;
  {
      std::string string(inputSerializedStruct);
      std::stringstream inputStringStream(string);
      // create and open an archive for input
      boost::archive::text_iarchive ia(inputStringStream);
      // read class state from archive
      ia >> response;
  }
  delete [] inputSerializedStruct;

  return response.status;
}
