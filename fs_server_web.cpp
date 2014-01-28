#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdint> //do rzutowania na intptr_t

#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "fs_server.h"
#include "fs_server_file.h"

#define NTHREADS 50
#define QUEUE_SIZE 5
#define BUFFER_SIZE 5000

using namespace std;

pthread_t threadid[NTHREADS]; // Thread pool
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
  //printf("New message received: %s", recievedSerializedStruct);
  // recievedSerializedStruct--;
  // bzero(recievedSerializedStruct, BUFFER_SIZE);

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
  // return ok;
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
  // std::BOOST_LOG_TRIVIAL(info) << "\nwritefile >>> " << recievedStruct.name << " " << recievedStruct.flags << std::endl;
  //printf("New message received: %s", recievedSerializedStruct);
  // recievedSerializedStruct--;
  // bzero(recievedSerializedStruct, BUFFER_SIZE);

  BOOST_LOG_TRIVIAL(info) << recievedStruct.fd << " ." <</* (char *)recievedStruct.data << */". " << recievedStruct.len << endl;
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
  // return ok;
}

int handleLockFile(int sockfd, char* recievedSerializedStruct) {
  FS_c_lock_fileT recievedStruct;
  {
    std::string string(recievedSerializedStruct);
    std::stringstream inputStringStream(string);
    boost::archive::text_iarchive inputArchive(inputStringStream);
    inputArchive >> recievedStruct;
  }
  // BOOST_LOG_TRIVIAL(info) << "\nopenfile >>> " << recievedStruct.name << " " << recievedStruct.flags << endl;
  //printf("New message received: %s", recievedSerializedStruct);
  // recievedSerializedStruct--;
  // bzero(recievedSerializedStruct, BUFFER_SIZE);

  BOOST_LOG_TRIVIAL(info) << "XA1" << endl;
  // int fd = hf_open(tid, *(new string(recievedStruct.name)), recievedStruct.flags);
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
  // return ok;
}

int handleCloseFile(int sockfd, char* recievedSerializedStruct) {
  FS_c_close_fileT recievedStruct;
  {
    std::string string(recievedSerializedStruct);
    std::stringstream inputStringStream(string);
    boost::archive::text_iarchive inputArchive(inputStringStream);
    inputArchive >> recievedStruct;
  }
  // BOOST_LOG_TRIVIAL(info) << "\nopenfile >>> " << recievedStruct.name << " " << recievedStruct.flags << endl;
  //printf("New message received: %s", recievedSerializedStruct);
  // recievedSerializedStruct--;
  // bzero(recievedSerializedStruct, BUFFER_SIZE);

  BOOST_LOG_TRIVIAL(info) << "XA1" << endl;
  // int fd = hf_open(tid, *(new string(recievedStruct.name)), recievedStruct.flags);
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
  // return ok;
}

int handleReadFile(int sockfd, char* recievedSerializedStruct) {
  FS_c_read_fileT recievedStruct;
  {
    std::string string(recievedSerializedStruct);
    std::stringstream inputStringStream(string);
    boost::archive::text_iarchive inputArchive(inputStringStream);
    inputArchive >> recievedStruct;
  }
  // BOOST_LOG_TRIVIAL(info) << "\nopenfile >>> " << recievedStruct.name << " " << recievedStruct.flags << endl;
  //printf("New message received: %s", recievedSerializedStruct);
  // recievedSerializedStruct--;
  // bzero(recievedSerializedStruct, BUFFER_SIZE);

  BOOST_LOG_TRIVIAL(info) << "XA1" << endl;
  // int fd = hf_open(tid, *(new string(recievedStruct.name)), recievedStruct.flags);

  char* bufFile = new char [recievedStruct.len+1];
  memset(bufFile, 0, recievedStruct.len+1);
  int status = hf_read(tid, recievedStruct.fd, bufFile, recievedStruct.len);

  BOOST_LOG_TRIVIAL(info) << status << " XA2" << endl;
  FS_s_read_fileT responseStruct;

  responseStruct.command = FILE_READ_RES;
  responseStruct.status = status;
  responseStruct.data = bufFile;
  BOOST_LOG_TRIVIAL(info) << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << recievedStruct.len << ". ." << strlen(bufFile) << ". " << endl;
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
  // BOOST_LOG_TRIVIAL(info) << "\nopenfile >>> " << recievedStruct.name << " " << recievedStruct.flags << endl;
  //printf("New message received: %s", recievedSerializedStruct);
  // recievedSerializedStruct--;
  // bzero(recievedSerializedStruct, BUFFER_SIZE);

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
  // return ok;
}

void* threadworker(void *arg)
{

  int sockfd, rw; // File descriptor and 'read/write' to socket indicator
  char *buffer; // Message buffer
  sockfd = (int) arg; // Getting sockfd from void arg passed in
  bool closeServer = false;

  // buffer = (char *)malloc(BUFFER_SIZE);
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

  /* Variable declarations */

  int serv_sockfd, new_sockfd; //Socket identifiers for server and incoming clients
  struct addrinfo flags; // Params used to establish listening socket
  struct addrinfo *host_info; // Resultset for localhost address info, set by getaddrinfo()

  socklen_t addr_size; // Client address size since we use sockaddr_storage struct to store
                       // client info coming in, not using addrinfo as done for host (local)
                       // by calling getaddrinfo for resolution, which stores results in
                       // the more convenient addrinfo struct

  struct sockaddr_storage client; // Sockaddr storage struct is larger than sockaddr_in,
                                  // can be used both for IPv4 and IPv6

  pthread_attr_t attr; // Thread attribute
  int i; // Thread iterator

  /* Start of main program */

  // if (argc < 2) {
  //   fprintf(stderr,"Error: no port provided\n");
  //   exit(-1);
  // }

  memset(&flags, 0, sizeof(flags));
  flags.ai_family = AF_UNSPEC; // Use IPv4 or IPv6, whichever
  flags.ai_socktype = SOCK_STREAM; // TCP
  flags.ai_flags = AI_PASSIVE; // Set address for me

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

  freeaddrinfo(host_info); // Don't need this struct anymore

  pthread_attr_init(&attr); // Creating thread attributes
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // FIFO scheduling for threads
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Don't want threads (particualrly main)
                                                               // waiting on each other


  listen(serv_sockfd, QUEUE_SIZE); // Pass in socket file descriptor and the size of the backlog queue
                                   // (how many pending connections can be in queue while another request
                                   // is handled)

  addr_size = sizeof(client);
  i = 0;

  while (1)
  {
    if (i == NTHREADS) // So that we don't access a thread out of bounds of the thread pool
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
    sleep(0); // Giving threads some CPU time
  }

  return 0;
}
