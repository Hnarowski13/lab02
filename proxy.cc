#include "proxy.h"

/*********************************
 * Name:  ClientHandler
 * Purpose: Handles a single web request
 * Receive: The data socket pointer
 * Return:  None
 *********************************/
void* connectionHandler(void *arg) {
  TCPSocket *clientSock = (TCPSocket *) arg;

    ProxyWorker *worker = new ProxyWorker(clientSock);
    worker->handleRequest();
   // worker.handleRequest();
    std::cout << "got to connection handler " << std::endl;
    
  /********TO BE IMPLEMENTED********/
  // Create a ProxyWorker to handle this connection.
  // When done handling the connection, remember to close and delete
  // the socket, and delete the ProxyWorker
    clientSock->Close();
    delete clientSock;
    delete worker;
    
  pthread_exit(0);
}

/*********************************
 * Name:    main
 * Purpose: Contains the main server loop that handles requests by 
 *          spawing threads
 * Receive: argc is the number of command line params, argv are the 
 *          parameters
 * Return:  0 on clean exit, and 1 on error
 *********************************/
int main(int argc, char *argv[]) {
  signal(SIGPIPE, SIG_IGN);  // To ignore SIGPIPE
  TCPSocket* clientSock;  // for accepting connections
    TCPSocket serverSock;
    
    unsigned short int tcpServerPort = 0;  // let the server decide
    
  int rc;  // return code for pthread
    
  parseArgs(argc, argv);
    

    
    /********TO BE IMPLEMENTED********/
    // Creata a socket, bind it and listen for incoming connection.
    
    unsigned short somePort = 0;  // let the OS decide, like proj1.
    try {
        serverSock.Bind(somePort);
    } catch (std::string msg)
    {
        std::cerr << msg << std::endl;
        exit(1);
    }
    try {
        serverSock.Listen();
    }
    catch (std::string msg)
    {
        std::cerr << msg << std::endl;
        exit(1);
    }
    serverSock.getPort(tcpServerPort);
    // print the port name
    std::cout << "[TCP] socket has port: " << tcpServerPort << std::endl;
    
  // start the infinite server loop
  while (true) {
    /********TO BE IMPLEMENTED********/
      //cout << "got here?" << endl;
      // accept the incoming connection
      try {
          clientSock = new TCPSocket();
          if (!serverSock.Accept(*clientSock)) {
              std::cout << "Cannot accept ..." << std::endl;
              continue;
          }
       //   cout << "got here?" << endl;
          
      } catch (std::string msg) {
          std::cout << msg << std::endl;
          continue;
      }
            

    //break; // remove this break when you have TCPSocket::Accept. This break
           // is to stop the infinite loop from creating too many thread and
           // crashs the program
    // accept incoming connections
      // create new thread
      pthread_t thread;
      rc = pthread_create(&thread, NULL, connectionHandler, clientSock);
      
      // if rc != 0, the creation of threadis failed.
      if (rc) {
          std::cerr << "Thread create error! Error code: " << rc << std::endl;
          exit(1);
      }

  }

  /********TO BE IMPLEMENTED********/
  // close the listening sock

  std::cout << "Parent process terminated." << std::endl;

  return 0;
}
