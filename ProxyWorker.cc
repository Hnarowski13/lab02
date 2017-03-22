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
    
//  std::cout << "Checking request..." << std::endl;
//  if (!checkRequest()) {
//    return;
//  }
//  std::cout << "Done. The request is valid." << std::endl;
//
//  std::string host;
//  clientRequest->getHost(host);
//  std::cout << std::endl << "Forwarding request to server "
//            << host << "..." << std::endl;
//  if (!forwardRequest()) {
//    return;
//  }

  // Receive the response header and modify the server header field
  // Receive the response body. Handle the default and chunked transfor
  // encoding.
    
//    
//  std::cout << "Getting the response from the server..." << std::endl;
//  if (!getResponse()) {
//    return;
//  }
//
//  // return the response to the client
//  std::cout << "Returning the response to the client..." << std::endl;
//  if (!returnResponse()) {
//    return;
//  }
//
//  return;
}

bool ProxyWorker::getRequest() {
  /********TO BE IMPLEMENTED********/
    
    HTTPRequest *request = HTTPRequest::receive(*clientSock);
    if (request != NULL)
    {
        serverUrl = HTTPRequest::getUrl(request);
    }
    return (request == NULL);

  // Get the request from the client (HTTPRequest::receive)
  // Chck if the request is received correctly
  //
  // Obtain the serverUrl from the request (HTTPRequest::getUrl)
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
  //    redirect to host="www.youtube.com" and
  //    path = "/embed/o7iny6VmnlA?autoplay=1"
  //    Note: the request is still valid
  // 4. Insert subliminal message if the requested object is a html and
  //    does not have a subliminal tag
    
  if (true /* 1. complete the condition*/) {
    /********TO BE IMPLEMENTED********/
  } else {  // serverUrl is good
    if (true /* 2. complete the condition*/) {
      /********TO BE IMPLEMENTED********/
    } else if (true /* 3. complete the condition*/) {
      /********TO BE IMPLEMENTED********/
    } else if (URL::isHtml(clientRequest->getPath()) &&  // 4.
               (!ProxyWorker::hasSubliminalTag(clientRequest->getUrl()))) {
      // Check if this request has subliminalTag
      // If this request does not contain the subliminalTag, the
      // proxy does not forward this request to the serer. Instead, the proxy
      // returns a subliminal message response to the client.
      /********TO BE IMPLEMENTED********/
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
  /********TO BE IMPLEMENTED********/
}

bool ProxyWorker::getResponse() {
  /********TO BE IMPLEMENTED********/
}

bool ProxyWorker::returnResponse() {
  /********TO BE IMPLEMENTED********/
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
