#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "fs_server.h"
#include "fs_server_file.h"

#define NTHREADS 50
#define QUEUE_SIZE 5
#define BUFFER_SIZE 5000

using namespace std;

pthread_t threadid[NTHREADS];
pthread_mutex_t lockTid;
int tid = 0;

int handleOpenFile(int sockfd, char* recievedSerializedStruct) {
  FS_c_open_fileT recievedStruct;
  {
    std::string string(recievedSerializedStruct);
    std::stringstream inputStringStream(string);
    boost::archive::text_iarchive inputArchive(inputStringStream);
    inputArchive >> recievedStruct;
  }
  BOOST_LOG_TRIVIAL(info) << "\nopenfile >>> " << recievedStruct.name << " " << recievedStruct.flags << endl;

  BOOST_LOG_TRIVIAL(info) << "XA1" << endl;
  int fd = hf_open(tid, *(new string(recievedStruct.name)), recievedStruct.flags);
BOOST_LOG_TRIVIAL(info) << fd << " XA2" << endl;
  FS_s_open_fileT responseStruct;

  responseStruct.command = FILE_OPEN_RES;
  responseStruct.fd = fd;

  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << responseStruct;
  }

  int rw = write(sockfd, (char *)outputStringStream.str().c_str(), outputStringStream.str().length());

  if (rw < 0)
  {
    perror("Error writing to socket, exiting thread");
    pthread_exit(0);
  }
  return 0;
}

int handleWriteFile(int sockfd, char* recievedSerializedStruct) {
  BOOST_LOG_TRIVIAL(info) << "handleWriteFile" << endl;
  FS_c_write_fileT recievedStruct;
  {
    std::string string(recievedSerializedStruct);
    std::stringstream inputStringStream(string);
    boost::archive::text_iarchive inputArchive(inputStringStream);
    inputArchive >> recievedStruct;
  }

  BOOST_LOG_TRIVIAL(info) << recievedStruct.fd << ". " << recievedStruct.len << endl;
  int status = hf_write(tid, recievedStruct.fd, (char *)recievedStruct.data, recievedStruct.len);

  delete [] (unsigned char *)recievedStruct.data;

  FS_s_write_fileT responseStruct;

  responseStruct.command = FILE_WRITE_RES;
  responseStruct.status = status;
  responseStruct.written_len = recievedStruct.len;

  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << responseStruct;
  }

  int rw = write(sockfd, (char *)outputStringStream.str().c_str(), outputStringStream.str().length());
  BOOST_LOG_TRIVIAL(info) << "hej JA!" << endl;

  if (rw < 0)
  {
    perror("Error writing to socket, exiting thread");
    pthread_exit(0);
  }
  return 0;
}

int handleLockFile(int sockfd, char* recievedSerializedStruct) {
  FS_c_lock_fileT recievedStruct;
  {
    std::string string(recievedSerializedStruct);
    std::stringstream inputStringStream(string);
    boost::archive::text_iarchive inputArchive(inputStringStream);
    inputArchive >> recievedStruct;
  }

  BOOST_LOG_TRIVIAL(info) << "XA1" << endl;
  int status = fs_lock(tid, recievedStruct.fd , recievedStruct.lock_type);
BOOST_LOG_TRIVIAL(info) << status << " XA2" << endl;
  FS_s_lock_fileT responseStruct;

  responseStruct.command = FILE_OPEN_RES;
  responseStruct.status = status;

  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << responseStruct;
  }

  int rw = write(sockfd, (char *)outputStringStream.str().c_str(), outputStringStream.str().length());

  if (rw < 0)
  {
    perror("Error writing to socket, exiting thread");
    pthread_exit(0);
  }
  return 0;
}

int handleCloseFile(int sockfd, char* recievedSerializedStruct) {
  FS_c_close_fileT recievedStruct;
  {
    std::string string(recievedSerializedStruct);
    std::stringstream inputStringStream(string);
    boost::archive::text_iarchive inputArchive(inputStringStream);
    inputArchive >> recievedStruct;
  }

  BOOST_LOG_TRIVIAL(info) << "XA1" << endl;
  int status = hf_close(tid, recievedStruct.fd);
BOOST_LOG_TRIVIAL(info) << status << " XA2" << endl;
  FS_s_close_fileT responseStruct;

  responseStruct.command = FILE_CLOSE_RES;
  responseStruct.status = status;

  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << responseStruct;
  }

  int rw = write(sockfd, (char *)outputStringStream.str().c_str(), outputStringStream.str().length());

  if (rw < 0)
  {
    perror("Error writing to socket, exiting thread");
    pthread_exit(0);
  }
  return 0;
}

int handleReadFile(int sockfd, char* recievedSerializedStruct) {
  FS_c_read_fileT recievedStruct;
  {
    std::string string(recievedSerializedStruct);
    std::stringstream inputStringStream(string);
    boost::archive::text_iarchive inputArchive(inputStringStream);
    inputArchive >> recievedStruct;
  }

  BOOST_LOG_TRIVIAL(info) << "XA1" << endl;

  char* bufFile = new char [recievedStruct.len+1];
  memset(bufFile, 0, recievedStruct.len+1);
  int status = hf_read(tid, recievedStruct.fd, bufFile, recievedStruct.len);

  BOOST_LOG_TRIVIAL(info) << status << " XA2" << endl;
  FS_s_read_fileT responseStruct;

  responseStruct.command = FILE_READ_RES;
  responseStruct.status = status;
  responseStruct.data = bufFile;
  BOOST_LOG_TRIVIAL(info) << "XX" << recievedStruct.len << ". ." << strlen(bufFile) << ". " << endl;
  responseStruct.read_len = recievedStruct.len;

  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << responseStruct;
  }

  int rw = write(sockfd, (char *)outputStringStream.str().c_str(), outputStringStream.str().length());

  delete [] bufFile;

  if (rw < 0)
  {
    perror("Error writing to socket, exiting thread");
    pthread_exit(0);
  }
}

int handleLseekFile(int sockfd, char* recievedSerializedStruct) {
  FS_c_lseek_fileT recievedStruct;
  {
    std::string string(recievedSerializedStruct);
    std::stringstream inputStringStream(string);
    boost::archive::text_iarchive inputArchive(inputStringStream);
    inputArchive >> recievedStruct;
  }

  BOOST_LOG_TRIVIAL(info) << "XA1" << endl;
  int status = hf_lseek(tid, recievedStruct.fd, recievedStruct.offset, recievedStruct.whence);
BOOST_LOG_TRIVIAL(info) << status << " XA2" << endl;
  FS_s_lseek_fileT responseStruct;

  responseStruct.command = FILE_LSEEK_RES;
  responseStruct.status = status;

  std::stringstream outputStringStream;
  {
    boost::archive::text_oarchive oa(outputStringStream);
    oa << responseStruct;
  }

  int rw = write(sockfd, (char *)outputStringStream.str().c_str(), outputStringStream.str().length());

  if (rw < 0)
  {
    perror("Error writing to socket, exiting thread");
    pthread_exit(0);
  }
  return 0;
}

void* threadworker(void *arg)
{

  int sockfd, rw; // File descriptor and 'read/write' to socket indicator
  char *buffer; // Message buffer
  sockfd = (int) arg; // Getting sockfd from void arg passed in
  bool closeServer = false;

  buffer = new char [BUFFER_SIZE];
  bzero(buffer, BUFFER_SIZE);

  pthread_mutex_lock (&lockTid);
  printf("Current tid value: %d, upping by 1...\n", tid);
  tid++;
  pthread_mutex_unlock (&lockTid);

  //petla oczekujaca na kolejne komendy
  while(1) {
    memset(buffer, 0, BUFFER_SIZE);
    rw = read(sockfd, buffer, BUFFER_SIZE);

    int command = (int)buffer[0] - 48;
    BOOST_LOG_TRIVIAL(info) << "Request : >" << buffer << "<" <<endl;
    buffer++;

    switch (command) {
      case FILE_OPEN_REQ:
      {
        BOOST_LOG_TRIVIAL(info) << "command FILE_OPEN_REQ received" << endl;
        handleOpenFile(sockfd, buffer);
        break;
      }
      case FILE_CLOSE_REQ:
      {
        BOOST_LOG_TRIVIAL(info) << "command FILE_CLOSE_REQ received" << endl;
        handleCloseFile(sockfd, buffer);
        break;
      }
      case FILE_READ_REQ:
      {
        BOOST_LOG_TRIVIAL(info) << "command FILE_READ_REQ received" << endl;
        handleReadFile(sockfd, buffer);
        break;
      }
      case FILE_WRITE_REQ:
      {
        BOOST_LOG_TRIVIAL(info) << "command FILE_WRITE_REQ received" << endl;
        handleWriteFile(sockfd, buffer);
        break;
      }
      case FILE_STAT_REQ:
      {
        BOOST_LOG_TRIVIAL(info) << "command FILE_STAT_REQ received" << endl;
        // handleStatFile(sockfd, buffer);
        break;
      }
      case FILE_LOCK_REQ:
      {
        BOOST_LOG_TRIVIAL(info) << "command FILE_LOCK_REQ received" << endl;
        handleLockFile(sockfd, buffer);
        break;
      }
      case FILE_LSEEK_REQ:
      {
        BOOST_LOG_TRIVIAL(info) << "command FILE_LSEEK_REQ received" << endl;
        handleLseekFile(sockfd, buffer);
        break;
      }
      case CLOSE_SRV_REQ:
      {
        BOOST_LOG_TRIVIAL(info) << endl << "command CLOSE_SRV_REQ received" << endl;
        closeServer = true;
        break;
      }
      default:
      {
        BOOST_LOG_TRIVIAL(info) << "unknown command received!!!" << endl;
        BOOST_LOG_TRIVIAL(info) << "closing server!!!" << endl;
        closeServer = true;
      }
    }
    buffer--;

    if (rw < 0)
    {
      perror("Error reading form socket, exiting thread");
      pthread_exit(0);
    }
    if (closeServer)
    {
      break;
    }
  }

  delete [] buffer;

  close(sockfd);
  printf("TID:0x%x served request, exiting thread\n", (unsigned int)pthread_self());
  pthread_exit(0);
}

int start(char *argv[])
{

  int serv_sockfd, new_sockfd;
  struct addrinfo flags;
  struct addrinfo *host_info;

  socklen_t addr_size;

  struct sockaddr_storage client;

  pthread_attr_t attr;
  int i;

  memset(&flags, 0, sizeof(flags));
  flags.ai_family = AF_UNSPEC;
  flags.ai_socktype = SOCK_STREAM;
  flags.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, argv[1], &flags, &host_info) < 0)
  {
    perror("Couldn't read host info for socket start");
    exit(-1);
  }

  serv_sockfd = socket(host_info->ai_family, host_info->ai_socktype, host_info->ai_protocol);

  if (serv_sockfd < 0)
  {
    perror("Error opening socket");
    exit(-1);
  }

  if (bind(serv_sockfd, host_info->ai_addr, host_info->ai_addrlen) < 0)
  {
    perror("Error on binding");
    exit(-1);
  }

  freeaddrinfo(host_info);

  pthread_attr_init(&attr);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  listen(serv_sockfd, QUEUE_SIZE);

  addr_size = sizeof(client);
  i = 0;

  while (2)
  {
    if (i == NTHREADS)
    {
      i = 0;
    }

    new_sockfd = accept(serv_sockfd, (struct sockaddr *) &client, &addr_size);

    if (new_sockfd < 0)
    {
      perror("Error on accept");
      exit(-1);
    }
    printf("eh\n");
    pthread_create(&threadid[i++], &attr, &threadworker, (void *) new_sockfd);
    sleep(0);
  }

  return 0;
}
