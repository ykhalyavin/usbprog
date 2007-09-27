/* Spartan3 JTAG programmer

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




#include "ioparport.h"
#include "ioxusb.h"
#include "bitfile.h"
#include "jtag.h"
#include "devicedb.h"
#include "progalgxcf.h"
#include "progalgxc3s.h"

#define PPDEV "/dev/parport0"
#define DEVICEDB "devlist.txt"

void process(IOBase &io, BitFile &file, int chainpos);
void programXC3S(Jtag &jtag, IOBase &io, BitFile &file);
void programXCF(Jtag &jtag, IOBase &io, BitFile &file);

int main(int argc, char **args)
{
  // Produce release info from CVS tags
  char release[]={"$Name: Release-0-5-xup-0-0-2 $"};
  char *loc0=strchr(release,'-');
  if(loc0>0){
    loc0++;
    char *loc=loc0;
    do{
      loc=strchr(loc,'-');
      if(loc)*loc='.';
    }while(loc);
    release[strlen(release)-1]='\0'; // Strip off $
  }
  printf("Release %s\n",loc0);

  
  
  //IOParport io(PPDEV);
  IOXUSB io("");
  int chainpos=0;
  if(io.checkError()){
#if 0
    fprintf(stderr,"Could not access parallel device '%s'.\n",PPDEV);
    fprintf(stderr,"You may need to set permissions of '%s' ",PPDEV);
    fprintf(stderr,"by issuing the following command as root:\n\n# chmod 666 %s\n\n",PPDEV);
#else
    fprintf(stderr,"Could not access JTAG cable\n");
#endif
  }
  if(argc<=1){
    fprintf(stderr,"\nUsage: %s infile.bit [POS]\n\n",args[0]);
    fprintf(stderr,"\tPOS  position in JTAG chain, 0=closest to TDI (default)\n\n",args[0]);
    return 1;
  }
  if(argc>2)chainpos=atoi(args[2]);
  BitFile file;
  if(file.load(args[1]))process(io,file,chainpos);
  else return 1;
  return 0;
}

void process(IOBase &io, BitFile &file, int chainpos)
{
  Jtag jtag(&io);
  int num=jtag.getChain();

  // Synchronise database with chain of devices.
  DeviceDB db(DEVICEDB);
  for(int i=0; i<num; i++){
    int length=db.loadDevice(jtag.getDeviceID(i));
    if(length>0)jtag.setDeviceIRLength(i,length);
    else{
      unsigned id=jtag.getDeviceID(i);
      fprintf(stderr,"Cannot find device having IDCODE=%08x\n",id);
      return;
    }
  }
  

  if(jtag.selectDevice(chainpos)<0){
    fprintf(stderr,"Invalid chain position %d, position must be less than %d (but not less than 0).\n",chainpos,num);
    return;
  }

  // Find the programming algorithm required for device
  const char *dd=db.getDeviceDescription(chainpos);
  if(strncmp("XC3S",dd,4)==0) programXC3S(jtag,io,file);
  else if(strncmp("XCF",dd,3)==0) programXCF(jtag,io,file);
  else{
    fprintf(stderr,"Sorry, cannot program '%s', a later release may be able to.\n",dd);
    return;
  }
}

void programXC3S(Jtag &jtag, IOBase &io, BitFile &file)
{

  ProgAlgXC3S alg(jtag,io);
  alg.program(file);
  return;
}

void programXCF(Jtag &jtag, IOBase &io, BitFile &file)
{
  ProgAlgXCF alg(jtag,io);
  alg.erase();
  alg.program(file);
  return;
}
