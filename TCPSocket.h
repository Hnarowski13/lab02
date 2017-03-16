/*********************************
 * TCPSocket - Class wrapping the TCP operations in C++ style class. Errors are 
 * returned by throwing exceptions.
 *********************************/
#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "URL.h"
#include <string>

class TCPSocket {
 private:
  int sock;
  struct sockaddr_in serverAddr;

  /*********************************
   * Name:    readNBytes
   * Purpose: Reads n bytes from the TCPSocket
   * Receive: vptr - the pointer to the buffer that will be used to hold the 
   *                 data
   *          n - the number of bytes to be read
   * Return:  The number of bytes read
   *********************************/
  int readNBytes(void* vptr, unsigned int n);

  /*********************************
   * Name:    readLine
   * Purpose: Reads a line from the TCPSocket
   * Receive: vptr - the pointer to the buffer that will be used to hold the 
   *                 data
   *          maxLen - the maximum size of the line
   * Return:  The number of bytes read
   *********************************/
  int readLine(void* vptr, unsigned int maxLen);

  /*********************************
   * Name:    readHeader
   * Purpose: Reads from a TCPSocket until \r\n\r\n is found, in order to
   *          receive a complete HTTP message header. The received data is
   *          then passed to be parsed by HTTPRequest or HTTPResponse
   * Receive: buffer - buffer to hold the received data
   *          bufferLen - the maximum length of the bufer
   *          totalReceivedLen - the total number of bytes received.
   * Return:  The end position of the HTTP message header
   *********************************/
  int receiveHeaders(char* buffer, unsigned int bufferLen,
      unsigned int& totalReceivedLen);

  /*********************************
   * Name:    createSocket
   * Purpose: Private function that handles socket creation, despite what 
   *          connect function is used.
   * Receive: None
   * Return:  None
   *********************************/
  void createSocket();

 public:
  /*********************************
   * Name:    TCPSocket
   * Purpose: Default constructor sets the socket to -1.
   * Receive: None
   * Return:  None
   *********************************/
  TCPSocket() {
    sock = -1;
  }

  /*********************************
   * Name:    ~TCPSocket
   * Purpose: Destructor, closes the socket by invoking close()
   * Receive: None
   * Return:  None
   *********************************/
  ~TCPSocket() {
    Close();
    sock = -1;
  }

  /*********************************
   * Name:    Connect, capitalized to avoid confusion with the connect in 
   *          socket.h
   * Purpose: Initiate a connection to a server with serverName and port number
   * Receive: serverName - the hostname to connect to
   *          serverPort - the port number to connect to
   * Return:  None
   *********************************/
  void Connect(const std::string& serverName, unsigned short serverPort);

  /*********************************
   * Name:    Connect
   * Purpose: Initiate a connection to a server with hostEnt and port number
   * Receive: hostEnt - the hostEnt structure
   *          serverPort - the port number to connect to
   * Return:  None
   *********************************/
  void Connect(hostent* host, unsigned short serverPort);

  /*********************************
   * Name:    Connect
   * Purpose: Initiate a connection to a URL
   * Receive: url - is the URL object holding server name, port number and 
   *                resource name
   * Return:  None
   *********************************/
  void Connect(const URL& url);

  /*********************************
   * Name:    Close
   * Purpose: Closes an open socket
   * Receive: None
   * Return:  None
   *********************************/
  int Close();

  /*********************************
   * Name:    Bind
   * Purpose: Creates and binds to a socket in a server process
   * Receive: serverPort - the port number for the service
   * Return:  None
   *********************************/
  void Bind(unsigned short serverPort);

  /*********************************
   * Name:    Listen
   * Purpose: Start to listen to a bound socket
   * Receive: None
   * Return:  None
   *********************************/
  void Listen();

  /*********************************
   * Name:    Accept
   * Purpose: Accept a connection waiting on a bound port
   * Receive: dataSock - is the TCPSocket object that holds the new connection
   *                     from/to the client
   * Return:  true if the connection is accepted, false otherwise.
   *********************************/
  bool Accept(TCPSocket& dataSock);

  /*********************************
   * Name:    Accept
   * Purpose: Alternative form of accept that creates a new TCPSocket object
   * Receive: None
   * Return:  the pointer to the new TCPSocket object
   *********************************/
  TCPSocket *Accept();

  /*********************************
   * Name:    writeString
   * Purpose: Writes a string on this TCPSocket
   * Receive: data - the string to be written to the TCPSocket
   * Return:  The number of bytes written, should always equal to data.size()
   *********************************/
  int writeString(const std::string& data);

  /*********************************
   * Name:    readString
   * Purpose: Reads a string from this TCPSocket
   * Receive: data - the string to hold the received bytes
   * Return:  The number of bytes read from the TCPSocket
   *********************************/
  int readString(std::string& data);

  /*********************************
   * Name:    readHeader
   * Purpose: Reads from a TCPSocket until \r\n\r\n is found, in order to
   *          receive a complete HTTP message header.
   * Receive: header - the variable to hold the header
   *          body - the variable to hold the (possibly partial) body
   * Return:  None
   *********************************/
  void readHeader(std::string& header, std::string& body);

  /*********************************
   * Name:    readData
   * Purpose: Read bytesLeft bytes from the TCPSocket
   * Receive: data - the string that will be used to hold the data
   *          bytesLeft-  the number of bytes to be read
   * Return:  the number of bytes read
   *********************************/
  int readData(std::string& data, unsigned int bytesLeft);

  /*********************************
   * Name:    readLine
   * Purpose: Reads a line from the TCPSocket, terminated by a CRLF (\r\n)
   * Receive: data - holds the data read from the TCPSocket
   * Return:  The number of bytes read
   *********************************/
  int readLine(std::string& data);

  /*********************************
   * Name:    getPort
   * Purpose: Get the port number of the TCPSocket
   * Receive: gettingPort - holds the port number
   * Return:  None
   *********************************/
  void getPort(unsigned short& gettingPort);
};

#endif  // _TCPSOCKET_H_
