/*********************************
 * HTTPRequest - Class representing an HTTP request message. May be used both
 * to parse an existing HTTP request into a comprehensible object, and to
 * construct new requests from scratch and print them out to a text string.
 * Makes no attempt to handle the body of the request -- only the request line
 * and the headers will be captured.
 *
 * If all you want to do is download a file, call createGetRequest() with
 * the path of the file that you want to download, and then call setHost()
 * on the returned object with the hostname of the server from which you'll
 * be downloading.  You should then be able to Print() the request out to a
 * character buffer to get something that the server will accept.
 *
 * Also see the HTTPMessage class for methods that can be used to query and
 * set the request's headers.
 *********************************/

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include "HTTPMessage.h"
#include "TCPSocket.h"
#include <string>


class HTTPRequest : public HTTPMessage {
 public:
  /*********************************
   * Name:    HTTPRequest
   * Purpose: constructor, constructs a new HTTPRequest. Note that 
   *          nothing is done to check the validity of the arguments 
   *          -- make sure you trust your input.
   * Receive: method - The action being requested (e.g. GET, POST, etc).
   *          path - The URL of the resource to which the request 
   *                applies. In most cases, this will typically 
   *                just be the path of the resource on the server
   *                (e.g. /somedir/something.txt).
   *          version - The HTTP version of the client making the 
   *                    request. Default is HTTP 1.1 (which ought to 
   *                    be what you support).
   * Return:  None
   *********************************/
  HTTPRequest(const std::string& method = "",
              const std::string& path = "",
              const std::string& version = "HTTP/1.1");

  /*********************************
   * Name:    ~HTTPRequest
   * Purpose: Destructor of HTTPRequest class objects
   * Receive: None
   * Return:  None
   *********************************/
  virtual ~HTTPRequest();

  /*********************************
   * Name:    parse
   * Purpose: constructs an HTTPRequest object corresponding to the 
   *          actual request text in the given buffer. Use this if 
   *          you've received a request and want to know what it's 
   *          asking.
   * Receive: data - The text buffer in which the request is stored.
   *          length - The length of the request data, in bytes.
   * Return:  An HTTPRequest parsed from the request text/data. If 
   *          parsing fails, a NULL pointer will be returned instead.
   *********************************/
  static HTTPRequest* parse(const char* data, unsigned length);

  /*********************************
   * Name:    parse
   * Purpose: constructs an HTTPRequest object corresponding to the 
   *          actual request string. Use this if you've received a 
   *          request and want to know what it's asking.
   * Receive: requestString - The string in which the request is stored.
   * Return:  An HTTPRequest parsed from the request string. If 
   *          parsing fails, a NULL pointer will be returned instead.
   *********************************/
  static HTTPRequest* parse(const std::string& requestString);

  /*********************************
   * Name:    createGetRequest
   * Purpose: Constructs a new HTTP GET request, with a header or 
   *          two set to make it more likely that the server will 
   *          return an easy-to-handle result.
   * Receive: path - The URL of the resource to get.
   *          version - The HTTP version to associate with the request.
   * Return:  A new HTTPRequest object for the GET request.
   *********************************/
  static HTTPRequest* createGetRequest(const std::string& path ="",
      const std::string& version = "HTTP/1.1");

  /*********************************
   * Name:    send
   * Purpose: Send this request to the socket sock
   * Receive: The TCPSocket we want to send to
   * Return:  None
   *********************************/
  void send(TCPSocket& socket);

  /*********************************
   * Name:    receive
   * Purpose: Receive data from the socket sock and create an 
   *          HTTPRequest object by parsing that piece of data.
   * Receive: socket - The TCPSocket we want to receive from
   * Return:  receive a piece of data from the socket, until a line 
   *          with only CLRF is found, which means it is the end of 
   *          the header. Create an HTTPRequest object from that 
   *          header.
   *********************************/
  static HTTPRequest* receive(TCPSocket& socket);

  /*********************************
   * Name:    getMethod
   * Purpose: Looks up the method of the request (e.g. GET, PUT, DELETE).
   * Receive: None
   * Return:  The request method.
   *********************************/
  const std::string& getMethod() const {
    return method;
  }

  /*********************************
   * Name:    getPath
   * Purpose: Looks up the path targeted by the request (e.g. /stuff.txt).
   * Receive: None
   * Return:  The request's path.
   *********************************/
  const std::string& getPath() const {
    return path;
  }

  /*********************************
   * Name:    getUrl
   * Purpose: Looks up the URL targeted by the request
   * Receive: None
   * Return:  The request's URL
   *********************************/
  const std::string getUrl() const;

  /*********************************
   * Name:    getVersion
   * Purpose: Looks up the HTTP version of the requesting client (e.g.
   *          HTTP/1.1).
   * Receive: None
   * Return:  The request's HTTP version.
   *********************************/
  const std::string& getVersion() const {
    return version;
  }

  /*********************************
   * Name:    getHost
   * Purpose: Looks up the host for which the request is intended, 
   *          from the request's Host header.
   * Receive: outHost - Will be set to the request's target host. If the host
   *          has not yet been entered, it will be set to an empty string.
   * Return:  None
   *********************************/
  void getHost(std::string& outHost) const;

  /*********************************
   * Name:    print
   * Purpose: Prints the request object to a text string, suitable 
   *          for transmission to an HTTP server. Includes the 
   *          terminating blank line and all request headers.
   * Receive: outputString - Will be set to the request text.
   * Return:  None
   *********************************/
  void print(std::string& outputString) const;

  /*********************************
   * Name:    print
   * Purpose: Prints the request object to a char array, suitable 
   *          for transmission to an HTTP server.  Includes the 
   *          terminating blank line and all request headers.
   * Receive: outputBuffer - The text buffer into which the request 
   *                         should be printed. Will be null-terminated.
   *          bufferLength - The number of characters available for 
   *                         writing in the buffer. Printing stops 
   *                         after this many characters have been
   *                         written.
   * Return:  None
   *********************************/
  void print(char* outputBuffer, unsigned bufferLength) const;

  /*********************************
   * Name:    setMethod
   * Purpose: Sets the method of the HTTP request (e.g. GET, PUT, DELETE).
   * Receive: method - The method to set for the request.
   * Return:  None
   *********************************/
  void setMethod(const std::string& method) {
    this->method = method;
  }

  /*********************************
   * Name:    setPath
   * Purpose: Sets the path that the request should target (e.g. /stuff.txt).
   * Receive: path - The path to set for the request.
   * Return:  None
   *********************************/
  void setPath(const std::string& path) {
    this->path = path;
  }

  /*********************************
   * Name:    setVersion
   * Purpose: Sets the HTTP version supported by the request's client.
   * Receive: version - The HTTP version the request should indicate.
   * Return:  None
   *********************************/
  void setVersion(const std::string& version) {
    this->version = version;
  }

  /*********************************
   * Name:    setHost
   * Purpose: Sets the host for which the request is intended, into 
   *          the request's Host header.
   * Receive: host - The host to set for the request.
   * Return:  None
   *********************************/
  void setHost(const std::string& host) {
    setHeaderField("Host", host);
  }

 private:
  std::string method;
  std::string path;
  std::string version;
};

#endif  // _HTTP_REQUEST_H_
