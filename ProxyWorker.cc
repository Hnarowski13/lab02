#include "ProxyWorker.h"
#include <sstream>

const std::string ProxyWorker::subliminalTag = "CSE422";

ProxyWorker::ProxyWorker(TCPSocket *cs) {
  clientSock = cs;
  port = 80;  // For a full blown proxy, the server information should be
              // obtained from each request. However, we simply assume it
              // to be 80 for our labs.
  serverUrl = NULL;  // Must be obtain from each request.
  serverResponse = NULL;
  clientRequest = NULL;
}

ProxyWorker::~ProxyWorker() {
  if (serverUrl) {
    delete serverUrl;
  }

  if (clientRequest) {
    delete clientRequest;
  }

  if (serverResponse) {
    delete serverResponse;
  }
  serverSock.Close();
}

void ProxyWorker::handleRequest() {
  std::string buffer;

  // Get HTTP request from the client, check if the request is valid by
  // parsing it. (parsing is done using HTTPRequest::receive)
  // Please refer to HTTPRequest class for the usage of HTTPRequest::receive
  // and/or HTTPRequest::prarse.
  // From the parsed request, obtain the server address (in code, serverUrl).
  std::cout << "New connection established." << std::endl;
  std::cout << "New proxy child thread started." << std::endl;
  std::cout << "Getting request from client..." << std::endl;

  if (!getRequest()) {
    return;
  }

  // get the request as a std::string
  clientRequest->print(buffer);

  // Just outputting the requrst.
  std::cout << std::endl << "Received request:" << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  clientRequest->print(buffer);
  std::cout << buffer.substr(0, buffer.length() - 4) << std::endl;
  std::cout << "=========================================================="
            << std::endl;

  // Check if the request is valid
  // Terminate this ProxyWorker if it is not a valid request
    
  std::cout << "Checking request..." << std::endl;
  if (!checkRequest()) {
    return;
  }
  std::cout << "Done. The request is valid." << std::endl;
    
    

  std::string host;
  clientRequest->getHost(host);
  std::cout << std::endl << "Forwarding request to server "
            << host << "..." << std::endl;
  if (!forwardRequest()) {
    return;
  }

  // Receive the response header and modify the server header field
  // Receive the response body. Handle the default and chunked transfor
  // encoding.
    
    
  std::cout << "Getting the response from the server..." << std::endl;
  if (!getResponse()) {
    return;
  }

  // return the response to the client
  std::cout << "Returning the response to the client..." << std::endl;
  if (!returnResponse()) {
    return;
  }

  return;
}

bool ProxyWorker::getRequest() {
    
    // Get the request from the client (HTTPRequest::receive)
    // Chck if the request is received correctly
    //
    // Obtain the serverUrl from the request (HTTPRequest::getUrl)
    
    clientRequest = HTTPRequest::receive(*clientSock);
    if (clientRequest != NULL)
    {
        std::string url = clientRequest->getUrl();
        
        std::cout <<  url<< std::endl;
        std::cout << "not null" << std::endl;
        serverUrl = URL::parse(url);
    }
    return (clientRequest != NULL);


}

bool ProxyWorker::checkRequest() {
    
  // 1. Make sure we're pointing to a server URL
  //    Respond a 404 Not Found if the server is invalid, parse failed
  //    (That is serverUrl == NULL)
  // 2. Filter out any "host" with the keyword "umich.edu"
  //    Note that we are filtering out "host" with "umich.edu".
  //    "path" with umich is allowed.
  //    Respond a 403 forbidden for host with umich.
  // 3. Filter full URL for "harbaugh" or "Harbaugh". If keyword is found,
  //    Note: the request is still valid
  // 4. Insert subliminal message if the requested object is a html and
  //    does not have a subliminal tag
    
  if (serverUrl == NULL) {
      proxyResponse(404);
      return false;
  } else {  // serverUrl is good
      std::string host = serverUrl->getHost();
      std::string path = serverUrl->getPath();
      std::string fullUrl = clientRequest->getUrl();
      std::cout << "host: "<< host << std::endl;
      if (host.find("umich.edu") != std::string::npos) {
          proxyResponse(403);
          return false;
      }
    else if ((fullUrl.find("harbaugh") != std::string::npos) or (path.find("Harbaugh") != std::string::npos)) {
        std::cout << "harbaugh detected, redirecting path and host" << std::endl;
        clientRequest->setPath("/~liuchinj/cse422ss17/lab2_files/whoa.html");
        clientRequest->setHost("cse.msu.edu");
        serverUrl = URL::parse(clientRequest->getUrl());
        
    } else if (URL::isHtml(clientRequest->getPath()) &&  // 4.
               (!ProxyWorker::hasSubliminalTag(clientRequest->getUrl()))) {
        
        // Check if this request has subliminalTag
        // If this request does not contain the subliminalTag, the
        // proxy does not forward this request to the server. Instead, the proxy
        // returns a subliminal message response to the client.
        return subliminalResponse(clientRequest->getUrl(),5);
      
    } else if (ProxyWorker::hasSubliminalTag(clientRequest->getUrl())){ // 4.
      // If this request contains the subliminalTag, the request has
      // been served before. The proxy handles the request like a normal proxy.

      // Therefore, we need to remove the subliminalTag
      std::string path = clientRequest->getPath();
      ProxyWorker::removeSubliminalTag(path);
      clientRequest->setPath(path);
    }
  }
  return true;
}

bool ProxyWorker::forwardRequest() {
  // pass the client request to the server
  // connected to the server
    
    /***SEND THE REQUEST TO THE SERVER***/
    
    try {
        clientSock->Connect(*serverUrl);  // Connect to the target server.
    } catch(std::string msg) {
        // Give up if sock is not created correctly.
        std::cerr << msg << std::endl;
        std::cerr << "Unable to connect to server: "
        << serverUrl->getHost() << std::endl;
        //delete serverUrl;
     //   exit(1);
        proxyResponse(404);
        return false;
        
    }
    
    try {  // send the request to the sock
        clientRequest->send(*clientSock);
    } catch(std::string msg) {  // something is wrong, send failed
        std::cerr << msg << std::endl;
        return false;
       // exit(1);
    }
    return true;
    
}

bool ProxyWorker::getResponse() {
      std::string buffer;
      std::string responseHeader, responseBody;
    
    // The client receives the response stream. Check if the data it has
    // contains the whole header.
    // read_header separates the header and data by finding the blank line.
    try {
        serverResponse->receiveHeader(*clientSock, responseHeader, responseBody);
    } catch (std::string msg) {
        std::cerr << msg << std::endl;
        std::cout << "unable to receive header?" << std::endl;
        return false;
    }
    
    // The HTTPResponse::parse construct a response object. and check if
    // the response is constructed correctly. Also it tries to determine
    // if the response is chunked transfer encoding or not.
    serverResponse = HTTPResponse::parse(responseHeader.c_str(),
                                   responseHeader.length());
    
    // The response is illegal.
    if (!serverResponse) {
        std::cerr << "Client: Unable to parse the response header." << std::endl;
        // clean up if there's something wrong
        //delete response;
        return false;
    }
    
    // get the response as a std::string
    serverResponse->print(buffer);
    
    // output the response header
    std::cout << std::endl << "Response header received" << std::endl;
    std::cout << "=========================================================="
    << std::endl;
    std::cout << buffer.substr(0, buffer.length() - 4) << std::endl;
    std::cout << "=========================================================="
    << std::endl;
    
    /***GET REST OF THE MESSAGE BODY AND STORE IT***/
  // Open a local copy in which to store the file.
  FILE * out = OpenLocalCopy(serverUrl); //HARK!!!!!!!!!!!! We aren't writing to file here so this is probably the wrong type 
															//of data structure to stuff the response into. MAYBE we can just shove it into response Body
  // check
  if (!out) {
    std::cerr << "Error opening local copy for writing." << std::endl;
    
    delete serverUrl;
    exit(1);
  }
  
  int bytesWritten = 0, bytesLeft;

  if (!(serverResponse->isChunked()) &&  // neither chunked transfer encoding
      serverResponse->getContentLen() == -1) {  // nor default transfer encoding
    std::cout << "The response is neither default tranfer encoding "
              << "nor chunked transfer encoding. This response is not "
              << "supported. Terminating the program without saving the file."
              << std::endl;
  } else if (!(serverResponse->isChunked())) {
    std::cout << std::endl << "Downloading rest of the file ... " << std::endl;
    // default transfer encoding does not split the data into
    // chunks. The header specifies a Content-Length field. The client knows
    // exactly how many data it is expecting. The client keeps receiving
    // the response until it gets the amount specified.

    std::cout << "Default transfer encoding" << std::endl;
    std::cout << "Content-length: " << serverResponse->getContentLen() << std::endl;
    bytesLeft = serverResponse->getContentLen();
    
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
        serverResponse->receiveBody(clientSock, responseBody, bytesLeft);
      } catch(std::string msg) {
        // something bad happend
        std::cerr << msg << std::endl;
        // clean up
        delete serverResponse;
        delete serverUrl;
        
        fclose(out);
        clientSock.Close();
        exit(1);
      }
    } while (bytesLeft > 0);
  } else {  // chunked encoding, WOWZA!!!!!!!!!!!we can do this last, gonna be a big copy&paste from client
								// BUTT should just be replacing the correct server variable and destination of the body
	  }
    
    return true;
}

bool ProxyWorker::returnResponse() {
	//
	//IDK if you coded this but I feel like it's weird to use serverResponse for this and for getResponse
	//
    std::cout << "trying to return response.." << std::endl;
    try {
        serverResponse->send(*clientSock);
        std::cout << "response sent successfully (apparently)" << std::endl;
    }
    catch (std::string msg)
    {
        std::cout << "someting went wong" << msg << std::endl;
        return false;
    }
    return true;
}

bool ProxyWorker::hasSubliminalTag(const std::string& url) {
  // Check if the url contains the subliminalTag in its fragment
  URL* requestUrl = URL::parse(url);  // parse it, URL class does it for us
 
  if (!requestUrl) {
    return false;
  }

  int tagPos = url.rfind(subliminalTag);

  // make sure the subliminalTag is at the end of the url.
  if ((tagPos != std::string::npos) &&
      (tagPos + subliminalTag.size() == url.size())) {
    return true;
  } else {
    return false;
  }
}

void ProxyWorker::removeSubliminalTag(std::string& url) {
  size_t tagPos = url.find(subliminalTag);
  // make sure the subliminalTag is at the end of the url.
  if ((tagPos != std::string::npos) &&
      (tagPos + subliminalTag.size() == url.size())) {
    url = url.replace(tagPos, tagPos + subliminalTag.size(), "");
  }
}

bool ProxyWorker::proxyResponse(int statusCode) {
  std::string buffer;
  HTTPResponse proxyResponse(statusCode);

  std::ostringstream oss;
  oss << statusCode;
  std::string page = "<html><body><h1>" + oss.str() + " " +
                     proxyResponse.getStatusDesc() + "</h1></body></html>";
  proxyResponse.setContent(page);

  std::cout << std::endl << "Returning " << statusCode << " to client ..."
            << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  buffer.clear();
  proxyResponse.print(buffer);
  std::cout << buffer.substr(0, buffer.length() - 4) << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  proxyResponse.send(*clientSock);
  return true;
}

bool ProxyWorker::subliminalResponse(const std::string& url, int duration) {
  std::string buffer;
  // create a new HTTPResponse
  HTTPResponse proxyResponse(200);
  // Randomly choose a image
  int figNumber = rand() % 4;

  std::stringstream ss;
  ss << figNumber;
  std::string figNumberStr = ss.str();
  ss.str("");
  ss << duration;
  std::string durationStr = ss.str();

  // create a webpage containing the image and automatically redirects to
  // original url in "duration" seconds
  proxyResponse.setContent("<html><head><meta http-equiv=\"refresh\" content=\"" + durationStr + ";url=" + url +subliminalTag + "\" /></head><body><center><font size=72>GO GREEN! GO WHITE!</font><br><img src=\"http://www.cse.msu.edu/~liuchinj/cse422ss17/images/" + figNumberStr + ".jpg\" width=800px><br>Redirecting...</center></body></html>");

  std::cout << std::endl << "Returning subliminal to client ..." << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  buffer.clear();
  proxyResponse.print(buffer);
  std::cout << buffer.substr(0, buffer.length() - 4) << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  proxyResponse.send(*clientSock);
  return true;
}
