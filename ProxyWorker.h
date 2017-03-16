/*********************************
 * ProxyWorker - Class for handling HTTP connections. Each instance acts as
 * a single worker to handle a request for a proxy. It passes the request to
 * the server and return the response to the client.
 *********************************/

#ifndef _PROXYWORKER_H_
#define _PROXYWORKER_H_

#include <iostream>
#include <string>

#include "HTTPResponse.h"
#include "HTTPRequest.h"
#include "TCPSocket.h"
#include "URL.h"

class ProxyWorker {
 private:
  URL *serverUrl;  // Server's URL, obtained from each request.
  unsigned short int port;  // For a full blown proxy

  TCPSocket *clientSock;   // The socket for client
  TCPSocket serverSock;
  HTTPRequest *clientRequest;  // Obj to handle client request
  HTTPResponse*serverResponse;  // Obj to handle server response

  static const std::string subliminalTag;

  /*********************************
   * Name:    getRequest
   * Purpose: Receives a request from a client and parse it
   * Receive: None
   * Return:  A boolean value indicating if getting the request is
   *          successful or not
   *********************************/
  bool getRequest();

  /*********************************
   * Name:    checkRequest
   * Purpose: Check if the request just gotten is valid
   * Receive: None
   * Return:  the request is valid or not 
   *********************************/
  bool checkRequest();

  /*********************************
   * Name:    forwardRequest
   * Purpose: Forwards a client request to the server and get the response
   *          1. Forward the request to the server
   *          2. Receive the response header and modify the server field
   *          3. Receive the response body. Handle both chunk/default encoding.
   * Receive: None
   * Return:  A boolean value indicating if forwarding the request is 
   *          successful or not
   *********************************/
  bool forwardRequest();

  /*********************************
   * Name:    getResponse
   * Purpose: Get a response from the server
   * Receive: None
   * Return:  A boolean value indicating if the request is received
   *********************************/
  bool getResponse();

  /*********************************
   * Name:    returnResponse
   * Purpose: Return the response from the server to the client. Also, modify
   *          the server field
   * Receive: None
   * Return:  A boolean indicating if returning the request was successful or
   *          not
   *********************************/
  bool returnResponse();

  /*********************************
   * Name:    proxyResponse
   * Purpose: Creates a response "locally" and return it to a client
   *          For error status like 403, 404, ... and 500.
   * Receive: statusCode - the status code
   * Return:  A boolean indicating if the creation is succssful
   *********************************/
  bool proxyResponse(const int statusCode);

  /*********************************
   * Name:    hasSubliminalTag
   * Purpose: Check if the url has a subliminal message tag in the path
   * Receive: url - the url as a string
   * Return:  true if the url has the subliminal tag, false otherwise
   *********************************/
  static bool hasSubliminalTag(const std::string& url);

  /*********************************
   * Name:    removeSubliminalTag
   * Purpose: Remove the subliminal message tag from the path
   * Receive: url - the url as a string
   * Return:  None
   *********************************/
  static void removeSubliminalTag(std::string& url);

  static const std::string& getSubliminalTag() {
    return subliminalTag;
  }

  /*********************************
   * Name:    subliminalResponse
   * Purpose: Create a "subliminal message" response "locally" and return
   *          it to the client
   * Receive: url - the original url
   *          duration - the duration of the subliminal message
   * Return:  a boolean indicating if the creation is succssful
   *********************************/
  bool subliminalResponse(const std::string& url, int duration = 0);

  /*********************************
    Name:    getChunkSize
   * Purpose: Extract the chunk size from a std::string
   * Receive: data - the chunk size in hex string
   * Return:  the chunk size in int
   *********************************/
  int getChunkSize(std::string& data);

 public:
  /*********************************
   * Name:    ProxyWorker
   * Purpose: Constructor of ProxyWorker class objects
   * Receive: cs - the TCPSocket that is connected to the requesting client
   * Return:  None
   *********************************/
  explicit ProxyWorker(TCPSocket *cs);

  /*********************************
   * Name:    ~ProxyWorker
   * Purpose: Destructor of ProxyWorker class objects
   * Receive: None
   * Return:  None
   *********************************/
  ~ProxyWorker();

  /*********************************
   * Name:    handleRequest
   * Purpose: handles a request by sending it to the serverUrl
   * Receive: None
   * Return:  None
   *********************************/
  void handleRequest();
};

#endif  // _PROXYWORKER_H_
