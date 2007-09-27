/* JTAG chain detection

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
#include "iodebug.h"
#include "jtag.h"
#include "devicedb.h"

#define PPDEV "/dev/parport0"
#define DEVICEDB "devlist.txt" 

int main(int argc, char **args)
{
//  IOParport io(PPDEV);
  IOXUSB io("");
  
  if(io.checkError()){
    fprintf(stderr,"Could not access parallel device '%s'.\n",PPDEV);
    fprintf(stderr,"You may need to set permissions of '%s' ",PPDEV);
    fprintf(stderr,"by issuing the following command as root:\n\n# chmod 666 %s\n\n",PPDEV);
    return 1;
  }

  Jtag jtag(&io);
  int num=jtag.getChain();
  DeviceDB db(DEVICEDB);
  int dblast=0;
  for(int i=0; i<num; i++){
    unsigned long id=jtag.getDeviceID(i);
    int length=db.loadDevice(id);
    printf("IDCODE: 0x%08x\t",id);
    if(length>0){
      jtag.setDeviceIRLength(i,length);
      printf("Desc: %s\tIR length: %d\n",db.getDeviceDescription(dblast),length);
      dblast++;
    } 
    else{
      printf("not found in '%s'.\n",DEVICEDB);
    }
  }
  return 0;
}
