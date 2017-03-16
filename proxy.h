#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "URL.h"
#include "TCPSocket.h"
#include "ProxyWorker.h"
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string>

int port = 80;
hostent *server = NULL;

void helpMessage(char *argv[]) {
  std::cout << "Usage " << argv[0] << " [options]" << std::endl;
  std::cout << "The following option is available:" << std::endl;
  std::cout << "  -h Display help message" << std::endl;
}

/*********************************
 * Name:    parseAgrv
 * Purpose: Parse the parameters
 * Recieve: argv and argc
 * Return:  none
 *********************************/
void parseArgs(int argc, char *argv[]) {
  char *endptr;  // for strtol

  for (int i = 1; i < argc; i++) {
    if ((!strncmp(argv[i], "-h", 2)) ||
       (!strncmp(argv[i], "-H", 2))) {
      helpMessage(argv);
      exit(1);
    } else {
      std::cerr << "Invalid parameter:" << argv[i] << std::endl; 
      helpMessage(argv);
      exit(1);
    }
  }
}
