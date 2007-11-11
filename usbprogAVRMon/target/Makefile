# Paths for various packages:

AVRTOOLSPATH = /usr/local/store/avr-1.0.0
GCCPATH = $(AVRTOOLSPATH)/bin
BINUTILPATH = $(AVRTOOLSPATH)/bin
LIBCPATH = $(AVRTOOLSPATH)/avr
LIBC_INC = $(LIBCPATH)/include
UISPPATH = /usr/local/bin

# The commands we'll need:

CC = $(GCCPATH)/avr-gcc
LD = $(GCCPATH)/avr-gcc
AS = $(BINUTILPATH)/avr-gcc
OBJCOPY = $(BINUTILPATH)/avr-objcopy
OBJDUMP = $(BINUTILPATH)/avr-objdump
SIZE = $(BINUTILPATH)/avr-size
UISP = $(UISPPATH)/uisp
RM = rm

# These architectures are tested, please enable exactly one of the
# following lines:

AVRARCH = at90s8515
#AVRARCH = at90s2313
#AVRARCH = at90s2323
# FIXME: not in ordnung yet

CFLAGS = -g -Wall -mmcu=$(AVRARCH) -O -I$(LIBC_INC)
ASFLAGS = -mmcu=$(AVRARCH) -I$(LIBC_INC)
LDFLAGS = -mmcu=$(AVRARCH)
LIBS = 

OBJS = \
	main.o \
	monitor.o

all: avrtest.srec

upload: avrtest.upload

info: avrtest.info

disasm: avrtest.disasm

size:
	$(SIZE) *.o *.elf

avrtest.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.srec : %.elf
	$(OBJCOPY) -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.upload: %.srec
	$(UISP) -dprog=stk200 --erase --upload if=$<

%.info : %.elf
	$(OBJDUMP) --headers $<

%.info : %.o
	$(OBJDUMP) --headers $<

%.disasm: %.elf
	$(OBJDUMP) -d -S $<

%.disasm: %.o
	$(OBJDUMP) -d -S $<

reset:
	$(UISP) -dprog=stk200

clean:
	$(RM) $(OBJS) *.elf *.srec *.bin *.o 

