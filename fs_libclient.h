/***


TODO: awesome comment



****/
#include <fstream>
//============= STALE ===============================//

// #define CREATE fstream::in | fstream::out | fstream::app
// #define READ fstream::in | fstream::app
// #define WRITE fstream::out | fstream::app

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



//===============================================//

/**
tworzy połączenie z serwerem, zwraca uchwyt serwera, który jest jednocześnie desktryptorem gniazda utworzonego do komunikacji, w przypadku niepowodzenia zwróci pewną wartość ujemną, zależną od typu zastniałego błędu
**/
int fs_open_server(char *adres_serwera);


/**
zamyka połączenie z serwerem, w wypadku sukcesu zwraca 0, w przypadku błędu jak wyżej.
**/
int fs_close_server(int srvhndl);

/**
otwarcie konkretnego pliku - ścieżka do pliku podawana przez klienta będzie podlegać translacji na serwerze, które będzie polegało na przekształceniu na ścieżkę bezwględną. Z uwagi na brak uwierzytelniania podłączanych klientów zakłada się brak kontroli praw dostępu, zatem każdy klient będzie w stanie otworzyć każdy plik z dowolną flagą. W odniesieniu do sposobu przekazywania deskryptorów plików, będą przekazywane unikalne wartości nadawne przez aplikacje serwera i będą podlegać mapowaniu na lokalną wartość wykorzystywaną przez danego klienta, który będzie osobnym procesem
TODO: wpisac tutaj jawnie jakie moga byc flagi!!!
**/
int fs_open(int srvhndl, char *name, int flags);

/**
zapisanie do pliku o deskrytporze fd danych z bufora. Aby funkcja zadziałała poprawnie, na pliku musi być założona dokładnie jedna blokada, i to do zapisu, należąca do klienta wykonującego operację
**/
int fs_write(int srvhndl , int fd , void * buf , size_t len);

/**
wczytanie z pliku o deskryptorze fd danych do bufora. Aby funkcja zadziałała poprawnie, na pliku musi występować blokada do czytania, należąca do klienta wykonującego operację
**/
int fs_read(int srvhndl , int fd , void * buf , size_t len);

/**
przesuwa wskaźnik pliku o zadaną wartość w określonym kierunku

TODO: dac tutaj opis czym jest offset i whence!!!
**/
int fs_lseek(int srvhndl, int fd , long offset , int whence);

/**
zamknięcie danego pliku. Zamknięcie pliku oznacza również zdjęcie wszystkich blokad założonych przez danego klienta
**/
int fs_close(int srvhndl, int fd);

/**
pobranie informacji o pliku. Nie wszystkie pola struktury mają sens, zatem poniżej są wymienione tylko te używane przez nas:
   mode_t    st_mode;    // protection
    off_t     st_size;    //total size, in bytes
    blksize_t st_blksize; // blocksize for file system I/O
    blkcnt_t  st_blocks;  // number of 512B blocks allocated
    time_t    st_atime;   // time of last access
    time_t    st_mtime;   // time of last modification
    time_t    st_ctime;   // time of last status change
**/
int fs_stat(int srvhndl , int fd, struct stat* buff);

/**
założenie blokady na pliku. W przypadku powodzenia zwróci 0. Niepowodzenie wystąpi w następujących sytuacjach:
-próba założenia blokady READ jeśli jest założona blokada WRITE,
-próba założenia blokady WRITE jeśli jest już założona inna blokada;
i będzie sygnalizowane poprzez zwrócenie ujemnej wartości. Informacja o założonych blokadach będzie przechowywana w strukturze opisującej dany plik

TODO: okreslic mozliwe wartosci mode
**/
int fs_lock(int srvhndl , int fd , int mode);
