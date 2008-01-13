all: handbuch

handbuch: usbprog_handbuch.asciidoc
	#asciidoc -a lang=german -d book -b docbook usbprog_handbuch.asciidoc 
	a2x -f pdf -d book -a lang=de usbprog_handbuch.asciidoc
	#fop usbprog_handbuch.fo usbprog_handbuch.pdf
	#docbook2pdf usbprog_handbuch.xml
	#xpdf usbprog_handbuch.pdf

html: usbprog_handbuch.html

usbprog_handbuch.html: usbprog_handbuch.asciidoc
	asciidoc -a toc -a lang=de -n $<
