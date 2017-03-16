# CSE 422 Lab 2 makefile
# Modified from CSE422 FS12

# Options to set when compiling/linking the project.
CXXFLAGS=-g
LDFLAGS=-pthread

# The name of the executable to generate.
TARGET=client proxy


# The objects that should be compiled from the project source files (expected
# to correspond to actual source files, e.g. URL.o will come from URL.cc).
#
# You will want to add the name of your driver object to this list.
client_OBJS=HTTPMessage.o \
	HTTPRequest.o \
	HTTPResponse.o \
	TCPSocket.o\
	URL.o \
	client.o

proxy_OBJS=HTTPMessage.o \
	HTTPRequest.o \
	HTTPResponse.o \
	TCPSocket.o\
	URL.o \
	ProxyWorker.o \
	proxy.o

# Have everything built automatically based on the above settings.
all: $(TARGET)

.cc.o:
	g++ -o $@ $(CXXFLAGS) -c $< -DBUFFER_SIZE=40960

client: $(client_OBJS)
	g++ -o $@ $^ $(LDFLAGS)

proxy: $(proxy_OBJS)
	g++ -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TARGET) $(proxy_OBJS) $(client_OBJS)
	$(RM) -rf Downloads


# Dependencies follow (i.e. which source files and headers a given object is
# built from).
TCPSocket.o: TCPSocket.h TCPSocket.cc
URL.o: URL.cc URL.h
HTTPMessage.o: HTTPMessage.cc HTTPMessage.h
HTTPRequest.o: HTTPRequest.cc HTTPRequest.h HTTPMessage.h URL.o TCPSocket.o
HTTPResponse.o: HTTPResponse.cc HTTPResponse.h HTTPMessage.h URL.o TCPSocket.o
client.o: client.cc client.h HTTPRequest.o HTTPResponse.o
ProxyWorker.o: ProxyWorker.cc ProxyWorker.h HTTPRequest.o HTTPResponse.o URL.o
proxy.o: proxy.cc ProxyWorker.o
