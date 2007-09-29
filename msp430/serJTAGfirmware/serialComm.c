/**
Serial communications module.
part of serJTAGfirmware
http://mspgcc.sf.net
chris <cliechti@gmx.net>
*/

#include <string.h>
#include "defs.h"
#include "hardware.h"
#include "serialComm.h"
#include "taskhandler.h"
#include "tasklist.h"
#include "asmlib.h"
#include "JTAGfunc.h"
#include "LowLevelFunc.h"
#include "flash.h"

#define UART_TIMEOUT_MILLISEC 30

volatile word serRxIndex = 0;        //receive position

BUFFER serTxBuf;
BUFFER serRxBuf;

/**
send a single character over the UART, first wait until transmit buffer is empty
*/
int putchar(int c) {
  while((UTCTL0 & TXEPT) == 0) {}       //wait until tx buffer free
  TXBUF0 = c;                           //transmit character
  return 1;
}

/**
Support function for calcChecksum(), Not for direct use!.

This function updates a checksum adding a single byte. A CRC algorithm is used.
Before first call crc_calc has to be zero. Consecutive calls use
the last return value from "crc" for crc_calc. e.g.:
  crc_clac = 0;
  crc_clac = crc(crc_clac, data);


And for consecutive calls:
  crc_clac = crc(crc_clac, data2);
*/
int crc(byte crc_calc, byte bt_to_crc) {
    byte al = bt_to_crc;
    byte c; //Boolean value
    byte bl;
    for (bl = 8; bl > 0; bl--) {
        al ^= crc_calc;
        c = (al & 0x01) != 0;
        al = crc_calc;
        if (c) {
            al ^= 0x18;
        }
        al >>= 1;
        if (c) {
            al |= 0x80;
        }
        crc_calc = al;
        al = bt_to_crc;
        al >>= 1;
        bt_to_crc = al;
    }
    return crc_calc;
}

/**
Handle incomming frames from the serial port.
*/
void rxhandler(void) {
    switch (serRxBuf.header.type) {
        case ACK:
        case NACK:
            break;

        case T_CONNECT:                         //connect JTAG to the target
            InitTarget();
            if (GetDevice() == STATUS_OK) {
                txACK();
                //printf("Device type: %04x\n", DEVICE);
            } else{
                txNACKstr("no target");
            }
            break;
        case T_RELEASE:                         //release JTAG from target
            ClrTCLK();
            IR_Shift(IR_CNTRL_SIG_16BIT);
            DR_Shift16(0x3001);         //lch
            IR_Shift(IR_CNTRL_SIG_RELEASE);
            SetTCLK();
            ReleaseTarget();
            txACK();
            break;
        case T_RESET:                           //reset the target and release JTAG
            ReleaseDevice(V_RESET);
            ReleaseTarget();
            txACK();
            break;
        case T_PUC:                             //reset the target through JTAG, keep connection
            ExecutePUC();
            txACK();
            break;

        case T_MEMREAD:                         //ream memory from target
            {                                   //use a new block for local vars
                word size = serRxBuf.data.memread.size;
                word address = serRxBuf.data.memread.address;
                word offset;
    
                txACK();
                HaltCPU();
                if (size > sizeof(serTxBuf.data.memdata.data)) {
                   size = sizeof(serTxBuf.data.memdata.data);
                }
                if (address <= 0xff) {          //peripherals in bytemode
                    for (offset = 0; offset < size; offset++) {
                        serTxBuf.data.memdata.data[offset] = ReadMem(F_BYTE, address+offset);
                    }
                } else {                        //peripherals in wordmode as well as RAM and Flash
                    address = address & 0xfffe; //startaddress has to be even!
                    for (offset = 0; offset < size/2; offset++) {
                        ((word *)serTxBuf.data.memdata.data)[offset] = ReadMem(F_WORD, address+offset*2);
                    }
                    if (size & 1) {
                        //odd size, read last byte separate
                        //odd sizes on word-mode peripherals yield  wrong results
                        //in the last word!!
                        serTxBuf.data.memdata.data[size-1] = ReadMem(F_BYTE, address+size-1);
                    }
                }
                serTxBuf.data.memdata.address = address;
                serTxBuf.header.type = T_MEMDATA;
                serTxBuf.header.size = 2+size;
                sendMessage(&serTxBuf);
            }
            break;
        case T_MEMWRITE:                        //write target memory (Peripherals, RAM, Flash)
            {
                word offset;
                word size = serRxBuf.header.size - 2;
                HaltCPU();
                if ((serRxBuf.data.memwrite.address >= 0x100) &&
                    (serRxBuf.data.memwrite.address <= 0x1ff)) { //peripherals in wordmode
                    for (offset = 0; offset < size; offset+=2) {
                        WriteMem(F_WORD,
                                 serRxBuf.data.memwrite.address+offset,
                                 ((word*)serRxBuf.data.memwrite.data)[offset/2]
                        );
                    }
                } else if (serRxBuf.data.memwrite.address < 0x1000) {  //ram + peripherals, bytemode
                    for (offset = 0; offset < size; offset++) {
                        WriteMem(F_BYTE,
                                 serRxBuf.data.memwrite.address+offset,
                                 serRxBuf.data.memwrite.data[offset]
                        );
                    }
                } else {                        //flash memory, wordmode
                    WriteFLASH(serRxBuf.data.memwrite.address,
                               size/2,
                               (word *)serRxBuf.data.memwrite.data
                    );
                }
                txACK();
            }
            break;
        case T_EXEC:                            //execute target program located at given address
            ReleaseDevice(serRxBuf.data.exec.address);
            txACK();
            break;
        case T_MEMERASE:                        //erase target flash (Segment, Main or All)
            HaltCPU();
            EraseFLASH(
                serRxBuf.data.memerase.mode,
                serRxBuf.data.memerase.address
            );
            txACK();
            break;
        case T_MCLK:                            //provide MCLKs, allows sort of single stepping
            ClrTCLK();
            IR_Shift(IR_CNTRL_SIG_16BIT);
            DR_Shift16(0x3401);         //lch
            while (serRxBuf.data.step.numsteps--) {
                SetTCLK();
                ClrTCLK();
            }
            SetTCLK();
            txACK();
            break;

        case MEMREAD:                           //host memory read
            {
                word size = serRxBuf.data.memread.size;
                byte *address = (byte*)serRxBuf.data.memread.address;
                word offset;
                txACK();
                if (size > sizeof(serTxBuf.data.memdata.data)) {
                   size = sizeof(serTxBuf.data.memdata.data);
                }
                if (address <= (byte *)0xff) {      //bytemode
                    for (offset = 0; offset < size; offset++) {
                        serTxBuf.data.memdata.data[offset] = address[offset];
                    }
                } else {                            //wordmode
                    address = (byte *)((word)address & 0xfffe);    //startaddress has to be even!
                    size &= 0xfffe;                     //size has to be even
                    for (offset = 0; offset < size/2; offset++) {
                        ((word *)serTxBuf.data.memdata.data)[offset] = ((word *)address)[offset];
                    }
                }
                serTxBuf.data.memdata.address = (word)address;
                serTxBuf.header.type = MEMDATA;
                serTxBuf.header.size = 2+size;
                sendMessage(&serTxBuf);
            }
            break;
        case MEMWRITE:                          //host memory write, used to download user programs
            {
                void *adr = (byte *)serRxBuf.data.memwrite.address;
                word offset;
                word size = serRxBuf.header.size - 2;
                if ((adr >= (void *)0x100) && (adr <= (void *)0x1ff)) { //peripherals, wordmode
                    for (offset = 0; offset < size/2; offset++) {
                        ((word *)adr)[offset] = ((word *)serRxBuf.data.memwrite.data)[offset];
                    }
                    txACK();
                    return;
                }
                if (adr < (void *)0x1000) {     //ram + peripherals, bytemode
                    for (offset = 0; offset < size; offset++) {
                        ((byte *)adr)[offset] = serRxBuf.data.memwrite.data[offset];
                    }
                } else {                        //flash
                    flashWriteBlock(
                        serRxBuf.data.memwrite.data,
                        (void *)serRxBuf.data.memwrite.address,
                        size);
                }
                txACK();
            }
            break;
        case MEMERASE:                          //erase one segemnt on host
            flashErase((void *)serRxBuf.data.memerase.address);
            txACK();
            break;
        //~ case MEMEXEC:                       //exec code on host
                //~ serRxBuf.memexec.function(
                    //~ serRxBuf.memexec.arg1,
                    //~ serRxBuf.memexec.arg2
                //~ );
            //~ txACK();
            //~ break;
        case STARTBSL:                          //start BSL for firmware upgrade
            txACK();
            while((UTCTL0 & TXEPT) == 0) {}     //wait until last byte is sent
            FCTL3 = 0;                          //generate a flash key violation -> POR reset, see main()
            break;
        default:
            txNACKstr("unknown command");       //serialComm error: unknown command
    }
}

/**
Handler for serial data (called in RX interrupt from UART).

return true if the buffer contains a complete message.

serRxIndex is reset on idle line (after a timeout).
*/
interrupt(UART0RX_VECTOR) rx0_isr() {
    static word last_sysclk=0;                  // Timeout variable
    if ((word)(sysclk-last_sysclk) > UART_TIMEOUT_MILLISEC) {
      serRxIndex = 0;                           // Timeout !!
    }
    last_sysclk = sysclk;

    if (serRxIndex < SERBUFSIZE) {              //buffer check
        ((byte *)&serRxBuf)[serRxIndex++] = RXBUF0;
    } else {
        txNACKstr("buffer full");
    }
    //is a complete frame received? Only check size of data if header is completely received
    if (serRxIndex >= (sizeof(FRAME_HEAD)+sizeof(FRAME_TAIL))) {
        if (serRxIndex == (serRxBuf.header.size + (sizeof(FRAME_HEAD)+sizeof(FRAME_TAIL)))) {
            //start serial handler task on complete frame
            taskreg |= TASK_serTask;
            _BIC_SR_IRQ(LPM4_bits);
        }
    }
}

/**
Task which handles incomming packets from the serial port.
Its started from the serial receiver interrupt.
*/
void serTask(void) {
    word i;
    byte serCRC;
    word crcpos;

    P6OUT |= LEDGN;                     //switch LED on as activity indicator
    AutoPowerOffDisable();              //disable auto power off timer

    crcpos = serRxBuf.header.size + sizeof(FRAME_HEAD);
    //calculate checksum of frame and compare it to the received one, which is the last rec. byte
    //calc CRC  for header and data, but not the crc itself
    for (i = 0, serCRC=0; i<crcpos; i++) {
      //printf("x: %02x", ((unsigned char *)&serRxBuf)[i]);
        serCRC = crc(serCRC, ((byte *)&serRxBuf)[i]);
    }
    if (serCRC == ((byte *)&serRxBuf)[crcpos]) {
        rxhandler();                    //Complete frame received
    } else {
      //printf("len: %d crc: %02x buf: %02x", crcpos, serCRC, ((byte *)&serRxBuf)[crcpos]);
        txNACKstr("CRC");               //Errorous frame received
    }
    AutoPowerOffReset();                //reset auto power off timer
    P6OUT &= ~LEDGN;                    //switch LED off
}


/**
Send a serialComm packet. This function blocks until everything is sent.
*/
void sendMessage(BUFFER *buffer) {
    word i;
    byte serCRC = 0;
    word len = buffer->header.size + sizeof(FRAME_HEAD);
    //calc CRC  for header and data, but not the crc itself
    for (i = 0; i<len; i++) {
        serCRC = crc(serCRC, ((byte *)buffer)[i]);
    }
    ((byte *)buffer)[len] = serCRC;
    
    //send message
    for( i = len + sizeof(FRAME_TAIL); i; i--) {
        putchar(*((byte *)buffer)++);
    }
}

/**
Send an aknowledge ("OK").
attetion this resets the input buffer!
*/
void txACK(void) {
    serRxIndex = 0; //clear buffer
    serTxBuf.header.type = ACK;
    serTxBuf.header.size = 0;
    sendMessage(&serTxBuf);
}

/**
Send an error. The message "smg" is transmited as reason for the other side.
attetion this resets the input buffer!
*/
void txNACKstr(byte *msg) {
    serRxIndex = 0; //clear buffer
    strncpy(serTxBuf.data.debugtext, msg, sizeof(serTxBuf.data.debugtext));
    serTxBuf.header.type = NACK;
    serTxBuf.header.size = strlen(msg);
    sendMessage(&serTxBuf);
}

/**
Send a debug message.
*/
void txDebug(byte *msg) {
    strncpy(serTxBuf.data.debugtext, msg, sizeof(serTxBuf.data.debugtext));
    serTxBuf.header.type = DEBUGTEXT;
    serTxBuf.header.size = strlen(msg);
    sendMessage(&serTxBuf);
}
