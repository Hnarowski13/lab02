#include "TCPSocket.h"
#include <sstream>

void TCPSocket::createSocket() {
  // close the socket if it's already open
  Close();

  // first try to make the TCP socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    throw std::string("TCPSocket Exception: Unable to create socket");
  }
}

void TCPSocket::Connect(const std::string& serverName,
    unsigned short serverPort) {
  hostent *hostEnt;

  createSocket();  // create a socket

  // convert the server name to a valid inet address
  if ((hostEnt = gethostbyname(serverName.c_str())) == NULL) {
    throw std::string("TCPSocket Exception: could not resolve hostname");
  }

  Connect(hostEnt, serverPort);
}

void TCPSocket::Connect(hostent *host, unsigned short serverPort) {
  // create the socket
  createSocket();

  // make sure it's zero to start
  memset(&serverAddr, 0, sizeof(serverAddr));
  // designate it as part of the Internet address family
  serverAddr.sin_family = AF_INET;
  // specify the port, host to network short
  serverAddr.sin_port = htons(serverPort);
  // specify the server IP address in network byte order
  memcpy(&serverAddr.sin_addr, host->h_addr, host->h_length);

  // now actually try to connect
  if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    throw std::string("TCPSocket Exception: connect failed");
  }
}

void TCPSocket::Connect(const URL& url) {
  hostent *hp = gethostbyname(url.getHost().c_str());

  if (hp == NULL) {
    throw std::string("TCPSocket Exception: Unable to resolve URL");
  } else {  // URL resolved successfully
    // If the port is not defined, connect to 80
    if (url.isPortDefined()) {
      Connect(hp, url.getPort());
    } else {
      Connect(hp, 80);
    }
  }
}

void TCPSocket::Bind(unsigned short serverPort) {
  // create the socket
  createSocket();

  // make sure it's zero to start
  memset(&serverAddr, 0, sizeof(serverAddr));
  // designate it as part of the Internet address family
  serverAddr.sin_family = AF_INET;
  // specify the port, host to network short
  serverAddr.sin_port = htons(serverPort);
  // specify the server IP address in network byte order
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    throw std::string("TCPSocket Exception: could not bind to interface");
  }
}

void TCPSocket::Listen() {
  // listen on socket sock, report error when fail
  if (listen(sock, 1) < 0) {
     throw std::string("TCPSocket Exception: listen call failed");
  }

  socklen_t serverAddrLen = sizeof(serverAddr);
  if (getsockname(sock, (sockaddr *) &serverAddr, &serverAddrLen) < 0) {
    throw std::string("TCPSocket Exception: Unable to obtain socket information.");
  }
}

bool TCPSocket::Accept(TCPSocket& dataSock) {
  int newSock;
  socklen_t sinSize;

  sinSize = sizeof(struct sockaddr_in);
  // waiting for new incoming connection
  if ((newSock = accept(sock, (struct sockaddr *) &(dataSock.serverAddr),
      &sinSize)) < 0) {
    throw std::string("TCPSocket Exception: could not accept incoming connection");
    return false;
  }

  dataSock.sock = newSock;
  return true;
}

TCPSocket *TCPSocket::Accept() {
  TCPSocket* newSock = new TCPSocket();
  Accept(*newSock);

  return newSock;
}

int TCPSocket::Close() {
  if (sock != -1) {  // If this socket is in use
    if (close(sock) < 0) {
      return -1;
    }
  }
  sock = -1;
  return 0;
}

int TCPSocket::writeString(const std::string& data) {
  int bytesSent = 0;

  if ((bytesSent = send(sock, (void *)data.data(), data.size(), 0)) < 0) {
    throw std::string("TCPSocket Exception: error sending data");
  }

  return bytesSent;
}

int TCPSocket::readString(std::string& data) {
  int bytesReceived;

  if ((bytesReceived = recv(sock, (void *)data.data(), data.size(), 0)) < 0) {
    throw std::string("TCPSocket Exception: error reading data from socket");
  }
  data = data.substr(0, bytesReceived);
  data += '\0';

  return bytesReceived;
}

int TCPSocket::readNBytes(void* vptr, unsigned int n) {
  size_t  nLeft;
  ssize_t nRead;
  char    *ptr;

  ptr = (char *) vptr;
  nLeft = n;

  while (nLeft > 0) {  // keeps reading until n is satisfied
    if ((nRead = read(sock, ptr, nLeft)) < 0) {  // something is wrong
      return -1;
    } else if (nRead == 0) {  // nothing's in the socket, stop
      break;
    }
    nLeft -= nRead;
    ptr += nRead;
  }

  return (n - nLeft);
}

int TCPSocket::readLine(void *vptr, unsigned int maxLen) {
  int n, readCount;
  char c, *ptr;

  ptr = (char *) vptr;
  for (n = 1; n < maxLen; n++) {
    readCount = read(sock, &c, 1);  // Keeps receiving, one byte by one byte
    if (readCount == 1) {
      *ptr++ = c;
      if (c == '\n') {  // check if the byte is newline
        break;  // break and end this function is yes
      }
    } else if (readCount == 0) {
      if (n == 1) {
        return 0;
      } else {
        return n;
      }
    } else {  // readCount < 0
      return -1;
    }
  }
  *ptr = 0;
  return n;
}

int TCPSocket::receiveHeaders(char *buffer, unsigned int bufferLen,
    unsigned int& totalReceivedLen) {
  static const char headerEnd[] = {'\r', '\n', '\r', '\n'};
  static const unsigned headerEndLen = sizeof(headerEnd);

  // Piece-by-piece, buffer the server's response and look for the end
  // of the headers. Make a note of where in the buffer the end occurs.
  int bytesReceived = 0;
  int headerEndPos = -1;
  int headerEndRead = 0;

  while ((bytesReceived < bufferLen) && (headerEndPos < 0)) {
    // Grab however many bytes are waiting for us right now.
    int receivedPiece = read(sock, buffer + bytesReceived,
                             bufferLen - bytesReceived);
    if (receivedPiece == -1) {
      // Something's wrong. If we cannot receive, reutrn -1
      return -1;
    }

    // Go over what we got in the buffer and look for the end of headers
    int i;
    for (i = bytesReceived;
         i < (bytesReceived + receivedPiece) && (headerEndRead < headerEndLen);
         i++) {
      if (buffer[i] == headerEnd[headerEndRead]) {
        headerEndRead++;
      } else {
        headerEndRead = 0;
      }
    }

    // If we found the end, mark it.  Also keep track of how much
    // we've read total, for several reasons (not filling the
    // buffer; knowing how much we've read past the header, etc.).
    if (headerEndRead >= headerEndLen) {
      headerEndPos = i;
    }
    bytesReceived += receivedPiece;
  }
  totalReceivedLen = bytesReceived;

  return headerEndPos;
  // Note that this headerEndPos here includes \r\n\r\n
}

// Receive a piece of response and extract the header portion from it.
// Stores the header in the std::string header and store the rest in the
// std::string data.
// One can check if the header is good by checking the length of header.
void TCPSocket::readHeader(std::string& header, std::string& data) {
  char buffer[BUFFER_SIZE];
  unsigned int total = 0;

  int headerEndPos = receiveHeaders(buffer, BUFFER_SIZE - 1, total);

  if (headerEndPos < 0) {
    throw std::string("TCPSocket Exception: Error receiving response header.");
  } else {
    // Store the received header and data into
    header.append(buffer, headerEndPos);
    data.append(buffer + headerEndPos, total - headerEndPos);
  }
}

int TCPSocket::readData(std::string& data, unsigned int bytesLeft) {
  int total = 0, bytesRead;
  char buffer[BUFFER_SIZE];

  while (total < bytesLeft) {
    memset(buffer, 0, sizeof(buffer));

    bytesRead = readNBytes(buffer, bytesLeft);

    if (bytesRead < 0) {
      throw std::string("TCPSocket Exception: error reading data from socket");
      return -1;
    } else if (bytesRead == 0) {
      break;
    }

    data.append(buffer, bytesRead);
    total += bytesRead;
  }

  return total;
}

int TCPSocket::readLine(std::string& data) {
  char buffer[BUFFER_SIZE];
  int bytesRead;

  memset(buffer, 0, BUFFER_SIZE);
  if ((bytesRead = readLine(buffer, BUFFER_SIZE)) < 0) {
    throw std::string("TCPSocket Exception: error reading line from socket");
  }

  buffer[bytesRead] = 0;
  data += buffer;

  return bytesRead;
}

void TCPSocket::getPort(unsigned short& gettingPort) {
  gettingPort = ntohs(serverAddr.sin_port);
}
