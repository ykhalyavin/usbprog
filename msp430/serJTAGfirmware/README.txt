pySerJTAG
---------

This is the firmware for the Serial-JTAG adapter. It uses its own serial
protocol. (GDB compatible firmware is in developement too) It's compiled to
be run on a F13x or F14x MSP430 processor, F148/9 recomended for the cloning
function.

Hardware as proposed in the schematics, found on http://mspgcc.sf.net

Features
--------
- up- and download to RAM, peripherals and Flash
- erase Flash by segment, MAIN only or all
- execute at address
- stop the CPU at any time
- reset CPU (PUC)
- cloning function -> copy a user programm stored on the adapter to the
  target MSP. Cloning is started by pressing the start button.

Installation
------------

Close the BSL enable jumper and use pyBSL to download the firmware. It is
recomended that you open the jumper after programing to avoid accidental
deletation afterwards.

Usage
-----
The start button begins the "cloning" function. In this mode the Serial-JTAG
adapter can programm a target without connection to a PC (standalone
operation). The green LED is used to show the status of such an operation:

It flashes (total):
0x no user program found
1x user program found, but no JTAG connection to target possible
2x erasing the target flash failed
7x program and verify succeeded

Use the pySerJTAG software to download an user program. That software also
allows to use the adapter the same way pyBSL worked. This includes reading
and writing memory incl. Flash, erase and execute at address.

The red LED shows host CPU activity, thus its on when communicating over the
serial port and when using the cloning function.

Developer Notes
---------------

The firmware is located in the top 8k of the Flash. This leaves the rest of
the Flash free for user programs for the cloning function.

A 13x series MCU is choosen as target so that the HW multiplier is not used
and that not too much ram is used.
The resulting binary can be downloaded to any F13x and F14x CPU (min 8k
Flash required)

The data for the cloning function can be stored anywhere in the Flash, the
address is determined by the pyserjtag software. A pointer to that data
is stored at 0x1000 (INFO-Flash).
The data block consists of a word that selects the Flash erase mode. This
allows to erase the target's main memory only, e.g. to preserve callibration
data in the INFO-Flash. Then a word indicates the number of entries in the
following table table. The table is organized in segments. Each segment has
a start address and a size in words.

Information
-----------

Please read the docs in the pyserjtag package. That package contains the PC
side software.

This is not a finished software ;-) the features listed above are supported,
but the verify of the cloning function is not yet working correctly. The
jumpers are not yet used, selection of baudrate is planned. Yet more things
to come...

The firmware is/will be flexible enough to support 4, 6 and 8 MHz crystals
as well as F123 CPU (without cloning).

Pins for JTAG fuse blowing are reserved, but currently not used.

References
----------
 - TI MSP430 home: www.ti.com/msp430
 
 - TI application note:
      slaa149: "Programming a Flash-Based MSP430 Using the JTAG Interface"
      docs:    http://www-s.ti.com/psheets/slaa149/slaa149.pdf
      sources: http://www-s.ti.com/psheets/slaa149/slaa149.zip


chris <cliechti@gmx.net>
