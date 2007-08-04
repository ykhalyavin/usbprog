all:	xmldemo demo

xmldemo: xmlparser
	g++ -o xmldemo xmldemo.cpp xmlParser.o

demo: xmlparser
	g++ -o demo demo.cpp xmlParser.o

xmlparser: xmlParser.h xmlParser.cpp
	g++ -c xmlParser.cpp

clean:
	rm *.o xmldemo demo
