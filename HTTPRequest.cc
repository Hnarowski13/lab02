#include "HTTPRequest.h"

using namespace std;

HTTPRequest::HTTPRequest(const std::string& method, const std::string& path,
    const std::string& version) : method(method), path(path), version(version) {
}

HTTPRequest::~HTTPRequest() {
  method.clear();
  path.clear();
  version.clear();
}

HTTPRequest *HTTPRequest::receive(TCPSocket& sock) {
  HTTPRequest *request;
  std::string incomingRequestString;
  std::string aLine;
  int zeroCount = 0;  // number of attempt to read from socket

  sock.readLine(aLine);  // get a line from the TCPSocket

  while (aLine != "\r\n") {
    incomingRequestString += aLine;
    aLine.clear();  // clear the variable before using it again
    if (sock.readLine(aLine) == 0) {  // failed to get data from the TCPSocket
      zeroCount++;
      if (zeroCount >= 1000) {
        break;
      }
    }
  }
  incomingRequestString.append("\r\n");

  request = HTTPRequest::parse(incomingRequestString.c_str(),
                               incomingRequestString.size());
  return request;
}

HTTPRequest* HTTPRequest::parse(const char* data, unsigned length) {
  HTTPRequest* request = new HTTPRequest();

  // Separate the opening line (for the request) from the rest.
  // find the starting position of the first header (which is the second line)
  const char* firstHeader = request->findNextLine(data, length);

  if (firstHeader == NULL) {
    // Ouch, not even a complete first line...
    delete request;
    return NULL;
  }

  // first line should look something like this
  // GET /~cse422/ HTTP/1.1\r\n
  size_t firstLineLen = static_cast<size_t>(firstHeader - data);

  // Figure out that opening request line. Look for the spaces that
  // separate the method, URL, and version.  Set as appropriate.
  // firstLineLen - 2 is to get rid of \r\n
  // requestLine == "GET /~cse422/ HTTP/1.1"
  std::string requestLine(data, firstLineLen - 2);

  size_t urlPos = requestLine.find(" ");
  size_t versionPos = std::string::npos;
  if (urlPos != std::string::npos) {
    request->setMethod(requestLine.substr(0, urlPos));  // obtained "GET"
    versionPos = requestLine.find(" ", urlPos + 1);
  }

  std::string path;

  if (versionPos != std::string::npos) {
    path = requestLine.substr(urlPos + 1, versionPos - urlPos - 1);
    // obtained "/~cse422/"
    // We are not sure if the path field here is the whole URL or just the path.
    // URL = http://host/path
    // For example: http://www.cse.msu.edu/~cse422
    //        host: www.cse.msu.edu
    //        path: ~cse422
    // We will parse it later when we get the host.
    request->setVersion(requestLine.substr(versionPos + 1));
  } else {
    // If we couldn't get those three fields out of it, it's a bad
    // request, and we should stop trying to handle it.
    delete request;
    return NULL;
  }

  // Go on and handle the remaining header lines in the request. If
  // they're good, we're good.  If not...
  bool headersOkay =
      request->parseFields(firstHeader, length - firstLineLen);

  std::string host;
  request->getHost(host);

  // Get rid of the extra "http://" in path field.
  int pos = path.find("http://");
  if (pos != std::string::npos) {
    path.replace(pos, 7, "");
  }

  // Get rid of the extra host field in path field
  pos = path.find(host);
  if (pos != std::string::npos) {
    path.replace(pos, host.length(), "");
  }
  request->setPath(path);

  if (headersOkay) {
    return request;
  } else {
    delete request;
    return NULL;
  }
}

HTTPRequest* HTTPRequest::parse(const std::string& requestString) {
  return HTTPRequest::parse(requestString.c_str(), requestString.size());
}

HTTPRequest* HTTPRequest::createGetRequest(const std::string& path,
    const std::string& version) {
  HTTPRequest* request = new HTTPRequest("GET", path, version);
  return request;
}

void HTTPRequest::send(TCPSocket& sock) {
  std::string outgoingBuffer;
  print(outgoingBuffer);
  sock.writeString(outgoingBuffer);
}

const std::string HTTPRequest::getUrl() const {
  std::string urlString = "http://";
  std::string serverHost, serverPath;
  getHost(serverHost);  // www.cse.msu.edu
  serverPath = getPath();  // /~cse422/
  urlString += serverHost;
  urlString += serverPath;
  return urlString;
}

void HTTPRequest::getHost(std::string& outHost) const {
  if (!getHeaderValue("Host", outHost)) {
    outHost = "";
  }
}

void HTTPRequest::print(std::string& outputString) const {
  outputString.clear();
  // Throw in our one request line.
  outputString = method;
  outputString += ' ';
  outputString += path;
  outputString += ' ';
  outputString += version;

  outputString += lineEnding;

  // Now have all the headers thrown in on top of that.
  HTTPMessage::print(outputString);
}

void HTTPRequest::print(char* outputBuffer, unsigned bufferLength) const {
  // Similar model as the above print, except with a character buffer.
  copyIfRoom(outputBuffer, method.c_str(), bufferLength);
  copyIfRoom(outputBuffer, " ", bufferLength);
  copyIfRoom(outputBuffer, path.c_str(), bufferLength);
  copyIfRoom(outputBuffer, " ", bufferLength);
  copyIfRoom(outputBuffer, version.c_str(), bufferLength);

  copyIfRoom(outputBuffer, lineEnding.c_str(), bufferLength);

  HTTPMessage::print(outputBuffer, bufferLength);
}
