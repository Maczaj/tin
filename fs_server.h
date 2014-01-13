#ifndef FS_SERVER
#define FS_SERVER 1

#include <sys/stat.h>
// #include <string>
#include <string.h>

#ifndef BOOST_ARCHIVE_TEXT_IARCHIVE_HPP
#include <boost/archive/text_oarchive.hpp>
#endif
// #include <boost/archive/text_iarchive.hpp>

//==================== REQUESTS ===========================//
enum FS_cmdT {
  CLOSE_SRV_REQ = 1,
  FILE_OPEN_REQ,
  FILE_CLOSE_REQ,
  FILE_READ_REQ,
  FILE_WRITE_REQ,
  FILE_STAT_REQ,
  FILE_LOCK_REQ,
  FILE_LSEEK_REQ
};

typedef struct FS_c_close_srvrT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
  }
  FS_cmdT command;
} FS_c_close_srvrT;


typedef struct FS_c_open_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & name;
    ar & flags;
  }
  FS_cmdT command;
  std::string name;
  int flags;
} FS_c_open_fileT;


typedef struct FS_c_close_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & fd;
  }
  FS_cmdT command;
  int fd;
} FS_c_close_fileT;


typedef struct FS_c_stat_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & fd;
  }
  FS_cmdT command;
  int fd;
} FS_c_stat_fileT;


typedef struct FS_c_write_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int version) const
  {
    ar & command;
    ar & fd;
    ar & len;
    for (int index = 0; index < len; ++index)
        ar & ((unsigned char *)data)[index];
  }
  template<class Archive>
  void load(Archive & ar, const unsigned int version)
  {
      ar & command;
      ar & fd;
      ar & len;
      data = new char[len];
      for (int index = 0; index < len; ++index)
          ar & ((unsigned char *)data)[index];
      ((unsigned char *)data)[len] = 0;
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER()
  FS_cmdT command;
  int fd;
  size_t len; //wielkość danych do zapisania
  void * data; //dane do zapisania w pliku
} FS_c_write_fileT;


typedef struct FS_c_read_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & fd;
    ar & len;
  }
  FS_cmdT command;
  int fd;
  size_t len;
} FS_c_read_fileT;


typedef struct FS_c_lseek_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & fd;
    ar & offset;
    ar & whence;
  }
  FS_cmdT command;
  int fd;
  long offset;
  int whence;
} FS_c_lseek_fileT;


typedef struct FS_c_lock_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & fd;
    ar & lock_type;
  }
  FS_cmdT command;
  int fd;
  int lock_type; //alias dla mode
} FS_c_lock_fileT;

//===================== RESPONSES ============================//
typedef int FS_statT;

enum FS_resT {
  FILE_OPEN_RES = 2,
  FILE_CLOSE_RES,
  FILE_READ_RES,
  FILE_WRITE_RES,
  FILE_STAT_RES,
  FILE_LOCK_RES,
  FILE_LSEEK_RES
};

typedef struct FS_s_open_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & fd;
  }
  FS_resT command; //na jaką komende jest to odpowiedź
  int fd; //deskryptor pliku lub wartość ujemna w przypadku niepowodzenia
} FS_s_open_fileT;


typedef struct FS_s_close_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & status;
  }
  FS_resT command; //na jaką komende jest to odpowiedź
  FS_statT status;
} FS_s_close_fileT;


typedef struct FS_s_stat_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & status;
  }
  FS_resT command;
  FS_statT status;
  struct stat buf; //bufor na dane o pliku
} FS_s_stat_fileT;


typedef struct FS_s_write_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & status;
    ar & written_len;
  }
  FS_resT command;
  FS_statT status;
  size_t written_len; //ilość faktycznie zapisanych danych
} FS_s_write_fileT;


typedef struct FS_s_read_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & status;
    ar & read_len;
    ar & data;
  }
  FS_resT command;
  FS_statT status;
  size_t read_len; //ilość faktycznie odczytanych danych
  void * data; //bufor na dane do odczytu
} FS_s_read_fileT;


typedef struct FS_s_lseek_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & status;
  }
  FS_resT command;
  FS_statT status;
} FS_s_lseek_fileT;


typedef struct FS_s_lock_fileT {
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & command;
    ar & status;
  }
  FS_resT command;
  FS_statT status;
} FS_s_lock_fileT;

#endif // FS_SERVER_H
