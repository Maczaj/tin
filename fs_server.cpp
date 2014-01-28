#include <iostream>
#include "fs_server_web.h"
#include "fs_server_file.h"
#include "fs_server.h"

using namespace std;

int main(int argc, char *argv[]){

  if (argc < 2) {
     BOOST_LOG_TRIVIAL(fatal) << "Error: no port provided\n";
    exit(-1);
  }
  cout << "Server is running...." << endl;
  start(argv);
  cout << endl;
  return 0;
}
