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


Logic *openLogic()
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
      if (dev->descriptor.idVendor == 0x0400) // 0x0400 changed to new VID
      {	
	located++;
	tmp->logic_handle = usb_open(dev);
      }
    }	
  }

  if (tmp->logic_handle==0) return (0);
  else 
  {
    usb_set_configuration(tmp->logic_handle,1);
    usb_claim_interface(tmp->logic_handle,0);
    usb_set_altinterface(tmp->logic_handle,0);
  
    return (tmp);
  }
}


   

void closeLogic(Logic* self)
{
  usb_close(self->logic_handle);	
}


int sendLogicCommand(Logic* self,char *command)
{
  return usb_bulk_write(self->logic_handle,0x02,command,(int)command[1],1000);
}

int readLogicData(Logic *self, char *data, int length, int samplerate)
{
  double snooze;
  int i,j,dataindex=0;
  char tmp[length];
  char command[2] = {CMD_GETDATA,2};

	if(samplerate) // sleep only when recording intern and samplerate > 0
	{
		switch(samplerate)
		{
			case SAMPLERATE_5US:   snooze =   5e-6; break;
			case SAMPLERATE_10US:  snooze =  10e-6; break;
			case SAMPLERATE_50US:  snooze =  50e-6; break;
			case SAMPLERATE_100US: snooze = 100e-6; break;
			case SAMPLERATE_1MS:   snooze =   1e-3; break;
			case SAMPLERATE_10MS:  snooze =  10e-3; break;
			default:               snooze = 100e-3; // SAMPLERATE_100MS
		}

		snooze *= length;
		snooze += 1.5;
//	printf("snooze: %d\n", (int) snooze);
    sleep( (int) snooze );
  }

//  printf("length: %d\n", length);

  while(1) 
  {
    sendLogicCommand(self,command);

    i = usb_bulk_read(self->logic_handle,0x02,tmp, length > 60 ? 60 : length,100);

//    printf("i: %d, %s\n", i, usb_strerror());
    for(j = 0; ((j<i) && (dataindex<length)); j++)
    {
      data[dataindex]=tmp[j];
//      printf("dataindex: %d | %d\n", dataindex, (int) tmp[dataindex]);
      dataindex++;
    }
    if(dataindex >= (length-1))
      break;
  }
  return dataindex;
}

int readLogicResults(Logic* self,char *data)
{
return 0; // XXX	
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
  return sendLogicCommand(self,command);
}

int GetLogicMode(Logic* self)
{
  char command[2] = {CMD_GETSCOPEMODE,2};
  return sendLogicCommand(self,command);
}


int GetLogicFIFOLoad(Logic* self)
{
  char command[2] = {CMD_GETFIFOLOAD,2};
  return sendLogicCommand(self,command);
}


void Recording(Logic* self,char samplerate,int numbers,char* data)
{
  SetLogicMode(self,MODE_LOGIC);
  SetLogicSampleRate(self,samplerate);
  StartLogic(self);
  readLogicData(self, data, numbers, 0); //XXX
  StopLogic(self);
}


void RecordingInternal(Logic* self,char samplerate)
{
  SetLogicMode(self,MODE_LOGICINTERN);

  SetLogicSampleRate(self,samplerate);
//  printf("logic samplerate: %d\n", GetLogicSampleRate(self));

  StartLogic(self);
//  printf("d\n");

  // TODO check here with an endless loop and  GetLogicState if record is ready
  // return;

/*
int i;
  while((i = GetLogicState(self)) != STATE_DONOTHING)
  {
    printf("schlafe %d\n", i);
    sleep(1);
  }
  printf("fertig");
  */
}

void GetRecordInternal(Logic *self, char *data, int length, int samplerate)
{
  if(length > 1000)
    length = 1000;
  readLogicData(self, data, length, samplerate);
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

