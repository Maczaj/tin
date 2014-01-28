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
            BOOST_LOG_TRIVIAL(info) << "CREATE error caused by:file exist and once of locks ENABLE\n";
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
      BOOST_LOG_TRIVIAL(info) << "File opened with flag: CREATE\t FD: "<<file.descriptor<<endl;

    lock_mutex.unlock();
    return file.descriptor;


  }
  else if (flags == READ)
  {
    streamStructure.stream.open(streamStructure.name.c_str(), fstream::in);

    if(!streamStructure.stream.is_open())
    {
      BOOST_LOG_TRIVIAL(info)<< "File to read does not exist\n";
      return NO_SUCH_FILE_ERROR;
    }

    for( list<Files>::iterator iter=fileList.begin(); iter != fileList.end(); iter++ )
      {
        if (iter->name == name)
        {
          iter->listStream.push_back(streamStructure);
          BOOST_LOG_TRIVIAL(info) << "File opened with flag: READ FD: "<<iter->descriptor<<endl;
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
          BOOST_LOG_TRIVIAL(info) << "File opened with flag: WRITE\n";
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
          BOOST_LOG_TRIVIAL(info) << "File opened with flags: READ and WRITE\n";
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
          BOOST_LOG_TRIVIAL(info) << "File opened with flags: APPEND\n";
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
    BOOST_LOG_TRIVIAL(info)<< "Bad flag";
    lock_mutex.unlock();
    return -1;
  }
  lock_mutex.unlock();
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
            BOOST_LOG_TRIVIAL(info) << "Stream already closed"<<endl;
            return -1;
          }
          iterStream->stream.close();
          iter->listStream.erase(iterStream);
          BOOST_LOG_TRIVIAL(info) <<"Stream closed for PID: " <<pid <<" and FILE: "<<fd <<endl;
          fs_lock(pid, fd, UNLOCK);
          return 0;
        }
      }

      BOOST_LOG_TRIVIAL(info) <<  "File STREAM does not exist for this user"<<endl;
      return -2;


    }
  }
  BOOST_LOG_TRIVIAL(info) << "File does not exist"<<endl;
  return NO_SUCH_FILE_ERROR;
}

int hf_write(int pid, int fd , char * buf , size_t len)
{
  lock_mutex.lock();
    list<Files>::iterator iter=fileList.begin();
    for(; iter != fileList.end(); iter++ )
    {
      if (iter->descriptor == fd)
      {
        for(list<Lock>::iterator iterLock=iter->listLock.begin(); iterLock != iter->listLock.end(); iterLock++)
        {
          if (iterLock->lockPID != pid)
          {
            BOOST_LOG_TRIVIAL(info) << "WRITE error caused by:once of locks ENABLE\n";
            lock_mutex.unlock();
            return -1;
          }
          else
          {
            if (iterLock->lockMode == READ_LOCK)
            {
              BOOST_LOG_TRIVIAL(info) << "WRITE error caused by:READ_LOCK ENABLE\n";
              lock_mutex.unlock();
              return NO_LOCK_ERROR;
            }
            else if (iterLock->lockMode == WRITE_LOCK)
            {
              for( list<FileStream>::iterator iterStream=iter->listStream.begin(); iterStream != iter->listStream.end(); iterStream++ )
              {
                if (iterStream->PID == pid)
                {
                  iterStream->stream.write(buf, len);
                  BOOST_LOG_TRIVIAL(info) << "Wrote to file with descriptor: " <<fd <<" and PID: "<<pid <<endl;
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
  if (iter == fileList.end()){
    BOOST_LOG_TRIVIAL(info) << "File not found.\n";
    lock_mutex.unlock();
    return NO_SUCH_FILE_ERROR;
  }

  BOOST_LOG_TRIVIAL(info) << "Missing WRITE_LOCK\n";
  lock_mutex.unlock();

  return NO_LOCK_ERROR;
}

int hf_read(int pid, int fd , char * buf , size_t len)
{
  lock_mutex.lock();
    list<Files>::iterator iter=fileList.begin();
    for( ; iter != fileList.end(); iter++ )
    {
      if (iter->descriptor == fd)
      {
        for( list<Lock>::iterator iterLock=iter->listLock.begin(); iterLock != iter->listLock.end(); iterLock++ )
        {
          if (iterLock->lockMode == READ_LOCK || iterLock->lockMode == WRITE_LOCK)
          {
            if (iterLock->lockPID == pid)
            {
              for( list<FileStream>::iterator iterStream=iter->listStream.begin(); iterStream != iter->listStream.end(); iterStream++ )
              {

                if (iterStream->PID == pid)
                {
                          iterStream->stream.read (buf,len);
                  BOOST_LOG_TRIVIAL(info) << "Read file with descriptor: " <<fd <<" and PID: "<<pid <<endl;
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
  if (iter == fileList.end()){
    BOOST_LOG_TRIVIAL(info) << "File not found.\n";
    lock_mutex.unlock();
    return NO_SUCH_FILE_ERROR;
  }
  BOOST_LOG_TRIVIAL(info) << "Missing READ_LOCK\n";
  lock_mutex.unlock();
  return NO_LOCK_ERROR;
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

        BOOST_LOG_TRIVIAL(info)<<"fstat() returned:\n";
        BOOST_LOG_TRIVIAL(info) <<"Mode:\t\t\t"<<buff->st_mode <<endl;
        BOOST_LOG_TRIVIAL(info) <<"Size:\t\t\t"<<buff->st_size <<endl;
        BOOST_LOG_TRIVIAL(info) <<"Block size:\t\t"<<buff->st_blksize <<endl;
        BOOST_LOG_TRIVIAL(info) <<"Allocated blocks:\t"<<(int) buff->st_blocks <<endl;
        BOOST_LOG_TRIVIAL(info) <<"Last access:\t\t"<<ctime(&buff->st_atime);
        BOOST_LOG_TRIVIAL(info) <<"Last modification:\t"<<ctime(&buff->st_mtime);
        BOOST_LOG_TRIVIAL(info) <<"Last status change:\t"<<ctime(&buff->st_ctime);
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
                BOOST_LOG_TRIVIAL(info) <<"Wrong offset value";
                return -1;
              }
              iterStream->stream.seekg(offset,ios_base::beg);
              BOOST_LOG_TRIVIAL(info) << "Cursor set on pozistion: " <<iterStream->stream.tellg()    <<endl;

            }
            else if(whence == SEEK_CURRENT)
            {
              iterStream->stream.seekg(offset,ios_base::cur);
              BOOST_LOG_TRIVIAL(info) << "Cursor set on pozistion: " <<iterStream->stream.tellg() <<endl;
            }
            else if(whence == SEEK_END_FILE)
            {
              if (offset > 0)
              {
                BOOST_LOG_TRIVIAL(info) <<"Wrong offset value";
                return -1;
              }
              iterStream->stream.seekg(offset,ios_base::end);
              BOOST_LOG_TRIVIAL(info) << "Cursor set on pozistion: " <<iterStream->stream.tellg() <<endl;
            }
          }
          else
          {
            if(whence == SEEK_BEGIN)
            {
              if (offset < 0)
              {
                BOOST_LOG_TRIVIAL(info) <<"Wrong offset value";
                return -1;
              }
              iterStream->stream.seekp(offset,ios_base::beg);
              BOOST_LOG_TRIVIAL(info) << "Cursor set on pozistion: " <<iterStream->stream.tellp() <<endl;
            }
            else if(whence == SEEK_CURRENT)
            {
              iterStream->stream.seekp(offset,ios_base::cur);
              BOOST_LOG_TRIVIAL(info) << "Cursor set on pozistion: " <<iterStream->stream.tellp() <<endl;
            }
            else if(whence == SEEK_END_FILE)
            {
              if (offset > 0)
              {
                BOOST_LOG_TRIVIAL(info) <<"Wrong offset value";
                return -1;
              }
              iterStream->stream.seekp(offset,ios_base::end);
              BOOST_LOG_TRIVIAL(info) << "Cursor set on pozistion: " <<iterStream->stream.tellp() <<endl;
            }
          }
          break;
        }
      }
      return 0;
    }
  }
  return NO_SUCH_FILE_ERROR;
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
            BOOST_LOG_TRIVIAL(info) << "File unlocked for PID:" <<pid <<endl;
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
            BOOST_LOG_TRIVIAL(info) << "READ_LOCK error caused by:WRITE LOCK ENABLE\n";
            lock_mutex.unlock();
            return LOCKING_NOT_POSSIBLE_ERROR;
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
          BOOST_LOG_TRIVIAL(info) << "READ_LOCK enable for PID: "<<pid<<endl;
          lock_mutex.unlock();
          return 0;
        }
        else
        {
          Lock lock;
          lock.lockMode = READ_LOCK;
          lock.lockPID = pid;
          iter->listLock.push_back(lock);
          BOOST_LOG_TRIVIAL(info) << "READ_LOCK enable for PID: "<<pid<<endl;
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
          BOOST_LOG_TRIVIAL(info) << "WRITE_LOCK error caused by: another lock ENABLE\n";
          lock_mutex.unlock();
          return LOCKING_NOT_POSSIBLE_ERROR;
        }
        else
        {
          Lock lock;
          lock.lockMode = WRITE_LOCK;
          lock.lockPID = pid;
          iter->listLock.push_back(lock);
          BOOST_LOG_TRIVIAL(info) << "WRITE_LOCK enable for PID: "<<pid<<endl;
          lock_mutex.unlock();
          return 0;
        }
      }
    }
  }

  lock_mutex.unlock();
  return -200999;
}
