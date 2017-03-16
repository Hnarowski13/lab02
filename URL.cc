#include "URL.h"
#include <sstream>

namespace {
  // Used to signal when the port number is not known.  There is an
  // excellent chance that this will never be a valid port for anything.
  const unsigned short UNDEFINED_PORT = 0xffff;
}

// NOTE:
// We want to assume that the port hasn't been set until we know otherwise.
// We also want to make sure that there's some kind of path, since HTTP
// requires a path.  The root path is the accepted default there.
URL::URL() : port(UNDEFINED_PORT),
  path("/") {
}

URL::~URL() {
  // Nothing to do...
}

// A few things to note about how the parsing is done:
//   - The protocol *must* be specified.  "http://example.org" will
//     parse. http:// will be the default protocl if not given
//   - If no port number is given in the URL, the returned URL object
//     will have the port clearly indicates as being undefined.
//   - If no path is given in the URL, it will be set to a forward slash
//     ("/"), to avoid having a blank std::string there.
URL* URL::parse(const std::string& urlString) {
  URL* newUrl = new URL();

  // Obtain the protocol from urlString
  size_t offset = newUrl->readProtocol(urlString);
  if (offset == std::string::npos) {
    delete newUrl;
    return NULL;
  }

  // Obtain the port from urlString
  offset = newUrl->readHostPort(urlString, offset);

  // if the offset has not yet read the end of the url std::string,
  // get the path
  if (offset < urlString.length()) {
    // Obtain the path
    newUrl->readPathDetails(urlString, offset);

    // If the client somehow input a URL with an empty path,
    // quietly save them from themselves.
    if (newUrl->path.length() == 0) {
      newUrl->path = "/";
    }
  }

  return newUrl;
}

const std::string& URL::getProtocol() const {
  return protocol;
}

const std::string& URL::getHost() const {
  return host;
}

bool URL::isPortDefined() const {
  return (port != UNDEFINED_PORT);
}

unsigned URL::getPort() const {
  return port;
}

const std::string& URL::getPath() const {
  return path;
}

const std::string& URL::getQuery() const {
  return query;
}

const std::string& URL::getFragment() const {
  return fragment;
}

void URL::print(std::ostream& out) {
  // Say the URL is http://www.example.org:8080/example.php?example#ex
  // Each piece follows.  Note that we should avoid printing optional
  // parts of the URL that have associated formatting characters, if
  // they aren't actually defined.

  // http://
  out << protocol << "://";

  // www.example.org
  out << host;

  // :8080 (if given)
  if (isPortDefined()) {
    out << ":" << port;
  }

  // /example.php
  out << path;

  // ?example(if given)
  if (query.length() > 0) {
    out << "?" << query;
  }

  // #ex (if given)
  if (fragment.length() > 0) {
    out << "#" << fragment;
  }
}

void URL::print(std::string& target) {
  // Much easier than duplicating the code.
  std::ostringstream targetOut;
  print(targetOut);
  target = targetOut.str();
}

size_t URL::readProtocol(const std::string& urlString, size_t offset) {
  size_t protocolEnd = urlString.find("://", offset);

  if (protocolEnd == std::string::npos) {
    // If protocol is not specified, assume protocol is HTTP
    protocol = "http";
    return 0;
  } else {
    protocol = urlString.substr(offset, protocolEnd - offset);
    return protocolEnd + 3;
  }
}

size_t URL::readHostPort(const std::string& urlString, size_t offset) {
  size_t partEnd = urlString.find_first_of("/#?", offset);
  if (partEnd == std::string::npos) {
    partEnd = urlString.length();
  }

  size_t portOffset = urlString.find(":", offset);
  if ((portOffset == std::string::npos) || (portOffset > partEnd)) {
    portOffset = partEnd;
  }

  host = urlString.substr(offset, portOffset - offset);
  if (portOffset < partEnd) {
    std::string portString(urlString.substr(portOffset + 1,
                           partEnd - portOffset - 1));
    std::istringstream iss(portString);  // create a istringstream
    iss >> port;  // get the converted unsigned short int
  }

  return partEnd;
}

size_t URL::readPathDetails(const std::string& urlString, size_t offset) {
  size_t unparsedEnd = urlString.length();

  // Once you hit the beginning of the fragment, that's the end of the
  // URL.  Since it's nice to know where our limits are, let's check for
  // that first.
  size_t fragmentOffset = urlString.find("#", offset);
  if (fragmentOffset != std::string::npos) {
    fragment = urlString.substr(fragmentOffset + 1);
    unparsedEnd = fragmentOffset;
  }

  size_t queryOffset = urlString.find("?", offset);
  if ((queryOffset != std::string::npos) && (queryOffset < unparsedEnd)) {
    query = urlString.substr(queryOffset + 1, unparsedEnd - queryOffset - 1);
    unparsedEnd = queryOffset;
  }

  path = urlString.substr(offset, unparsedEnd - offset);

  return urlString.length();
}

bool URL::isHtml(const std::string& path) {
  std::string end1 = "/";
  std::string end2 = "html";
  std::string end3 = "htm";

  if ((path.rfind(end1) + end1.size() == path.size()) ||
      (path.rfind(end2) + end2.size() == path.size()) ||
      (path.rfind(end3) + end3.size() == path.size())) {
    return true;
  } else {
    return false;
  }
}

void URL::setProtocol(const std::string& protocol) {
  this->protocol = protocol;
}

void URL::setHost(const std::string& host) {
  this->host = host;
}

void URL::clearPort() {
  port = UNDEFINED_PORT;
}

void URL::setPort(unsigned short port) {
  this->port = port;
}

void URL::setPath(const std::string& path) {
  this->path = path;
}

void URL::setQuery(const std::string& query) {
  this->query = query;
}

void URL::setFragment(const std::string& fragment) {
  this->fragment = fragment;
}
