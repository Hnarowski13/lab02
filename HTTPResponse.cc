#include "HTTPResponse.h"

HTTPResponse::HTTPResponse(unsigned statusCode, const std::string& statusDesc,
    const std::string& version, const std::string& content) {
  setStatusCode(statusCode);
  buildStatus();

  setVersion("HTTP/1.1");

  setHeaderField("Content-Type", "text/html");
  setHeaderField("Server", "MSU/CSE422/SS17-Section001");
  setHeaderField("Connection", "close");  // non-persistent
  setHeaderField("Date", buildTime().c_str());
}

HTTPResponse::~HTTPResponse() {
  version.clear();
  statusDesc.clear();
  content.clear();
}

// NOTE:
// People parse the response differently. The way they slice the header
// varies as well. In this implementation The header MUST END WITH \r\n\r\n.
//
// Examines the HTTP response header in the buffer: data. Make sure the
// header is good.
//
// If the request succeeded, the "Content-Length" indicates the length
// of the response body. According to that value, we know how many
// bytes we need to recevie.
//
// If the request failed, or if the response is not correctly formatted
// return a NULL pointer and release all resource.
HTTPResponse *HTTPResponse::parse(const char* data, unsigned length) {
  HTTPResponse *response = new HTTPResponse();

  // Separate the opening line (for the response) from the rest.
  // find the starting position of the first header (which is the second line)
  const char* firstHeader = response->findNextLine(data, length);

  if (firstHeader == NULL) {
    // Not even a complete first line...
    delete response;
    return NULL;
  }
  size_t firstLineLen = static_cast<size_t>(firstHeader - data);
  std::string responseLine(data, firstLineLen - 2);

  // parse the pieces of the response.
  size_t statusCodePos = responseLine.find(" ");
  size_t statusDescPos = std::string::npos;
  if (statusCodePos != std::string::npos) {
    response->setVersion(responseLine.substr(0, statusCodePos));
    statusDescPos = responseLine.find(" ", statusCodePos + 1);
  }

  if (statusDescPos != std::string::npos) {
    std::string statusCodeStr =
        responseLine.substr(statusCodePos + 1,
            statusDescPos - statusCodePos - 1);
    response->setStatusCode(statusCodeStr);  // statusCode is updated in it

    if ((response->statusCode < 100) || (response->statusCode >= 600)) {
      // bad status code
      delete response;
      return NULL;
    }

    response->setStatusDesc(responseLine.substr(statusDescPos + 1));
  } else {  // Missing fields = bad response.
    delete response;
    return NULL;
  }

  // Have the header lines parsed now; response line is okay.
  // Handled in HTTPMessage.cc
  bool headersOkay = response->parseFields(firstHeader, length - firstLineLen);

  std::string transferEncoding;
  response->getHeaderValue("Transfer-Encoding", transferEncoding);

  if (transferEncoding.find("chunked") != std::string::npos) {
    // chunked transfer encoding
    response->chunked = true;
  } else {  // default transfer encoding
    response->chunked = false;
  }

  if (headersOkay) {
    return response;
  } else {
    delete response;
    return NULL;
  }
}

HTTPResponse* HTTPResponse::createStandardResponse(
    unsigned contentLen, unsigned statusCode, const std::string& statusDesc,
    const std::string& version) {
  HTTPResponse* response = new HTTPResponse(statusCode, statusDesc, version);

  // Assume we're not bothering with chunked/gzipped data.
  response->setHeaderField("Content-Encoding", "identity");
  response->setHeaderField("Transfer-Encoding", "identity");

  // Also assume that we don't want to have to keep track of connections and
  // we use only non-persistent connection
  response->setHeaderField("Connection", "close");

  // HTTP requires responses to include the data of construction.
  // Therefore, let's set that.
  char timeBuffer[128];
  time_t responseTime = time(NULL);
  strftime(timeBuffer, sizeof(timeBuffer) / sizeof(char),
    "%a, %d %b %Y %H:%M:%S %Z", gmtime(&responseTime));
  response->setHeaderField("Date", timeBuffer);

  // Finally, we know how long the body's going to be, so set that, too.
  std::ostringstream lengthStr;
  lengthStr << contentLen;
  response->setHeaderField("Content-Length", lengthStr.str());

  return response;
}

// For the client, it needs to remove the chunkLen from the data std::string
// because the client is storing the data as a file. The chunk length
// is no longer needed anymore. However, for proxies, they need to keep the
// chunk length, so that the forwarded response body can be decoded/received
// by the clients.
int HTTPResponse::getChunkSize(std::string &data) {
  int chunkLen;      // The value we want to obtain
  int chunkLenStrEnd;  // The var to hold the end of chunk length std::string
  std::stringstream ss;   // For hex to in conversion

  chunkLenStrEnd = data.find("\r\n");  // Find the first CLRF
  std::string chunkLenStr;
  if (chunkLenStrEnd != std::string::npos) {
    chunkLenStr = data.substr(0, chunkLenStrEnd);
  } else {
    return chunkLenStrEnd;
  }
  // take the chunk length std::string out

  // convert the chunk length std::string hex to int
  ss << std::hex << chunkLenStr;
  ss >> chunkLen;

  // reorganize the data
  // remove the chunk length std::string and the CLRF
  data = data.substr(chunkLenStrEnd + 2, data.length() - chunkLenStrEnd - 2);

  // cout << "chunkLenStr: " << chunkLenStr << std::endl;
  // cout << "chunkLen:   " << chunkLen << std::endl;

  return chunkLen;
}

void HTTPResponse::receiveHeader(TCPSocket& sock, std::string& responseHeader,
    std::string& responseBody) {
  try {
    sock.readHeader(responseHeader, responseBody);
  } catch (std::string msg) {
    std::cout << "HTTPResponse throw" << msg << std::endl;
    throw msg;
  }
}

int HTTPResponse::receiveBody(TCPSocket& sock, std::string& responseBody,
    int bytesLeft) {
  if (bytesLeft > BUFFER_SIZE) {
    return sock.readData(responseBody, BUFFER_SIZE);
  } else {
    return sock.readData(responseBody, bytesLeft);
  }
}

int HTTPResponse::receiveLine(TCPSocket& sock, std::string& data) {
  return sock.readLine(data);
}

const int HTTPResponse::getContentLen() const {
  int len = 0;
  std::string len_str;
  if (getHeaderValue("Content-Length", len_str) == true) {
    std::istringstream conv(len_str);
    conv >> len;
    return len;
  }
  return -1;
}

void HTTPResponse::print(std::string& outputString) const {
  outputString.clear();
  // Have the sstream library format the response line for us, since we
  // need to turn the status code back into a std::string somehow.
  std::ostringstream responseLine;
  responseLine << version << " " << statusCode << " " << statusDesc;

  // Take that and toss on the ending to get the first line...
  outputString = responseLine.str();
  outputString += lineEnding;

  // ...and then add the associated headers.
  HTTPMessage::print(outputString);
}

void HTTPResponse::print(char* outputBuffer, unsigned bufferLen) const {
  // Similar business, though we have to be more choosy with how we
  // apply sstream.
  std::ostringstream codeStr;
  codeStr << statusCode;

  copyIfRoom(outputBuffer, version.c_str(), bufferLen);
  copyIfRoom(outputBuffer, " ", bufferLen);
  copyIfRoom(outputBuffer, codeStr.str().c_str(), bufferLen);
  copyIfRoom(outputBuffer, " ", bufferLen);
  copyIfRoom(outputBuffer, statusDesc.c_str(), bufferLen);

  copyIfRoom(outputBuffer, lineEnding.c_str(), bufferLen);

  HTTPMessage::print(outputBuffer, bufferLen);
}

void HTTPResponse::send(TCPSocket& sock) {
  std::string outgoingBuffer;
  print(outgoingBuffer);
  outgoingBuffer.append(content);
  sock.writeString(outgoingBuffer);
}

std::string HTTPResponse::buildTime() {
  // format a time
  time_t t;
  struct tm *ts;
  char result[38];
  static char wdayName[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri",
                                "Sat"};
  static char monName[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  // get the time
  t = time(NULL);
  ts = gmtime(&t);

  // format the time std::string according to the specification,
  // e.g. Sun, 06 Nov 1994 08:49:37 GMT)
  snprintf(result, sizeof(result), "%.3s, %.2d %.3s %d %.2d:%.2d:%.2d GMT",
       wdayName[ts->tm_wday], ts->tm_mday, monName[ts->tm_mon],
       1900+ts->tm_year, ts->tm_hour, ts->tm_min, ts->tm_sec);
  return std::string(result);
}

void HTTPResponse::buildStatus() {
  switch (statusCode) {
    case 200:
      statusDesc = "OK";
      break;
    case 400:
      statusDesc = "Bad request";
      break;
    case 403:
      statusDesc = "Forbidden";
      break;
    case 404:
      statusDesc = "Not Found";
      break;
    case 500:
      statusDesc = "Internal server error";
      break;
    case 501:
      statusDesc = "Not implemented";
      break;
    case 503:
      statusDesc = "Service unavailable";
      break;
    default:
      statusDesc = "Code not implemented/recognized";
      break;
  }
}
