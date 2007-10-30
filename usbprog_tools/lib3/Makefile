all:	demo_xml demo demo_http

demo: usbprog.o demo.cpp http_error_codes.o http_fetcher.o xmlParser.o
	g++ -g -o demo demo.cpp usbprog.o http_error_codes.o http_fetcher.o xmlParser.o -lusb

demo_xml: xmlparser
	g++ -g -o demo_xml demo_xml.cpp xmlParser.o

demo_http: http_fetcher.o http_error_codes.o demo_http.cpp
	g++ -o demo_http demo_http.cpp http_error_codes.o http_fetcher.o

xmlparser: xmlParser.h xmlParser.cpp
	g++ -c xmlParser.cpp

http_fetcher.o: http_fetcher.c http_fetcher.h
	g++ -c http_fetcher.c
	
http_error_codes.o: http_error_codes.c http_error_codes.h
	gcc -c http_error_codes.c

usbprog.o: usbprog.cpp usbprog.h
	g++ -g -c usbprog.cpp

clean:
	rm *.o demo_xml demo demo_http
