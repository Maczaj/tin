#ifndef FS_SERVER_FILE_H
#define FS_SERVER_FILE_H 1

#include <fstream>
#include <iostream>
#include <string>
#include <list>
#include <mutex>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <time.h>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>

#define SEEK_BEGIN 0
#define SEEK_CURRENT 1
#define SEEK_END_FILE 2

#define READ_LOCK 0
#define WRITE_LOCK 1
#define UNLOCK 2

#define CREATE 1
#define READ 2
#define WRITE 3
#define READWRITE 4
#define APPEND 5

#define NO_SUCH_FILE_ERROR -200999
#define NO_LOCK_ERROR -200998
#define LOCKING_NOT_POSSIBLE_ERROR -200997

using namespace std;

struct Lock {
  int lockMode;
  int lockPID;

  Lock(){}

  Lock (const Lock &obj)
  {
    this->lockMode = obj.lockMode;
    this->lockPID = obj.lockPID;
  }
};

struct FileStream {

  int PID;
  int flags;
  string name;
  fstream stream;

  FileStream(){}

  FileStream (const FileStream &obj)
  {
    this->PID = obj.PID;
    this->stream << obj.stream;
    this->flags = obj.flags;
    this->name = obj.name;

    if(obj.stream.is_open())
    {
      if (this->flags == CREATE)
      {
          this->stream.open(this->name.c_str(), fstream::in | fstream::out | fstream::app);
      }
      else if (this->flags == READ)
      {
        this->stream.open(this->name.c_str(), fstream::in);
      }
      else if (this->flags == WRITE)
      {
        this->stream.open(this->name.c_str(), fstream::out);
      }
      else if (this->flags == READWRITE)
      {
        this->stream.open(this->name.c_str(), fstream::in | fstream::out | fstream::app);
      }
      else if (this->flags == APPEND)
      {
        this->stream.open(this->name.c_str(), fstream::app);
      }
    }
  }
};

struct Files {
  int descriptor;
  string name;
  list<Lock> listLock;
  list<FileStream> listStream;

  Files ()
  {
  }

  Files (const Files &obj)
  {
    this->descriptor = obj.descriptor;
    this->name = obj.name;
    this->listLock = obj.listLock;

    for( list<FileStream>::const_iterator iter=obj.listStream.begin(); iter != obj.listStream.end(); iter++ )
    {
      this->listStream.push_back(*iter);
    }

  }
};

int hf_open(int pid, std::string name, int flags);

int hf_write(int pid, int fd , char * buf , size_t len);

int hf_read(int pid, int fd , char * buf , size_t len);

int hf_lseek(int pid, int fd , long offset , int whence);

int hf_close(int pid, int fd);

int hf_stat(char * fn, struct stat* buff);

int fs_lock(int pid, int fd , int mode);

#endif // FS_SERVER_FILE_H
