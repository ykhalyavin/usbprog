#ifndef SERIALCOMM_H
#define SERIALCOMM_H

//Definitions and function prototypes for the Serial communications module.
//
//see frames.txt for further documentation
//
//part of serJTAGfirmware
//http://mspgcc.sf.net
//chris <cliechti@gmx.net>

#include "defs.h"
#include <stdio.h>

//redirect stdio looking funcs to serial port
#define printf(args...) {\
    snprintf(serTxBuf.data.debugtext, sizeof(serTxBuf.data.debugtext), args);\
    serTxBuf.header.type = DEBUGTEXT;\
    serTxBuf.header.size = strlen(serTxBuf.data.debugtext);\
    sendMessage(&serTxBuf);\
}

/** size for serial rx and tx buffers*/
#define SERBUFSIZE 70

/** Header of a serial frame */
typedef struct {
        byte type;
        byte size;
} FRAME_HEAD;

/** Tail of a serial frame (currently the checksum)*/
typedef byte FRAME_TAIL;

//------------------------------------------------------------------------
//Frame data types
//------------------------------------------------------------------------
//serial frame IDs
#define ACK         0x00  //PC <-> BOX   Acknowledege for a message
#define NACK        0xff  //PC <-> BOX   Not-Acknowledege. Message generated an error

#define T_CONNECT   0x01  //PC --> BOX   Connect to target, this stops it
#define T_RELEASE   0x02  //PC --> BOX   Release JTAG connection, target continues
#define T_RESET     0x03  //PC --> BOX   Reset target, release JTAG
#define T_PUC       0x04  //PC --> BOX   Perform a PUC on target, keep conection
#define T_EXEC      0x05  //PC --> BOX   Start target execution at address, release JTAG
#define T_MCLK      0x06  //PC --> BOX   Single step target

#define T_MEMREAD   0x11  //PC --> BOX   Request memory range from target
#define T_MEMDATA   0x12  //PC <-- BOX   Answer to MEMREAD from target
#define T_MEMWRITE  0x13  //PC --> BOX   Write data to specified address to target
#define T_MEMERASE  0x14  //PC --> BOX   Erase Flash segment on target containing the address
                        
#define DEBUGTEXT   0xe0  //PC <-- BOX   Debug messages from BOX
#define MEMREAD     0xe1  //PC --> BOX   Request memory range from the BOX
#define MEMDATA     0xe2  //PC <-- BOX   Answer to MEMREAD from the BOX
#define MEMWRITE    0xe3  //PC --> BOX   Write data to specified address on the BOX
#define MEMERASE    0xe4  //PC --> BOX   Erase Flash segment on the BOX containing the address

#define STARTBSL    0xf0  //PC --> BOX   Start BSL for firmware upgrade

typedef struct {
    word address;
} FRAME_T_EXEC;

typedef struct {
    word numsteps;
} FRAME_T_STEP;

typedef struct {
    word address;
    word size;
} FRAME_T_MEMREAD;

typedef struct {
    word address;
    byte data[64];
} FRAME_T_MEMDATA;

typedef struct {
    word address;
    byte data[64];
} FRAME_T_MEMWRITE;

typedef struct {
    word address;
    word mode;
} FRAME_T_MEMERASE;

//serial rx buffer definition for frame and data (byte) access
typedef struct {
    FRAME_HEAD header;
    union {
        FRAME_T_EXEC     exec;              //these fields are used for the T_ and for the non T_ types 
        FRAME_T_STEP     step;
        FRAME_T_MEMREAD  memread;
        FRAME_T_MEMDATA  memdata;
        FRAME_T_MEMWRITE memwrite;
        FRAME_T_MEMERASE memerase;
        byte debugtext[SERBUFSIZE-(sizeof(FRAME_HEAD)+sizeof(FRAME_TAIL))];  //debug messages
    } data;
    FRAME_TAIL tail;                        //only to reserve space!!
} BUFFER;

//------------------------------------------------------------------------
//Protos
//------------------------------------------------------------------------
extern void txACK(void);
extern void txNACKstr(byte *msg);
extern void sendMessage(BUFFER * buf);
extern void txDebug(byte *msg);
extern word procchar(void);
    
extern BUFFER serRxBuf;
extern BUFFER serTxBuf;

#endif //SERIALCOMM_H
