#include "HTTPMessage.h"
#include <algorithm>
#include <string>

HTTPMessage::HTTPMessage() {
  // Nothing to do...
}

HTTPMessage::~HTTPMessage() {
  // Nothing to do here, either...
  headers.clear();
}

unsigned HTTPMessage::getNumHeaderFields() const {
  return headers.size();
}

void
HTTPMessage::getHeaderSet(
    std::vector<std::pair<std::string, std::string> >& outSet) const {
  outSet.clear();

  for (std::map<std::string, std::string>::const_iterator it = headers.begin();
       it != headers.end(); it++) {  // iterate thourhg all headers
    std::pair<std::string, std::string> header(it->first, it->second);
    outSet.push_back(header);
  }
}

bool HTTPMessage::getHeaderValue(const std::string& name, std::string& outValue)
    const {
  std::map<std::string, std::string>::const_iterator it = headers.find(name);
  if (it != headers.end()) {  // found the header name
    outValue = it->second;  // get the header value
    return true;
  } else {
    return false;
  }
}

void HTTPMessage::setHeaderField(const std::pair<std::string, std::string>&
    headerPair) {
  setHeaderField(headerPair.first, headerPair.second);
}

void HTTPMessage::setHeaderField(const std::string& name,
    const std::string& value) {
  headers[name] = value;
}

bool HTTPMessage::parseFields(const char* data, unsigned length) {
  // Keep parsing fields until we run up against the end of the data
  // or we reach the end-of-lines marking the end of the headers.
  const char* dataEnd = data + length;
  bool foundEoh = false;  // found end-of-header CRLF

  while (data < dataEnd) {
    // Figure out where this header line ends.  Check if it's a
    // blank line (signifying the end of the headers), and make
    // sure it has an ending at all (if it doesn't, we haven't read
    // the complete header yet).
    const char* lineEnd = findNextLine(data, length);

    if (lineEnd == (data + lineEnding.length())) {  // lineEnding == "\r\n"
      foundEoh = true;
      break;
    } else if (lineEnd == NULL) {  // The current data does not have a complete
                                   // line to parse
      break;
    }
    // We won't be working with the EOL characters, so skip 'em.
    lineEnd -= lineEnding.length();

    // Figure out where the break between the header name and
    // value appears.
    const char* delimPos = data;
    for (; (delimPos < lineEnd) &&
      (*delimPos != headerDelimiter); delimPos++) {
    }
    // If it doesn't, we've got a bad header.
    if (delimPos >= lineEnd) {
      break;
    }

    // Grab out the name & value.  Trim any crud off the value
    // that we can.
    std::string name, value;
    name = std::string(data, static_cast<size_t>(delimPos - data));
    value = std::string(delimPos + 1,
        static_cast<size_t>(lineEnd - delimPos - 1));

    size_t ltrimPos = value.find_first_not_of(" \t\r\n");
    size_t rtrimPos = value.find_last_not_of(" \t\r\n");
    if (ltrimPos != std::string::npos) {
      value = std::string(value, ltrimPos, rtrimPos);
    } else {
      value = "";
    }

    setHeaderField(name, value);

    // Jump to the next line, for the next header.
    data = lineEnd + lineEnding.length();
  }

  return foundEoh;
}


void HTTPMessage::print(std::string& outputString) const {
  // Append the contents of our headers one-by-one.
  for (std::map<std::string, std::string>::const_iterator it = headers.begin();
      it != headers.end(); it++) {
    outputString += it->first;
    outputString += headerDelimiter;
    outputString += " ";
    outputString += it->second;
    outputString += lineEnding;
  }

  // Toss in a final line ending to signify the headers' end.
  outputString += lineEnding;
}


void HTTPMessage::print(char* outputBuffer, unsigned bufferLength) const {
  const char delimString[] = {headerDelimiter, ' ', '\0'};

  for (std::map<std::string, std::string>::const_iterator it = headers.begin();
      it != headers.end(); it++) {
    copyIfRoom(outputBuffer, it->first.c_str(), bufferLength);
    copyIfRoom(outputBuffer, delimString, bufferLength);
    copyIfRoom(outputBuffer, it->second.c_str(), bufferLength);
    copyIfRoom(outputBuffer, lineEnding.c_str(), bufferLength);
  }

  copyIfRoom(outputBuffer, lineEnding.c_str(), bufferLength);
}


void HTTPMessage::copyIfRoom(char*& outputBuffer,
    const char* dataString, unsigned& remainingLength) const {
  // Quit now if there's nothing at all that we can do.
  if (remainingLength == 0) {
    return;
  }

  // Figure out how much data we've got to copy, given the remaining
  // space.
  unsigned dataLength = strlen(dataString);
  if (dataLength > remainingLength) {
    dataLength = remainingLength;
  }

  // copy exactly that much.  Advance the buffer pointer accordingly.
  memcpy(outputBuffer, dataString, dataLength);
  remainingLength -= dataLength;
  outputBuffer += dataLength;

  // Be nice and null-terminate what we've written so far.
  *outputBuffer = '\0';
}

const char* HTTPMessage::findNextLine(const char* data, unsigned length)
    const {
  // Go character-by-character through the data until we either get past
  // the end or we get past a line-ending std::string.  Note that in the latter
  // case, we intentionally move a character past the line ending, so
  // the returned pointer will point to the *next* line.
  const char* dataEnd = data + length;
  unsigned endCharsFound = 0;
  while ((data < dataEnd) && (endCharsFound < lineEnding.length())) {
    if (*data == lineEnding[endCharsFound]) {
      endCharsFound++;
    } else {
      endCharsFound = 0;
    }

    data++;
  }

  // If we found the line end, great.  If not, boo.
  if (endCharsFound >= lineEnding.length()) {
    return data;
  } else {
    return NULL;
  }
}
