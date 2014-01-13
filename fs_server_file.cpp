#include "fs_server_file.h"
using namespace std;


int hf_open(int pid, int fd, string name, int flags)
{
	

	Files file;
	file.descriptor = fd;
	

	FileStream streamStructure;

	streamStructure.flags = flags;
	streamStructure.PID = pid;
	streamStructure.name = name;

	if (flags == CREATE)
	{
		lock_mutex.lock();

			for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
			{
				if (iter->descriptor == fd)
				{
					if (iter->listLock.size() != 0)
					{
						cout << "CREATE error caused by:file exist and once of locks ENABLE\n";
						lock_mutex.unlock();
						return -1;
					}

					break;
				}
			}

		lock_mutex.unlock();

  		streamStructure.stream.open(streamStructure.name.c_str(), fstream::in | fstream::out | fstream::app);
		cout << "File opened with flag: CREATE\n";
	}
	else if (flags == READ)
	{
		streamStructure.stream.open(streamStructure.name.c_str(), fstream::in);

		if(!streamStructure.stream.is_open())
		{
			cout<< "File to read does not exist\n";
			return -2;
		}
		cout << "File opened with flag: READ\n";
	}
	else if (flags == WRITE)
	{
		streamStructure.stream.open(streamStructure.name.c_str(), fstream::out);
		cout << "File opened with flag: WRITE\n";
	}
	else if (flags == READWRITE)
	{
		streamStructure.stream.open(streamStructure.name.c_str(), fstream::in | fstream::out | fstream::app);
		cout << "File opened with flags: READ and WRITE\n";
	}
	else if (flags == APPEND)
	{
		streamStructure.stream.open(streamStructure.name.c_str(), fstream::app);
		cout << "File opened with flags: APPEND\n";
	}
	else
	{
		cout<< "Bad flag";
		return -1;
	}

	file.listStream.push_back(streamStructure);
	fileList.push_back(file);
	
	return 0;
}

int hf_close(int pid, int fd)
{
	for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
	{
		if (iter->descriptor == fd)
		{
			for( list<FileStream>::iterator iterStream=iter->listStream.begin(); iterStream != iter->listStream.end(); iterStream++ )
			{
				if (iterStream->PID == pid)
				{
					iterStream->stream.close();
					cout <<"Stream closed for PID: " <<pid <<" and FILE: "<<fd <<endl;  
					break;
				}	
			}

		

		}
	}

	return 0;
}

int hf_write(int pid, int fd , char * buf , size_t len)
{
	lock_mutex.lock();

		for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
		{
			if (iter->descriptor == fd)
			{
				if (iter->listLock.size() != 0)
				{
					cout << "WRITE error caused by:once of locks ENABLE\n";
					lock_mutex.unlock();
					return -1;
				}

				break;
			}
		}


	lock_mutex.unlock();

	for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
	{
		if (iter->descriptor == fd)
		{
			for( list<FileStream>::iterator iterStream=iter->listStream.begin(); iterStream != iter->listStream.end(); iterStream++ )
			{
				if (iterStream->PID == pid)
				{
					
					iterStream->stream.write(buf, len);
					cout << "Wrote to file with descriptor: " <<fd <<" and PID: "<<pid <<endl;  
					break;
				}	
			}
		}
	}

	return 0;
}

int hf_read(int pid, int fd , char * buf , size_t len)
{
	lock_mutex.lock();

		for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
		{
			if (iter->descriptor == fd)
			{
				for( list<Lock>::iterator iterLock=iter->listLock.begin(); iterLock != iter->listLock.end(); iterLock++ )
				{
					if (iterLock->lockMode == WRITE)
					{
						cout << "READ error caused by:WRITE_LOCK ENABLE\n";
						lock_mutex.unlock();
						return -1;
					}
					
				}

				break;
			}
		}


	lock_mutex.unlock();



	for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
	{
		if (iter->descriptor == fd)
		{
			for( list<FileStream>::iterator iterStream=iter->listStream.begin(); iterStream != iter->listStream.end(); iterStream++ )
			{
				if (iterStream->PID == pid)
				{
					
					iterStream->stream.read(buf, len);
					cout << "Read file with descriptor: " <<fd <<" and PID: "<<pid <<endl;  
					break;
				}	
			}
		}
	}

	return 0;
}


int hf_stat(char * fn, struct stat* buff)
{

  	int fd;

  	if ((fd = creat(fn, S_IWUSR)) < 0)
    		perror("creat() error");
  	else {
    		if (fstat(fd, buff) != 0)
		{
      			perror("fstat() error");
		}
   		 else {
			
     			cout<<"fstat() returned:\n";
     			cout <<"Mode:\t\t\t"<<buff->st_mode <<endl;
      			cout <<"Size:\t\t\t"<<buff->st_size <<endl;
      			cout <<"Block size:\t\t"<<buff->st_blksize <<endl;
      			cout <<"Allocated blocks:\t"<<(int) buff->st_blocks <<endl;
      			cout <<"Last access:\t\t"<<ctime(&buff->st_atime);
			cout <<"Last modification:\t"<<ctime(&buff->st_mtime);
			cout <<"Last status change:\t"<<ctime(&buff->st_ctime);
		}

   		close(fd);

  	}
	return 0;
}


int hf_lseek(int pid, int fd , long offset , int whence)
{
	for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
	{
		if (iter->descriptor == fd)
		{
			for( list<FileStream>::iterator iterStream=iter->listStream.begin(); iterStream != iter->listStream.end(); iterStream++ )
			{
				if (iterStream->PID == pid)
				{
					
					if(iterStream->flags == READ)
					{
						if(whence == SEEK_BEGIN)
						{
							if (offset < 0)
							{
								cout <<"Wrong offset value";
								return -1;
							}
							iterStream->stream.seekg(offset,ios_base::beg);
							cout << "Cursor set on pozistion: " <<iterStream->stream.tellg() 		<<endl;				
						}				
						else if(whence == SEEK_CURRENT)
						{
							iterStream->stream.seekg(offset,ios_base::cur);
							cout << "Cursor set on pozistion: " <<iterStream->stream.tellg() <<endl;
						}
						else if(whence == SEEK_END_FILE)
						{
							if (offset > 0)
							{
								cout <<"Wrong offset value";
								return -1;
							}
							iterStream->stream.seekg(offset,ios_base::end);
							cout << "Cursor set on pozistion: " <<iterStream->stream.tellg() <<endl;
						}
					}
					else
					{
						if(whence == SEEK_BEGIN)
						{
							if (offset < 0)
							{
								cout <<"Wrong offset value";
								return -1;
							}
							iterStream->stream.seekp(offset,ios_base::beg);
							cout << "Cursor set on pozistion: " <<iterStream->stream.tellp() <<endl;
						}
						else if(whence == SEEK_CURRENT)
						{
							iterStream->stream.seekp(offset,ios_base::cur);
							cout << "Cursor set on pozistion: " <<iterStream->stream.tellp() <<endl;
						}
						else if(whence == SEEK_END_FILE)
						{
							if (offset > 0)
							{
								cout <<"Wrong offset value";
								return -1;
							}
							iterStream->stream.seekp(offset,ios_base::end);
							cout << "Cursor set on pozistion: " <<iterStream->stream.tellp() <<endl;
						}
					}
					break;
				}	
			}

	
		}
	}

	return 0;
}



int fs_lock(int pid, int fd , int mode)
{
	lock_mutex.lock();
	int temp = 0;

	if (mode == UNLOCK)
	{
		for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
		{
			if (iter->descriptor == fd)
			{
				for( list<Lock>::iterator iterLock=iter->listLock.begin(); iterLock != iter->listLock.end(); iterLock++ )
				{
					if (iterLock->lockPID == pid)
					{
						iter->listLock.erase(iterLock);
						cout << "File unlocked for PID:" <<pid <<endl;
						break;
					}
				}
				
			}
		}	
	}
	else if (mode == READ_LOCK)
	{
		for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
		{
			if (iter->descriptor == fd)
			{
				for( list<Lock>::iterator iterLock=iter->listLock.begin(); iterLock != iter->listLock.end(); iterLock++ )
				{
					if (iterLock->lockMode == WRITE)
					{
						cout << "READ_LOCK error caused by:WRITE LOCK ENABLE\n";
						return -1;
					}
					if (iterLock->lockMode == READ)
					{
						if (iterLock->lockPID == pid)
						{
							temp = 1;
							break;
						}
					}
					
				}

				if (temp)
				{	
					cout << "OK\n";
					return 0;
				}
				else
				{
					Lock lock;
					lock.lockMode = READ_LOCK;
					lock.lockPID = pid;
					iter->listLock.push_back(lock);
					cout << "OK\n";
					return 0;
				}
				break;
			}
		}	
	}
	else if (mode == WRITE_LOCK)
	{
		for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
		{
			if (iter->descriptor == fd)
			{
				if (iter->listLock.size() != 0)
				{	
					cout << "WRITE_LOCK error caused by: another lock ENABLE\n";
					return -1;
				}
				else
				{
					Lock lock;
					lock.lockMode = READ_LOCK;
					lock.lockPID = pid;
					iter->listLock.push_back(lock);
					cout << "OK\n";
					return 0;
				}
				break;
			}
		}	
	}

	lock_mutex.unlock();
	return 0;
}
int main(){
	
	cout << "Server is running...." << endl;
	hf_open(1,1,"a111", CREATE);
	//hf_open(2,"a222", READ);
	//hf_open(3,"a333", WRITE);
	//hf_open(4,"a444", READWRITE);

	char* buffer = (char *) "1234567890";

	hf_write(1,1 , buffer , sizeof(buffer));
	struct stat info;

	hf_stat((char *) "a111", &info);

	buffer = (char *) "aaaaaa";
	hf_lseek(1,1,5,SEEK_BEGIN);
	hf_write(1,1 , buffer , sizeof(buffer));
	

	hf_close(1,1);
	//hf_close(2);
	//hf_close(3);
	//hf_close(4);
	return 0;
}
