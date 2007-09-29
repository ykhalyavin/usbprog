import os, sys, traceback, time
import serial, threading

if not sys.argv[0].endswith('.exe'):
    sys.path.append("../pybsl")
    sys.path.append("../pyserjtag")
import bsl as bslmodule

import pyserjtag

DEBUG = 0

def main():
    global DEBUG
    import getopt
    comPort     = 0     #Default setting.
    speed       = 115200
    box         = pyserjtag.JTAGBox()

    sys.stderr.write("MSP430 Serial<->JTAG updater\n")
    try:
        opts, args = getopt.getopt(sys.argv[1:],
            "hc:DS:",
            ["help", "comport=", "debug", "speed="]
        )
    except getopt.GetoptError:
        # print help information and exit:
        usage()
        sys.exit(2)

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-c", "--comport"):
            try:
                comPort = int(a)                    #try to convert decimal
            except ValueError:
                comPort = a                         #take the string and let serial driver decide
        elif o in ("-D", "--debug"):
            DEBUG = DEBUG + 1
        elif o in ("-S", "--speed"):
            try:
                speed = int(a)                      #try to convert decimal
            except ValueError:
                sys.stderr.write("Speed must be decimal number\n")
                sys.exit(2)

    if len(args) != 1:
        raise ValueError("Wrong number of arguments, expexted firmare file.")

    if not os.path.exists(args[0]):
        raise ValueError("Firmware file does not exist.")
    
    bslmodule.DEBUG = DEBUG
    pyserjtag.DEBUG = DEBUG
    serialport = serial.Serial(comPort, speed, timeout=1)
    bsl = bslmodule.BootStrapLoader()
    bsl.serialport = serialport
    bsl.seqNo = 0
    bsl.reqNo = 0
    bsl.rxPtr = 0
    bsl.txPtr = 0

    
    #1st try BSL startup
    serialport.setBaudrate(9600)
    serialport.setParity(serial.PARITY_EVEN)
    try:
        bsl.actionStartBSL()
    except bslmodule.BSLException:
        sys.stderr.write("Could not start BSL with hardware handshake.\n")
        
        sys.stderr.write("Trying to start over firmware command...\n")
        #~ box.comInit(comPort, speed)                     #init port
        serialport.setBaudrate(speed)
        serialport.setParity(serial.PARITY_NONE)
        box.serial = serialport
        time.sleep(0.25)    #give time to load caps if powered form serial port
        box.serial.flushInput()
        box.stopthread = 0
        box.t = threading.Thread(target=box.collector_thread)
        box.t.setDaemon(1)
        box.t.start()
    
        #start BSL, then stop the receiver thread
        box.startBSL()
        box.stopthread = 1
        box.t.join()
    
    #prepare serial port for BSL
    serialport.setBaudrate(9600)
    serialport.setParity(serial.PARITY_EVEN)
    time.sleep(0.25)
    
    bsl.serialport.flushInput()
    bsl.serialport.flushOutput()
    
    #erase memory
    sys.stderr.write("Erasing Memory...\n")
    for i in range(bsl.meraseCycles):
        if i == 1: sys.stderr.write("Additional Mass Erase Cycles...\n")
        bsl.bslTxRx(bsl.BSL_MERAS,            #Command: Mass Erase
                            0xff00,             #Any address within flash memory.
                            0xa506)             #Required setting for mass erase!
    
    #Transmit password to get access to protected BSL functions.
    bsl.actionStartBSL(bslreset=0)
    
    #prepare data to download
    bsl.data = bslmodule.Memory()                   #prepare downloaded data
    bsl.data.loadFile(args[0])
    bsl.actionProgram()

    bsl.SetRSTpin(0)                        #disable power
    bsl.SetTESTpin(0)                       #disable power

    bsl.comDone()                           #Release serial communication port


if __name__ == '__main__':
    try:
        main()
    except SystemExit:
        raise               #let pass exit() calls
    except KeyboardInterrupt:
        if DEBUG: raise     #show full trace in debug mode
        sys.stderr.write("user abort.\n")   #short messy in user mode
        sys.exit(1)         #set errorlevel for script usage
    except Exception, msg:  #every Exception is caught and displayed
        if DEBUG: raise     #show full trace in debug mode
        sys.stderr.write("\nAn error occoured:\n%s\n" % msg) #short messy in user mode
        sys.exit(1)         #set errorlevel for script usage
