#ifndef DEFS_H
#define DEFS_H

//Typedefs and other definitions
//
//part of serJTAGfirmware
//http://mspgcc.sf.net
//chris <cliechti@gmx.net>

#define __BYTEWORD__
typedef unsigned int  word;
typedef unsigned char byte;

void AutoPowerOffDisable(void);
void AutoPowerOffReset(void);

#endif //DEFS_H
