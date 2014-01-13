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


/* Preprocessor Directives */

#define NTHREADS 50
#define QUEUE_SIZE 5
#define BUFFER_SIZE 256



pthread_t threadid[NTHREADS]; // Thread pool
// pthread_mutex_t lock;
// int counter = 0;

FS_s_open_fileT* handleFileOpen(int sockfd, char* buffer){
  FS_c_open_fileT newg;
  {
      // create and open an archive for input
      std::stringstream iss;
      std::string string(buffer);
      iss.str(string);
      boost::archive::text_iarchive ia(iss);
      // read class state from archive
      ia >> newg;
      // archive and stream closed when destructors are called
  }
  std::cout << "\nopenfile >>> " << newg.name << " " << newg.flags << std::endl;
  //printf("New message received: %s", buffer);
  buffer--;
  bzero(buffer, BUFFER_SIZE);
  // sprintf(buffer, "Acknowledgement from TID:0x%x", (unsigned int)pthread_self());
  FS_c_open_fileT responseStruct;
  std::stringstream oss;
  {
    boost::archive::text_oarchive oa(oss);
    oa << responseStruct;
  }

  //prepare command+serializedStruct to char*
  std::string s = oss.str();
  // const char* serializedStruct = s.c_str();
  sprintf(buffer, "%s", s.c_str());

  // int rw = write(sockfd, buffer, strlen(buffer));
  int rw = write(sockfd, "OK!\n", 5);

  if (rw < 0)
  {
    perror("Error writing to socket, exiting thread");
    pthread_exit(0);
  }
  // return resultToClient;
}

void* threadworker(void *arg)
{

  int sockfd, rw; // File descriptor and 'read/write' to socket indicator
  char *buffer; // Message buffer
  sockfd = (int) arg; // Getting sockfd from void arg passed in

  buffer = (char *)malloc(BUFFER_SIZE);
  bzero(buffer, BUFFER_SIZE);

  rw = read(sockfd, buffer, BUFFER_SIZE);

  int command = (int)buffer[0] - 48;

  std::cout << command << std::endl;

  buffer++;

  switch (command) {
    case FILE_OPEN_REQ:
    {
      FS_s_open_fileT *resultToClient;
      resultToClient = handleFileOpen(sockfd, buffer);

      break;
    }
    case FILE_CLOSE_REQ:
    {
      FS_c_close_fileT newg;
      {
          // create and open an archive for input
          std::stringstream iss;
          std::string string(buffer);
          iss.str(string);
          boost::archive::text_iarchive ia(iss);
          // read class state from archive
          ia >> newg;
          // archive and stream closed when destructors are called
      }
      std::cout << "\nclosefile >>> " << newg.fd << std::endl;
      printf("New message received: %s", buffer);
      buffer--;
      bzero(buffer, BUFFER_SIZE);
      // sprintf(buffer, "Acknowledgement from TID:0x%x", (unsigned int)pthread_self());

      rw = write(sockfd, buffer, strlen(buffer));

      if (rw < 0)
      {
      perror("Error writing to socket, exiting thread");
      pthread_exit(0);
      }

      break;
    }
  }



  // write(sockfd, "OK!", 4);

  if (rw < 0)
  {
    perror("Error reading form socket, exiting thread");
    pthread_exit(0);
  }


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

