all:	demo

demo:	xmlparser
	g++ -o demo demo.c xmlParser.o

xmlparser: xmlParser.h xmlParser.cpp
	g++ -c xmlParser.cpp
