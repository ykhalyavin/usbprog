all: handbuch

handbuch: usbprog_handbuch.asciidoc
	asciidoc -a lang=de -d book -b docbook usbprog_handbuch.asciidoc 
	docbook2pdf usbprog_handbuch.xml
	xpdf usbprog_handbuch.pdf
