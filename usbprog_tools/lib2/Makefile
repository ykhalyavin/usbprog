all:	xmldemo demo http_fetcher.o http_error_codes.o

xmldemo: xmlparser
	g++ -o xmldemo xmldemo.cpp xmlParser.o

demo: xmlparser
	g++ -o demo demo.cpp xmlParser.o

xmlparser: xmlParser.h xmlParser.cpp
	g++ -c xmlParser.cpp

http_fetcher.o: http_fetcher.c http_fetcher.h
	gcc -c http_fetcher.c
	
http_error_codes.o: http_error_codes.c http_error_codes.h
	gcc -c http_error_codes.c

clean:
	rm *.o xmldemo demo
