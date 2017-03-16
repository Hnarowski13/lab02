/*********************************
 * HTTPMessage - Base class for HTTP requests and responses. Defines the
 * methods for accessing the various headers on a request/response. Also
 * defines some internal things that are shared by the request/response classes.
 *********************************/

#ifndef _HTTP_MESSAGE_H_
#define _HTTP_MESSAGE_H_

#include <string.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace {
  // The exact string of characters used to represent HTTP line endings.
  const std::string lineEnding = "\r\n";

  // The character used to separate the name of a header from its value.
  const char headerDelimiter = ':';
}

class HTTPMessage {
 public:
  /*********************************
   * Name:    ~HTTPMessage
   * Purpose: destructor of HTTPMessage class objects
   * Receive: None
   * Return:  None
   *********************************/
  virtual ~HTTPMessage();

  /*********************************
   * Name:    getNumHeaderFields
   * Purpose: indicates how many header fields the message has.
   * Receive: None
   * Return:  the number of header fields stored in the message.
   *********************************/
  unsigned getNumHeaderFields() const;

  /*********************************
   * Name:    getHeaderSet
   * Purpose: copies all of the message's headers into the given vector.
   *          Use this if you need to iterate through the headers.  If you 
   *          know the name of the header you want, getHeaderValue() is 
   *          far more useful.
   * Receive: outSet - will be set to a collection of 
   *                   std::pair<std::string, std::string> representing all of 
   *                   the header<name, value> stored in the message
   * Return:  None
   *********************************/
  void getHeaderSet(std::vector<std::pair<std::string, std::string> >& outSet)
      const;

  /*********************************
   * Name: 
   * Purpose: retrieves the value of the header with the given name.
   * Receive: name - the name of the header to look up.
   *          outValue - Will be set to that header's value, if it is 
   *                     found. If no header with that name is found, 
   *                     value will be undefined.
   * Return:  true if the requested header name was found in the message
   *          (in which case, outValue is valid);
   *          false if the requested header name was not found.
   *********************************/
  bool getHeaderValue(const std::string& name, std::string& outValue) const;

  /*********************************
   * Name:    setHeaderField
   * Purpose: Updates the message to have the given header field.  
   *          Overwrites the old value of the specified header if 
   *          the message already had it.
   * Receive: field - The name/value of the header to set.
   * Return:  None
   *********************************/
  void setHeaderField(const std::pair<std::string, std::string>& field);

  /*********************************
   * Name:    setHeaderField
   * Purpose: Updates the given header field in the message. If the 
   *          header is not already present, it will be added to the 
   *          message.  If the header *is* already present, its previous 
   *          value will be overwritten.
   * Receive: name - The name of the header to set.
   *          value - The new value to set.
   * Return:  None
   *********************************/
  void setHeaderField(const std::string& name, const std::string& value);

 protected:
  /*********************************
   * Name:    ~HTTPMessage
   * Purpose: destructor of HTTPMessage class objects
   * Receive: None
   * Return:  the number of headers stored in the message.
   *********************************/
  HTTPMessage();

  /*********************************
   * Name:    parseFields 
   * Purpose: parse the received data to construct the object
   * Receive: the data to be parsed and the length of the data
   * Return:  true if the data is valid, false otherwise.
   *********************************/
  bool parseFields(const char* data, unsigned length);

  /*********************************
   * Name:    print
   * Purpose: construct a string that represents this message, for
   *          sending or other purposes.
   * Receive: outputString - the string to hold the message.
   * Return:  None
   *********************************/
  virtual void print(std::string& outputString) const;

  /*********************************
   * Name:    print
   * Purpose: construct a string that represents this message, for
   *          sending or other purposes
   * Receive: outputBuffer - the char array to hold the message.
   *          bufferLength - the lengthe of the buffer
   * Return:  None
   *********************************/
  virtual void print(char* outputBuffer, unsigned bufferLength) const;

  /*********************************
   * Name:    copyIfRoom
   * Purpose: copy the dataString into the buffer, if the buffer stil
   *          has room for this dataString.
   * Receive: outputBuffer - the char array to store dataString
   *          dataString - the string to be copied
   *          remainingLength - the remaining room of the buffer
   * Return:  None
   *********************************/
  virtual void copyIfRoom(char*& outputBuffer, const char* dataString,
                          unsigned& remainingLength) const;

  /*********************************
   * Name:    findNextLine
   * Purpose: scan the data char-by-char until a newline char is found.
   * Receive: data - the char array to be scaned
   *          length - the length of the data
   * Return:  the pointer points to the beginning of next line.
   *********************************/
  const char* findNextLine(const char* data, unsigned length) const;

 private:
  std::map<std::string, std::string> headers;
};

#endif  // _HTTP_MESSAGE_H_
