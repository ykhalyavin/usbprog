#makefile for firmaware for the Serial-JTAG adapter
#
#part of serJTAGfirmware
#http://mspgcc.sf.net
#chris <cliechti@gmx.net>

NAME    = firmware
CPU     = msp430x149

ASFLAGS = -mmcu=${CPU} -x assembler-with-cpp -D_GNU_ASSEMBLER_ -c
CFLAGS  = -mmcu=${CPU} -O2 -Wall -g
CC      = msp430-gcc
AS      = msp430-gcc
LD      = msp430-gcc
BSL     = msp430-bsl

CFILES = main.c JTAGfunc.c LowLevelFunc.c serialComm.c
AFILES = flash.S asmlib.S taskhandler.S
OFILES = $(CFILES:.c=.o) $(AFILES:.S=.o)

.PHONY: all clean dist makefile.dep

all: ${NAME}.lst ${NAME}.a43

#geneate an intel hex for download
#this is also the default target "all" (1st entry)
${NAME}.a43:    ${NAME}.elf
	msp430-objcopy -O ihex $< $@

#linking the application, geneate a listing
#note the extra options to fill only the top 8K flash.
${NAME}.elf:    $(OFILES)
	msp430-gcc -mmcu=${CPU} -Wl,-Ttext -Wl,0xe000 -o $@ $^ -lm

${NAME}.lst:    ${NAME}.elf
	msp430-objdump -dSt $< >$@
	msp430-size $^ >>$@

#clean files generated during compilation
clean:
	rm -f ${NAME}.elf ${NAME}.a43 ${NAME}.lst ${NAME}.ini ${OFILES}

#use pybsl to download the firmware
download: all
	${BSL} --speed=38400 -e ${NAME}.elf

.PHONY: all clean dist

#make an archive, only the binary and readme.
serjtag.zip: ${NAME}.a43 README.txt LICENSE.txt
	zip -9 serjtag.zip $^

#prepare a binary distribution
dist: serjtag.zip

#automatic collection of dependencies in the source files.
#it's only updated the first time, after that it must be done maually
#with "make depend"
#the dependecies are included from a separate file:
-include dependencies.in
#target to update the file, it's removed first
depend: rmdepend dependencies.in
#remove the file
rmdepend:
	rm -f dependencies.in
#build the file that contains the dependencies. no deps in this rule.
#if there were deps it would be rebuilt every chnage, which is unneded:
dependencies.in:
	${CC} -MM ${CFLAGS} $(CFILES) $(AFILES) >$@

