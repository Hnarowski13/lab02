/*********************************
 * URL - Represents most aspects of a uniform resource identifier (URL).  Can
 * be used to parse existing URL strings into their component parts, and to form
 * new URL strings piece-by-piece from those components.
 *
 * Expects URLs to be formatted in the following manner (note that most fields
 * are optional, depending on which other fields are also present):
 *
 * protocol: *host:port/path?query#fragment
 *********************************/

#ifndef _URL_H_
#define _URL_H_

#include <iostream>
#include <string>

class URL {
 public:
  /*********************************
   * Name:    URL
   * Purpose: constructor of URL class objects
   * Receive: None
   * Return:  None
   *********************************/
  URL();

  /*********************************
   * Name:    ~URL
   * Purpose: destructor of URL class objects
   * Receive: None
   * Return:  None
   *********************************/
  ~URL();

  /*********************************
   * Name:    parse
   * Purpose: Creates a new URL based on the contents of the given string.
   * Receive: urlString - The URL string to parse.
   * Return:  A URL object with its components taken from the given
   *          string. If the given string is not formatted like a proper 
   *          URL and cannot be parsed, a NULL pointer will be returned 
   *          instead.
   *********************************/
  static URL* parse(const std::string& urlString);

  /*********************************
   * Name:    isHtml
   * Purpose: Check if the path in the given string points to an HTML file
   *          does NOT check if the path is valid 
   * Receive: urlString - The URL string to checked.
   * Return:  true if the path is an html, false otherwise 
   *********************************/
  static bool isHtml(const std::string& pathString);

  /*********************************
   * Name:    getProtocol
   * Purpose: Looks up the protocol given in the URL.
   * Receive: None
   * Return:  The URL's protocol.
   *********************************/
  const std::string& getProtocol() const;

  /*********************************
   * Name:    getHost
   * Purpose: Looks up the target host of the URL.
   * Receive: None
   * Return:  The URL's host.
   *********************************/
  const std::string& getHost() const;

  /*********************************
   * Name:    isPortDefined
   * Purpose: Checks if the URL refers to a specific port, or none at all.
   * Receive: None
   * Return:  true if the URL has a defined port, false if not.
   *********************************/
  bool isPortDefined() const;

  /*********************************
   * Name:    getPort
   * Purpose: Looks up the port number to which the URL refers.
   * Receive: None
   * Return:  The URL's port. If isPortDefined() returns false, this
   *          value is meaningless.
   *********************************/
  unsigned getPort() const;

  /*********************************
   * Name:    getPath
   * Purpose: Looks up the path of the resource to which the URL refers.
   * Receive: None
   * Return:  The URL's path.
   *********************************/
  const std::string& getPath() const;

  /*********************************
   * Name:    getQuery 
   * Purpose: Looks up the query part of the URL (which may be used to identify
   *          a resource in a non-hierarchical manner, unlike the path).
   * Receive: None
   * Return:  The URL's query.
   *********************************/
  const std::string& getQuery() const;

  /*********************************
   * Name:    getFragment
   * Purpose: Looks up the fragment of the primary resource to which the URL
   *          specifically refers (e.g. an anchor in a web page).
   * Receive: None
   * Return:  The URL's fragment.
   *********************************/
  const std::string& getFragment() const;

  /*********************************
   * Name:    print
   * Purpose: Has the URL printed to the given output stream, in standard format.
   * Receive: out - The output stream to which to print the URL.
   * Return:  None
   *********************************/
  void print(std::ostream& out);

  /*********************************
   * Name:    print
   * Purpose: Has the URL printed into the given string, in standard format.
   * Receive: target - Will be set to a string representation of this URL.
   * Return:  None
   *********************************/
  void print(std::string& target);

  /*********************************
   * Name:    setProtocol
   * Purpose: Sets the URL protocol to the given string.
   * Receive: protocol - The protocol to set.
   * Return:  None
   *********************************/
  void setProtocol(const std::string& protocol);

  /*********************************
   * Name:    setHost
   * Purpose: Sets the URL's host to the given string.
   * Receive: host - The host to set.
   * Return:  None
   *********************************/
  void setHost(const std::string& host);

  /*********************************
   * Name:    clearPort 
   * Purpose: Throws out the port of the URL, making it undefined.
   * Receive: None
   * Return:  None
   *********************************/
  void clearPort();

  /*********************************
   * Name:    setPort
   * Purpose: Sets a specific port number for the URL.
   * Receive: port - The port to set.
   * Return:  None
   *********************************/
  void setPort(unsigned short port);

  /*********************************
   * Name:    setPath
   * Purpose: Sets the path of the resource to which the URL refers.
   * Receive: path - The path to set.
   * Return:  None
   *********************************/
  void setPath(const std::string& path);

  /*********************************
   * Name:    setQuery 
   * Purpose: Sets the query string for the URL.
   * Receive: query - The query to set.  Can be used to effectively delete the
   *                  URL's query by passing in a blank string.
   * Return:  None
   *********************************/
  void setQuery(const std::string& query);

  /*********************************
   * Name:    setFragment
   * Purpose: Sets the fragment for the URL.
   * Receive: fragment - The fragment to set. Will effectively delete the URL's
   *                     fragment if you pass in a blank string.
   * Return:  None
   *********************************/
  void setFragment(const std::string& fragment);

 private:
  /*********************************
   * Name:    readProtocol
   * Purpose: read the protocol from the URL string
   * Receive: urlString - the URL string to be parsed
   *          offset - the offset, the position to start parsing
   *                   default = 0
   * Return:  the offset indicates that the part before this offset
   *          has been parsed
   *********************************/
  size_t readProtocol(const std::string& urlString, size_t offset = 0);

  /*********************************
   * Name:    readHostPort
   * Purpose: read the port from the URL string, if specified
   * Receive: urlString - the URL string to be parsed
   *          offset - the offset, the position to start parsing
   * Return:  the offset indicates that the part before this offset
   *          has been parsed
   *********************************/
  size_t readHostPort(const std::string& urlString, size_t offset);

  /*********************************
   * Name:    readPathDetails
   * Purpose: read the path from the URL string, if specified
   * Receive: urlString - the URL string to be parsed
   *          offset - the offset, the position to start parsing
   * Return:  the offset indicates that the part before this offset
   *          has been parsed
   *********************************/
  size_t readPathDetails(const std::string& urlString, size_t offset);

  std::string protocol;
  std::string host;
  unsigned short port;
  std::string path;
  std::string query;
  std::string fragment;
};


#endif  // _URL_H_
