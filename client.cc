// Simple HTTP client for CSE422 SS17 lab 02.
#include "client.h"

int main(int argc, char* argv[]) {
  signal(SIGPIPE, SIG_IGN);  // To ignore SIGPIPE

  char* serverAddr = NULL;
  char* proxyAddr = NULL;
  URL* serverUrl = NULL;
  URL* proxyUrl = NULL;
  HTTPRequest* request = NULL;
  HTTPResponse* response = NULL;
  FILE* out = NULL;

  parseArgs(argc, argv, &serverAddr, &proxyAddr);

  /***PARSE THE ADDRS RECEIVED TO URL OBJECTS***/
  // Must have a server to get data from
  if (!serverAddr) {
    std::cerr << "You did not specify the host address." << std::endl;
    helpMessage(argv[0], std::cout);
    exit(1);
  }

  serverUrl = URL::parse(serverAddr);
  if (!serverUrl) {  // If URL parsing is failed
    std::cerr << "Unable to parse host address: " << serverAddr
              << std::endl;
    helpMessage(argv[0], std::cout);
    exit(1);
  }

  // Proxy is an otional argument
  // If a proxy URL is specified, the client connects to the proxy and
  // interacts with the proxy. Otherwise, the client connects to the host
  // and interacts with the host.
  proxyUrl = NULL;
  if (proxyAddr) {
    proxyUrl = URL::parse(proxyAddr);
    if (!proxyUrl) {
      std::cerr << "Unable to parse proxy address: " << proxyAddr
                << std::endl;
      helpMessage(argv[0], std::cout);
      exit(1);
    }
    if (!(proxyUrl->isPortDefined())) {
      std::cout << "Proxy port is not defined, assumed to be 8080" << std::endl;
      proxyUrl->setPort(8080);
    }
  }
  /***END OF PARSING THE ADDRS RECEIVED TO URL OBJECTS***/


  /***CREATE A TCPSocket OBJECT AND CONNECT TO THE URL***/
  // TCPSocket class to handle TCP communications.
  TCPSocket clientSock;
  if (!proxyAddr) {  // proxy not specified, connect to the host directly.
    try {
      clientSock.Connect(*serverUrl);  // Connect to the target server.
    } catch(std::string msg) {
      // Give up if sock is not created correctly.
      std::cerr << msg << std::endl;
      std::cerr << "Unable to connect to server: "
                << serverUrl->getHost() << std::endl;
      delete serverUrl;
      exit(1);
    }
  } else {  // proxy is specified, connect to proxy
    try {
      clientSock.Connect(*proxyUrl);  // Connect to the proxy, instead of server
    } catch(std::string msg) {
      // Give up if sock is not created correctly.
      std::cout << msg << std::endl;
      std::cout << "Unable to connect to proxy: "
                << proxyUrl->getHost() << std::endl;
      delete serverUrl;
      delete proxyUrl;
      exit(1);
    }
  }
  /***END OF CREATING A TCPSocket OBJECT AND CONNECT TO THE URL***/




  /***SEND THE REQUEST TO THE SERVER***/
  // Send a GET request for the specified file.
  // No matter connecting to the server or the proxy, the request is
  // alwasy destined to the server.
  request = HTTPRequest::createGetRequest(serverUrl->getPath());
  request->setHost(serverUrl->getHost());
  // set this request to non-persistent.
  request->setHeaderField("Connection", "close");
  // For real browsers, If-Modified-Since field is always set.
  // if the local object is the latest copy, the browser does not
  // respond the object.
  request->setHeaderField("If-Modified-Since", "0");

  try {  // send the request to the sock
    request->send(clientSock);
  } catch(std::string msg) {  // something is wrong, send failed
    std::cerr << msg << std::endl;
    exit(1);
  }

  // get the request as a std::string
  std::string printBuffer;
  request->print(printBuffer);

  // output the request
  std::cout << "Request sent..." << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  std::cout << printBuffer.substr(0, printBuffer.size() - 4) << std::endl;
  std::cout << "=========================================================="
            << std::endl;

  delete request;  // We do not need it anymore
  /***END OF SENDING REQUEST***/








  /***RECEIVE RESPONSE HEADER FROM THE SERVER***/
  // The server response is a stream starts with a header and then
  // the body/data. A blank line separates the header and the body/data.
  //
  // Read enough of the server's response to get all of the headers,
  // then have that response interpreted so we at least know what
  // happened.
  //
  // We create two std::strings to hold the incoming data. As described in the
  // hanout, a HTTP message is composed of two portions, a header and a body.
  std::string responseHeader, responseBody;

  // The client receives the response stream. Check if the data it has
  // contains the whole header.
  // read_header separates the header and data by finding the blank line.
  try {
    response->receiveHeader(clientSock, responseHeader, responseBody);
  } catch (std::string msg) {
    std::cerr << msg << std::endl;
  }

  // The HTTPResponse::parse construct a response object. and check if
  // the response is constructed correctly. Also it tries to determine
  // if the response is chunked transfer encoding or not.
  response = HTTPResponse::parse(responseHeader.c_str(),
                                 responseHeader.length());

  // The response is illegal.
  if (!response) {
    std::cerr << "Client: Unable to parse the response header." << std::endl;
    // clean up if there's something wrong
    delete response;
    if (proxyUrl) {
      delete proxyUrl;
    }
    delete serverUrl;
    exit(1);
  }

  // get the response as a std::string
  response->print(printBuffer);

  // output the response header
  std::cout << std::endl << "Response header received" << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  std::cout << printBuffer.substr(0, printBuffer.length() - 4) << std::endl;
  std::cout << "=========================================================="
            << std::endl;

  /***END OF RECEIVING RESPONSE HEADER FROM THE SERVER***/












  /***GET REST OF THE MESSAGE BODY AND STORE IT***/
  // Open a local copy in which to store the file.
  out = OpenLocalCopy(serverUrl);
  // check
  if (!out) {
    std::cerr << "Error opening local copy for writing." << std::endl;
    // clean up if failed
    if (!proxyAddr) {
      delete proxyUrl;
    }
    delete serverUrl;
    exit(1);
  }


  int bytesWritten = 0, bytesLeft;

  if (!(response->isChunked()) &&  // neither chunked transfer encoding
      response->getContentLen() == -1) {  // nor default transfer encoding
    std::cout << "The response is neither default tranfer encoding "
              << "nor chunked transfer encoding. This response is not "
              << "supported. Terminating the program without saving the file."
              << std::endl;
  } else if (!(response->isChunked())) {
    std::cout << std::endl << "Downloading rest of the file ... " << std::endl;
    // default transfer encoding does not split the data into
    // chunks. The header specifies a Content-Length field. The client knows
    // exactly how many data it is expecting. The client keeps receiving
    // the response until it gets the amount specified.

    std::cout << "Default transfer encoding" << std::endl;
    std::cout << "Content-length: " << response->getContentLen() << std::endl;
    bytesLeft = response->getContentLen();

    do {
      // If we got a piece of the file in our buffer for the headers,
      // have that piece written out to the file, so we don't lose it.
      fwrite(responseBody.c_str(), 1, responseBody.length(), out);
      bytesWritten += responseBody.length();
      bytesLeft -= responseBody.length();

      std::cout << "bytes written:" <<  bytesWritten << std::endl;
      std::cout << "data gotten:" <<  responseBody.length() << std::endl;

      responseBody.clear();
      try {
        // Keeps receiving until it gets the amount it expects.
        response->receiveBody(clientSock, responseBody, bytesLeft);
      } catch(std::string msg) {
        // something bad happend
        std::cerr << msg << std::endl;
        // clean up
        delete response;
        delete serverUrl;
        if (proxyAddr) {
          delete proxyUrl;
        }
        fclose(out);
        clientSock.Close();
        exit(1);
      }
    } while (bytesLeft > 0);
  } else {  // chunked encoding
    std::cout << std::endl << "Downloading rest of the file ... " << std::endl;
    std::cout << "Chunked transfer encoding" << std::endl;

    // As mentioned above, receiveHeader function already split the
    // body from the header from us. The beginning of this respnse_data
    // now holds the first chunk size.
    int chunkLen = HTTPResponse::getChunkSize(responseBody);
    int totalData = chunkLen;

    while (1) {
      std::cout << "       chunk length: " << chunkLen << std::endl;
      std::cout << "responseBody length: " << responseBody.length()
                << std::endl;
      if (chunkLen == 0) {  // the end of response body
        break;
      } else if (chunkLen == -1) {
        // If chunk length is not found
        // It is possible that the receieveHeader gets exactly only the
        // header and the first chunk length is not recevied yet. In this
        // case, getChunkSize returns -1. Receive more to get the chunk length
        response->receiveLine(clientSock, responseBody);
        chunkLen = HTTPResponse::getChunkSize(responseBody);
      } else if (responseBody.length() < chunkLen) {
        try {
          // If current data holding is less than the chunkLen, this
          // piece of data contains only part of this chunk. Receive more
          // until we have a complete chunk to store!
          // receive more until we have the whole chunk.
          response->receiveBody(clientSock, responseBody,
                                (chunkLen - responseBody.length()));
          response->receiveLine(clientSock, responseBody);
          // get the blank line between chunks
          response->receiveLine(clientSock, responseBody);
          // get next chunk, at least get the chunk size
        } catch(std::string msg) {
          // something bad happend
          std::cerr << msg << std::endl;
          // clean up
          delete response;
          delete serverUrl;
          if (proxyAddr) {
            delete proxyUrl;
          }
          fclose(out);
          clientSock.Close();
          exit(1);
        }
      } else {
        // If current data holding is longer than the chunk size, this
        // piece of data contains more than one chunk. Store the chunk.
        fwrite(responseBody.c_str(), 1, chunkLen, out);
        bytesWritten += chunkLen;

        // reorganize the data, remove the chunk from it
        // the + 2 here is to consume the extra CLRF

        responseBody = responseBody.substr(chunkLen + 2,
                           responseBody.length() - chunkLen - 2);
        response->receiveLine(clientSock, responseBody);
        // get the blank line between chunks
        response->receiveLine(clientSock, responseBody);
        // get next chunk, at least get the chunk size

        // get next chunk size
        chunkLen = HTTPResponse::getChunkSize(responseBody);

        totalData += chunkLen;
      }
    }

    // This checks if the chunked encoding transfer mode is downloading
    // the contents correctly.
    if ((totalData != bytesWritten) && response->isChunked()) {
      std::cout << "WARNING" << std::endl
                << "Data received does not match the sum of chunks."
                << std::endl;
    }
    std::cout << "Download complete (" << bytesWritten
              << " bytes written)" << std::endl;
  }


  // If the response is not OK, something is wrong.
  // However, we still downloaded the content, because even the response
  // is not 200. The server still replies with an error page (403, 404 ...)
  if (response->getStatusCode() != 200) {
    std::cerr << response->getStatusCode() << " "
              << response->getStatusDesc() << std::endl;
  }

  // everything's done.
  clientSock.Close();

  delete response;
  delete serverUrl;
  if (proxyAddr) {
    delete proxyUrl;
  }
  fclose(out);

  return 0;
}
