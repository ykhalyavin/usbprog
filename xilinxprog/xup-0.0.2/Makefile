all:
	sdcc -mmcs51 fw.c
	mkdir -p /usr/share/xup
	cp -af fw.ihx /usr/share/xup
	chown root:root /usr/share/xup/fw.ihx
	chmod a+r /usr/share/xup/fw.ihx
	gcc -Wall -O2 -g -o p main.c lenval.c micro.c ports.c -lusb

clean:
	rm -rf p
