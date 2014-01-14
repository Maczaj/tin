#include "fs_server_file.h"
#include <cstring>
using namespace std;

list<Files> fileList;
mutex lock_mutex;

int hf_open(int pid, string name, int flags)
{


	Files file;
	


	FileStream streamStructure;

	streamStructure.flags = flags;
	streamStructure.PID = pid;
	streamStructure.name = name;

	if (flags == CREATE)
	{
		lock_mutex.lock();

			for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
			{
				if (iter->name == name)
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


			
  			streamStructure.stream.open(streamStructure.name.c_str(), fstream::in | fstream::out | fstream::trunc);
		
			file.descriptor = fileList.size() +1;
			file.name = name;
			file.listStream.push_back(streamStructure);
			fileList.push_back(file);
			cout << "File opened with flag: CREATE\t FD: "<<file.descriptor<<endl;

		lock_mutex.unlock();
		return file.descriptor;


	}
	else if (flags == READ)
	{
		streamStructure.stream.open(streamStructure.name.c_str(), fstream::in);

		if(!streamStructure.stream.is_open())
		{
			cout<< "File to read does not exist\n";
			return -200999;
		}
		
		for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
			{
				if (iter->name == name)
				{
					iter->listStream.push_back(streamStructure);
					cout << "File opened with flag: READ FD: "<<iter->descriptor<<endl;
					return iter->descriptor;
				}
		}
		
	}
	else if (flags == WRITE)
	{
		streamStructure.stream.open(streamStructure.name.c_str(), fstream::out);

		lock_mutex.lock();

			for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
			{
				if (iter->name == name)
				{
					iter->listStream.push_back(streamStructure);
					cout << "File opened with flag: WRITE\n";
					lock_mutex.unlock();
					return iter->descriptor;
			
				}
			}

			file.descriptor = fileList.size() +1;
			file.name = name;
			file.listStream.push_back(streamStructure);
			fileList.push_back(file);

		lock_mutex.unlock();
		return file.descriptor;
			

	}
	else if (flags == READWRITE)
	{
		streamStructure.stream.open(streamStructure.name.c_str(), fstream::in | fstream::out | fstream::app);

		lock_mutex.lock();

			for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
			{
				if (iter->name == name)
				{
					iter->listStream.push_back(streamStructure);
					cout << "File opened with flags: READ and WRITE\n";
					lock_mutex.unlock();
					return iter->descriptor;
			
				}
			}

			file.descriptor = fileList.size() +1;
			file.name = name;
			file.listStream.push_back(streamStructure);
			fileList.push_back(file);

		lock_mutex.unlock();
		return file.descriptor;
		
	}
	else if (flags == APPEND)
	{
		streamStructure.stream.open(streamStructure.name.c_str(), fstream::app);

		lock_mutex.lock();

			for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
			{
				if (iter->name == name)
				{
					iter->listStream.push_back(streamStructure);
					cout << "File opened with flags: APPEND\n";
					lock_mutex.unlock();
					return iter->descriptor;
			
				}
			}

			file.descriptor = fileList.size() +1;
			file.name = name;
			file.listStream.push_back(streamStructure);
			fileList.push_back(file);

		lock_mutex.unlock();
		return file.descriptor;
		
	}
	else
	{
		cout<< "Bad flag";
		return -1;
	}



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
					if (!iterStream->stream.is_open())
					{
						cout << "Stream already closed"<<endl;
						return -1;
					}
					iterStream->stream.close();
					iter->listStream.erase(iterStream);
					cout <<"Stream closed for PID: " <<pid <<" and FILE: "<<fd <<endl;
					return 0;
				}
			}

			cout <<  "File STREAM does not exist for this user"<<endl;
			return -2;


		}
	}
	cout << "File does not exist"<<endl;
	return -200999;
}

int hf_write(int pid, int fd , char * buf , size_t len)
{
	lock_mutex.lock();

		for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
		{
			if (iter->descriptor == fd)
			{
				for(list<Lock>::iterator iterLock=iter->listLock.begin(); iterLock != iter->listLock.end(); iterLock++)
				{
					if (iterLock->lockPID != pid)
					{
						cout << "WRITE error caused by:once of locks ENABLE\n";
						lock_mutex.unlock();
						return -1;
					}
					else
					{
						if (iterLock->lockMode == READ_LOCK)
						{
							cout << "WRITE error caused by:READ_LOCK ENABLE\n";
							lock_mutex.unlock();
							return -200998;
						}
						else if (iterLock->lockMode == WRITE_LOCK)
						{
							for( list<FileStream>::iterator iterStream=iter->listStream.begin(); iterStream != iter->listStream.end(); iterStream++ )
							{
								if (iterStream->PID == pid)
								{
									iterStream->stream.write(buf, len);
									cout << "Wrote to file with descriptor: " <<fd <<" and PID: "<<pid <<endl;
									lock_mutex.unlock();
									return 0;
								}
							}
						}
					}
				}

				break;
			}
		}

	cout << "Missing WRITE_LOCK\n";
	lock_mutex.unlock();
	
	return -200998;
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
					if (iterLock->lockMode == WRITE_LOCK)
					{
						cout << "READ error caused by:WRITE_LOCK ENABLE PID: "<<pid<<endl;;
						lock_mutex.unlock();
						return -200998;
					}
					else if (iterLock->lockMode == READ_LOCK)
					{
						if (iterLock->lockPID == pid)
						{
							for( list<FileStream>::iterator iterStream=iter->listStream.begin(); iterStream != iter->listStream.end(); iterStream++ )
							{
				
								if (iterStream->PID == pid)
								{
  				        				iterStream->stream.read (buf,len);
									cout << "Read file with descriptor: " <<fd <<" and PID: "<<pid <<endl;
									lock_mutex.unlock();
									return 0;
								}
							}
						}
					}

				}

				break;
			}
		}

	cout << "Missing READ_LOCK\n";
	lock_mutex.unlock();
	return -200998;
}


int hf_stat(char * fn, struct stat* buff)
{

  	int fd;

  	if ((fd = creat(fn, S_IWUSR)) < 0)
	{
    		perror("creat() error");
		return -1;
	}
  	else {
    		if (fstat(fd, buff) != 0)
		{
      			perror("fstat() error");
			return -1;
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

			return 0;
		}
	}

	return -200999;
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
					if (iterLock->lockMode == WRITE_LOCK)
					{
						cout << "READ_LOCK error caused by:WRITE LOCK ENABLE\n";
						return -200997;
					}
					if (iterLock->lockMode == READ_LOCK)
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
					cout << "READ_LOCK enable for PID: "<<pid<<endl;
					lock_mutex.unlock();
					return 0;
				}
				else
				{
					Lock lock;
					lock.lockMode = READ_LOCK;
					lock.lockPID = pid;
					iter->listLock.push_back(lock);
					cout << "READ_LOCK enable for PID: "<<pid<<endl;
					lock_mutex.unlock();
					return 0;
				}

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
					lock_mutex.unlock();
					return -200997;
				}
				else
				{
					Lock lock;
					lock.lockMode = WRITE_LOCK;
					lock.lockPID = pid;
					iter->listLock.push_back(lock);
					cout << "WRITE_LOCK enable for PID: "<<pid<<endl;
					lock_mutex.unlock();
					return 0;
				}
			}
		}
	}

	lock_mutex.unlock();
	return 200999;
}

 int main(){

 	cout << "Server is running...." << endl;

	int fd;
 	fd = hf_open(1,"a111", CREATE);
 	char* buffer = (char *) "1234567890";
	fs_lock(1, fd , WRITE_LOCK);
	hf_write(1,fd , buffer , strlen(buffer));
	//fs_lock(1, fd , UNLOCK);
	hf_close(1,fd);

	char* bufread = (char*)calloc(1024,sizeof(char));
	
	fd = hf_open(2,"a111", READ);

	fs_lock(2,fd , READ_LOCK);
	hf_lseek(2, fd , 3 , SEEK_BEGIN);
	hf_read(2, fd , bufread ,5 );
	//fs_lock(1, fd , UNLOCK);

	//fs_lock(2,fd , READ_LOCK);
	//hf_lseek(2, fd , 3 , SEEK_BEGIN);
	//hf_read(2, fd , bufread ,5 );
	cout<<bufread<<endl;
 	hf_close(2,fd);
// 	//hf_close(2);
// 	//hf_close(3);
// 	//hf_close(4);
// 	return 0;
 }
