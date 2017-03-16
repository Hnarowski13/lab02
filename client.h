#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "URL.h"
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>

#include <climits>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>

/*********************************
 * Name:    helpMessage
 * Purpose: prints a brief usage std::string describing how to use the application,
 *          in case the user passes in something that just doesn't work.
 * Receive: exeName - the name of the executable
 *          out - the ostream
 * Return:  None
 *********************************/
void helpMessage(const char* exeName, std::ostream& out) {
  out << "Usage: " << exeName << " [options]" << std::endl;
  out << "The following options are available:" << std::endl;
  out << "    -s host URL" << std::endl;
  out << "    -p proxy URL" << std::endl;
  out << "    -h display help message" << std::endl;
  out << std::endl;
  out << "Example: " << exeName
      << " -s http://www.some_server.com/ -p 100.200.50.150:8080" << std::endl;
}

/*********************************
 * Name:    parseArgs 
 * Purpose: parse the parameters
 * Receive: argv and argc
 *          targetUrl - the target object we are going to download
 *          proxyAddr - the address of the proxy
 * Return:  None 
 *********************************/
void parseArgs(int argc, char *argv[],
              char **targetAddr, char **proxyAddr) {
  for (int i = 1; i < argc; i++) {
    if ((!strncmp(argv[i], "-s", 2)) ||
       (!strncmp(argv[i], "-S", 2))) {
      *targetAddr = argv[++i];
    } else if ((!strncmp(argv[i], "-p", 2)) ||
              (!strncmp(argv[i], "-P", 2))) {
      *proxyAddr = argv[++i];
    } else if ((!strncmp(argv[i], "-h", 2)) ||
              (!strncmp(argv[i], "-H", 2))) {
      helpMessage(argv[0], std::cout);
      exit(1);
    } else {
      std::cerr << "Invalid parameter: argv[i]" << std::endl;
      helpMessage(argv[0], std::cout);
      exit(1);
    }
  }
}

/*********************************
 * Name:    OpenLocalCopy
 * Purpose: Open a file pointer to store the data in ./Downloads
 * Receive: url - the url for the object
 * Return:  The file pointer
 *********************************/
// Opens a local copy of the file referenced by the given request URL, for
// writing. Ignores any directories in the URL path, instead opening the file
// in the ./Downloads. Makes up a filename if none is given.
//
// Returns a pointer to the open file, or a NULL pointer if the open fails.
FILE* OpenLocalCopy(const URL* url) {
  FILE* outfile = NULL;

  struct stat sb;  // For checking if ./Download exists

  if (stat("./Downloads", &sb) == -1) {  // if ./Downloads does not exist
     mkdir("./Downloads", 0700);   // create it
  }

  const std::string& fullPath = url->getPath();
  size_t filenamePos = fullPath.rfind('/');
  // find the last '/', the substring after it should be the filename

  if ((filenamePos != std::string::npos) &&  // if found a '/'
      ((filenamePos + 1) < fullPath.length())) {
      // or / is not the end of the std::string, there is a filename in the URL
    std::string fn =
        std::string("Downloads/") + fullPath.substr(filenamePos + 1);
    outfile = fopen(fn.c_str(), "wb");
  } else {  // there is no filename in the URL, name it index.html
    outfile = fopen("Downloads/index.html", "wb");
  }

  return outfile;
}
