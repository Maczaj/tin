#include <sys/stat.h>

//==================== REQUESTS ===========================//
enum FS_cmdT { close_srv_req , file_open_req, file_close_req , file_read_req, file_write_req, file_stat_req, file_lock_req, file_lseek_req};

struct FS_c_close_srvrT {
FS_cmdT command;
};

struct FS_c_open_fileT {
FS_cmdT command;
char* name;
int flags;
};


struct FS_c_close_fileT {
FS_cmdT command;
int fd;
};


struct FS_c_stat_fileT {
FS_cmdT command;
int fd;
};


struct FS_c_write_fileT
{
FS_cmdT command;
int fd;
size_t len; //wielkość danych do zapisania
void* data; //dane do zapisania w pliku
};


struct FS_c_read_fileT {
FS_cmdT command;
int fd;
size_t len;
};


struct FS_c_lseek_fileT {
FS_cmdT command;
int fd;
long offset;
int whence; 
};


struct FS_c_lock_fileT {
FS_cmdT command;
int fd;
int lock_type; //alias dla mode
};

//===================== RESPONSES ============================//
typedef int FS_statT;

enum FS_resT { close_srv_res , file_open_res, file_close_res , file_read_res, file_write_res, file_stat_res, file_lock_res, file_lseek_res};

struct FS_s_open_fileT {
FS_resT command; //na jaką komende jest to odpowiedź
int fd; //deskryptor pliku lub wartość ujemna w przypadku niepowodzenia
};


struct FS_s_close_fileT {
FS_resT command; //na jaką komende jest to odpowiedź
FS_statT status; 
};


struct FS_s_stat_fileT {
FS_resT command;
FS_statT status;
struct stat buf; //bufor na dane o pliku
};


struct FS_s_write_fileT
{
FS_resT command;
FS_statT status;
size_t written_len; //ilość faktycznie zapisanych danych
};


struct FS_s_read_fileT {
FS_resT command;
FS_statT status;
size_t read_len; //ilość faktycznie odczytanych danych
void * data; //bufor na dane do odczytu
};


struct FS_s_lseek_fileT {
FS_resT command;
FS_statT status;
};


struct FS_s_lock_fileT {
FS_resT command;
FS_statT status;
};


