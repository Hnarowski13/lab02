/*********************************
 * HTTPResponse - Class representing an HTTP response message.  May be used
 * both to parse existing HTTP response into a comprehensible object, and to
 * construct new responses from scratch and print them out to a text string.
 * Makes no attempt to handle the body of the response -- only the response code
 * and the headers will be captured.
 *
 * If you're planning on servicing GET and HEAD requests only, you can use
 * the createStandardResponse() method to have a lot of headers automatically
 * set up for you.  The HTTP specification mandates these headers, and some
 * clients may expect them.
 *
 * Also see the HTTPMessage class for methods that can be used to query and
 * set the response headers.
 *********************************/

#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include "HTTPMessage.h"
#include "TCPSocket.h"
#include <string>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <sstream>


class HTTPResponse : public HTTPMessage {
 public:
  /*********************************
   * Name:    HTTPResponse 
   * Purpose: Constructs a new HTTPResponse. Note that nothing is done 
   *          to check the validity of the arguments -- make sure you 
   *          trust your input and/or yourself.
   * Receive: statusCode - The code representing the response status (e.g. 
   *                       200, 403).
   *          statusDesc - A one-line textual description of the response.
   *          version - The HTTP version used to transmit the response.
   *          content - The text string to set as the response's description.
   * Return:  None
   *********************************/
  HTTPResponse(unsigned statusCode = 0, const std::string& statusDesc = "",
      const std::string& version = "HTTP/1.1",
      const std::string& content = "");

  /*********************************
   * Name:    ~HTTPRequest
   * Purpose: Destructor of HTTPResponse class objects
   * Receive: None
   * Return:  None
   *********************************/
  virtual ~HTTPResponse();

  /*********************************
   * Name:    parse
   * Purpose: Parse the response from server in the given buffer to construct
   *          an HTTPResponse object. Check if the response is formatted correctly.
   * Receive: data - the received data piece stored in a buffer
   *          length -  the length of the data, in bytes
   * Return:  a pointer to an HTTPResponse object, if this data is good.
   *          NULL otherwise
   *********************************/
  static HTTPResponse *parse(const char* data, unsigned length);

  /*********************************
   * Name:    createStandardResponse
   * Purpose: Constructs a new HTTPResponse with some mandatory header 
   *          fields convenienty set for you (unlike the constructor, 
   *          which sets no fields for you at all). Assumes that you 
   *          will be sending back some kind of message body, and that 
   *          the message body will be sent verbatim (i.e. not compressed).
   *          Also assumes that the connection will be closed immediately 
   *          (non-persistent connection) after the send ends.
   * Receive: contentLen - The length of the message body that will 
   *                       be sent following this response.
   *          statusCode - The code representing the response status (e.g. 500).
   *          statusDesc - A short description of the response code's meaning.
   *          version - The HTTP version used to transmit the response.
   * Return:  An HTTPResponse created for the given input, containing
   *          all of the mandatory headers.
   *********************************/
  static HTTPResponse* createStandardResponse(unsigned contentLen,
      unsigned statusCode = 0, const std::string& statusDesc = "",
      const std::string& version = "HTTP/1.1");

  /*********************************
   * Name:    getChunkSize
   * Purpose: for a given data, extract the chunk length
   * Receive: data - the data as std::string
   * Return:  the extracted chunk length
   *          note that the chunk length is removed from data std::string.
   *********************************/
  static int getChunkSize(std::string& data);

  /*********************************
   * Name:    receiveHeader
   * Purpose: receive a piece of data from the socket sock. Slice the
   *          received data into two parts, header and the body
   * Receive: sock - the TCPSocket to receive from
   *          header - the string to hold the incoming header string
   *          data - the string to hold the incoming body string,
   *                 probably just partial
   * Return:  None
   *********************************/
  void receiveHeader(TCPSocket& sock, std::string& header, std::string& data);

  /*********************************
   * Name:    receiveBody
   * Purpose: receive the desired number of bytes of data from the socket
   * Receive: sock - the TCPSocket to receive from
   *          data - the string to hold the incoming response body as string
   *          bytesLeft - the number of bytes to receive
   * Return:  the number of bytes received.
   *********************************/
  int receiveBody(TCPSocket& sock, std::string& body, int bytesLeft = BUFFER_SIZE);

  /*********************************
   * Name:    receiveLine 
   * Purpose: receive until a newline char is found
   * Receive: sock - the TCPSocket to receive from
   *          data - the string to hold incoming data
   * Return:  the number of bytes received
   *********************************/
  int receiveLine(TCPSocket& sock, std::string& data);

  /*********************************
   * Name:    getContentLen
   * Purpose: from the header, extract the "Content-Length"
   * Receive: None
   * Return:  the content length as int
   *********************************/
  const int getContentLen() const;

  /*********************************
   * Name:    getVersion 
   * Purpose: Looks up the version of the HTTP response (e.g. HTTP/1.1).
   * Receive: None
   * Returns: the response's HTTP version.
   *********************************/
  const std::string& getVersion() const {
    return version;
  }

  /*********************************
   * Name:     
   * Purpose: Looks up the status code of the HTTP response (e.g. 404, 
   *      500).
   * Receive: None
   * Return:  The response's status code.
   *********************************/
  unsigned getStatusCode() const {
    return statusCode;
  }

  /*********************************
   * Name:    getStatusDesc
   * Purpose: Looks up the description of the response (e.g. "OK").
   * Receive: None
   * Return:  The response's associated statusDesc string.
   *********************************/
  const std::string& getStatusDesc() const {
    return statusDesc;
  }

  /*********************************
   * Name:    getContent
   * Purpose: Looks up the content, the response body
   * Receive: None
   * Return:  the content as a string
   *********************************/
  const std::string& getContent() const {
    return content;
  }

  /*********************************
   * Name:    isChunked 
   * Purpose: Looks up if the response is chunked transfer encoding
   * Receive: None
   * Return:  true if this response is chunked, false otherwise
   *********************************/
  const bool isChunked() const {
    return chunked;
  }

  /*********************************
   * Name:    print
   * Purpose: prints the response object to a text string, suitable 
   *          for sending to an HTTP client.  Includes the terminating 
   *          blank line and all response headers.
   * Recieve: output_string - Will be set to the response text.
   * Return:  None
   *********************************/
  void print(std::string& output_string) const;

  /*********************************
   * Name:    print 
   * Purpose: prints the response object to a text string, suitable for 
   *          sending to an HTTP client. Includes the terminating blank 
   *          line and all response headers.
   * Receive: output_buffer - The text buffer into which the response 
   *                          should be printed. Will be null-terminated.
   *          bufferLen - The number of characters available for writing
   *                      in the buffer.  printing stops after this 
   *                      many characters have been written.
   * Return:  None
   *********************************/
  void print(char* output_buffer, unsigned bufferLen) const;

  /*********************************
   * Name:    setVersion 
   * Purpose: Sets the HTTP version of the response (e.g. HTTP/1.1).
   * Receive: version -  The version to set.
   * Return:  None
   *********************************/
  void setVersion(const std::string& version) {
    this->version = version;
  }

  /*********************************
   * Name:    setStatusCode
   * Purpose: Sets the status code to indicate in the response.
   * Receive: statusCode - The HTTP status code to set.
   * Return:  None
   *********************************/
  void setStatusCode(const unsigned statusCode) {
    this->statusCode = statusCode;
  }

  /*********************************
   * Name:    setStatusCode
   * Purpose: Sets the status code to indicate in the response.
   * Receive: statusCodeStr - The HTTP status code (in string) to set.
   * Return:  None
   *********************************/
  void setStatusCode(const std::string& statusCodeStr) {
    std::istringstream iss(statusCodeStr);
    iss >> this->statusCode;
  }

  /*********************************
   * Name:    setStatusDesc
   * Purpose: Sets the desc for the given status code being sent.
   * Receive: statusDesc - The text string to set as the response's description.
   * Return:  None
   *********************************/
  void setStatusDesc(const std::string& statusDesc) {
    this->statusDesc = statusDesc;
  }

  /*********************************
   * Name:    setContent
   * Purpose: Sets the content/response body for the HTTPResponse
   * Receive: content - The text string to set as the response's description.
   * Return:  None
   *********************************/
  void setContent(const std::string& content) {
    this->content = content;
    std::stringstream out;
    out << content.size();
    setHeaderField("Content-Length", out.str().c_str());
  }

  /*********************************
   * Name:    send 
   * Purpose: Send this response to this TCP socket sock
   * Receive: sock - the socket to send to
   * Return:  None
   *********************************/
  void send(TCPSocket& sock);

 private:
  /*********************************
   * Name:    buildStatus 
   * Purpose: private function that builds a data header field that matches the
   *          spec of HTTP
   * Receive: None
   * Return:  None
   *********************************/
  void buildStatus();

  /*********************************
   * Name:    buildTime  
   * Purpose: private function that creates a current time for time-stamping
   *          this response
   * Receive: None
   * Return:  the string for currnet time
   *********************************/
  std::string buildTime();

  unsigned int statusCode;
  std::string version;
  std::string statusDesc;
  std::string content;
  bool chunked;
};

#endif  // _HTTP_RESPONSE_H_
