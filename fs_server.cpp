#include <iostream>
#include "fs_server_web.h"
#include "fs_server_file.h"
#include "fs_server.h"

using namespace std;


int main(int argc, char *argv[]){
  cout << "Server is running...." << endl;
  if (argc < 2) {
    cout << "Error: no port provided\n";
    exit(-1);
  }
  start(argv);
  cout << endl;
  return 0;
}
