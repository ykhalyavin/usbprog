/*
An API for the logicdevice
Copyright (C) 2006 Benedikt Sauter <sauter@ixbat.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>

#include "logic.h"


Logic* openLogic()
{
  unsigned char located = 0;
  struct usb_bus *bus;
  struct usb_device *dev;

  Logic *tmp = (Logic*)malloc(sizeof(Logic));
  tmp->logic_handle=0;
  //usb_set_debug(2);
 		
  usb_init();
  usb_find_busses();
  usb_find_devices();
 
  for (bus = usb_busses; bus; bus = bus->next)
  {
    for (dev = bus->devices; dev; dev = dev->next)	
    {
      if (dev->descriptor.idVendor == 0x0400) 
      {	
	located++;
	tmp->logic_handle = usb_open(dev);
      }
    }	
  }

  if (tmp->logic_handle==0) return (0);
  else 
  {
    printf("found\n");
    usb_set_configuration(tmp->logic_handle,1);
    usb_claim_interface(tmp->logic_handle,0);
    usb_set_altinterface(tmp->logic_handle,0);
  
    return (tmp);
  }
}


   

int closeLogic(Logic* self)
{
  usb_close(self->logic_handle);	
}


int sendLogicCommand(Logic* self,char *command)
{
  return usb_bulk_write(self->logic_handle,2,command,(int)command[1],1000);
}

int readLogicData(Logic* self, char* data, int length)
{
  int i,j,dataindex=0;
  char tmp[length];
  char command[2] = {CMD_GETDATA,2};

  while(1) 
  {
    sendLogicCommand(self,command);
    i = usb_bulk_read(self->logic_handle,2,tmp,length,100);	
    for(j=0;((j<i) && (dataindex<length));j++)
    {
      data[dataindex]=tmp[j];
      dataindex++;
    }
    if(dataindex >= (length-1))
      break;
  }
  return dataindex;
}

int readLogicResults(Logic* self,char *data)
{
	
}

void SetLogicSampleRate(Logic* self,char samplerate)
{
  char command[3] = {CMD_SETSAMPLERATE,3,samplerate};
  sendLogicCommand(self,command);
}


void SetLogicMode(Logic* self,char state)
{
  char command[3] = {CMD_SETMODE,3,state};
  sendLogicCommand(self,command);
}

void StartLogic(Logic* self)
{
  char command[2] = {CMD_STARTSCOPE,2};
  sendLogicCommand(self,command);
}

void StopLogic(Logic* self)
{
  char command[2] = {CMD_STOPSCOPE,2};
  sendLogicCommand(self,command);
}

int GetLogicState(Logic* self)
{
  char command[2] = {CMD_GETSCOPESTATE,2};
  sendLogicCommand(self,command);
}

int GetLogicMode(Logic* self)
{
  char command[2] = {CMD_GETSCOPEMODE,2};
  sendLogicCommand(self,command);
}


int GetLogicFIFOLoad(Logic* self)
{
  char command[2] = {CMD_GETFIFOLOAD,2};
  sendLogicCommand(self,command);
}


void Recording(Logic* self,char samplerate,int numbers,char* data)
{
  SetLogicMode(self,MODE_LOGIC);
  SetLogicSampleRate(self,samplerate);
  StartLogic(self);
  readLogicData(self, data, numbers);
  StopLogic(self);
}


void RecordingInternal(Logic* self,char samplerate)
{
  SetLogicMode(self,MODE_LOGICINTERN);
  SetLogicSampleRate(self,samplerate);
  StartLogic(self);
  // TODO check here with an endless loop and  GetLogicState if record is ready
  sleep(1);
  return;
  while(GetLogicState(self) !=STATE_DONOTHING)
  {
    sleep(1);
  }
}

void GetRecordInternal(Logic* self,char*data,int lengths)
{
  if(lengths>1000)
    lengths=1000;
  readLogicData(self, data, lengths);
}


void ActivateEdgeTrigger(Logic* self,int channel,int value)
{
  char command[4] = {CMD_SETEDGETRIG,4,channel,value};
  sendLogicCommand(self,command);
}

void ActivatePatternTrigger(Logic* self,char pattern,char ignore)
{
  char command[4] = {CMD_SETPATTRIG,4,pattern,ignore};
  sendLogicCommand(self,command);
}

void DeActivateTrigger(Logic* self)
{
  char command[2] = {CMD_DEACTIVTRIG,2};
  sendLogicCommand(self,command);
}

