/* XCF Flash PROM JTAG programming algorithms

Copyright (C) 2004 Andrew Rogers

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */




#include "progalgxcf.h"

const byte ProgAlgXCF::SERASE=0x0a;
const byte ProgAlgXCF::ISPEN=0xe8;
const byte ProgAlgXCF::FPGM=0xea;
const byte ProgAlgXCF::FADDR=0xeb;
const byte ProgAlgXCF::FERASE=0xec;
const byte ProgAlgXCF::FDATA0=0xed;
const byte ProgAlgXCF::FVFY0=0xef;
const byte ProgAlgXCF::NORMRST=0xf0;
const byte ProgAlgXCF::IDCODE=0xfe;
const byte ProgAlgXCF::BYPASS=0xff;

const byte ProgAlgXCF::BIT3=0x08;
const byte ProgAlgXCF::BIT4=0x10;

ProgAlgXCF::ProgAlgXCF(Jtag &j, IOBase &i)
{
  jtag=&j;
  io=&i;
}

int ProgAlgXCF::erase()
{
  byte data[4];
  jtag->shiftIR(&NORMRST);
  io->cycleTCK(40000);
  byte ircap[1];
  jtag->shiftIR(&BYPASS,ircap);
  if((ircap[0]&BIT3)==BIT3){
    fprintf(stderr,"Device is write protected.\n",ircap[0]);
    return 1;
  }
  jtag->shiftIR(&ISPEN);
  jtag->shiftIR(&FADDR);
  jtag->longToByteArray(1,data);
  jtag->shiftDR(data,0,16);
  io->cycleTCK(2);
  
  printf("Erasing...."); fflush(stdout);
  jtag->shiftIR(&FERASE);
  io->cycleTCK(2400000);
  printf("done.\n");

  jtag->shiftIR(&BYPASS);
  io->tapTestLogicReset();
 
  return 0;
}

int ProgAlgXCF::program(BitFile &file)
{
  jtag->shiftIR(&NORMRST);
  io->cycleTCK(40000);
  io->setTapState(IOBase::TEST_LOGIC_RESET);
  byte data[4];
  jtag->shiftIR(&ISPEN);
  data[0]=0x34;
  jtag->shiftDR(data,0,6);

  for(int i=0; i<file.getLength(); i+=4096){
    int frame=i/128;
    printf("Programming frames 0x%04x to 0x%04x....",frame,frame+31); fflush(stdout);
    jtag->shiftIR(&FDATA0);
    if((i+4096)<=file.getLength()){
      jtag->shiftDR(&(file.getData())[i/8],0,4096);
    }
    else{
      int rem=(file.getLength()-i)/8; // Bytes remaining
      int pad=512-rem;
      byte paddata[pad]; for(int k=0; k<pad; k++)paddata[k]=0xff;
      jtag->shiftDR(&(file.getData())[i/8],0,rem*8,0,false); // Do not goto EXIT1-DR
      jtag->shiftDR(paddata,0,pad*8);
    }
    jtag->longToByteArray(frame,data);
    jtag->shiftIR(&FADDR);
    jtag->shiftDR(data,0,16);
    io->cycleTCK(2);
    jtag->shiftIR(&FPGM);
    io->cycleTCK(5000);
    printf("done.\n");
  } 

  jtag->shiftIR(&BYPASS);
  io->tapTestLogicReset();
  return 0;
}

int ProgAlgXCF::verify(BitFile &file)
{
  return 0;
}
