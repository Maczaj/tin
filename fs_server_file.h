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

#define SEEK_BEGIN 1
#define SEEK_CURRENT 2
#define SEEK_END_FILE 3


#define CREATE 1
#define READ 2
#define WRITE 3
#define READWRITE 4
#define APPEND 5

#define NO_SUCH_FILE_ERROR -200999
#define NO_LOCK_ERROR -200998
#define LOCKING_NOT_POSSIBLE_ERROR -200997
using namespace std;

mutex lock_mutex;

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
	
	list<Lock> listLock;
	list<FileStream> listStream;

	Files ()
	{
	}

	Files (const Files &obj) 
	{

   		this->descriptor = obj.descriptor;
		
		this->listLock = obj.listLock;
		//this->listStream = obj.listStream;
		for(  list<FileStream>::const_iterator iter=obj.listStream.begin(); iter != obj.listStream.end(); iter++ )
		{
			this->listStream.push_back(*iter);
		}

	}
};


list<Files> fileList;

int hf_open(int pid, int fd,std::string name, int flags);

int hf_write(int pid, int fd , char * buf , size_t len);

int hf_read(int pid, int fd , char * buf , size_t len);

int hf_lseek(int pid, int fd , long offset , int whence);

int hf_close(int pid, int fd);

int hf_stat(char * fn, struct stat* buff);

int fs_lock(int pid, int fd , int mode);
