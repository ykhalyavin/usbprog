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

#include <usb.h>



#define MODE_NONE	  0x00
#define MODE_COUNTER	  0x01
#define MODE_LOGIC	  0x02
#define MODE_1CHANNELAD	  0x03
#define MODE_LOGICINTERN  0x04

#define TRIGGER_OFF       0x01
#define TRIGGER_EDGE      0x02
#define TRIGGER_PATTERN   0x03


#define STATE_DONOTHING	  0x01
#define STATE_RUNNING	  0x02
#define STATE_TRIGGER	  0x03

#define SAMPLERATE_5US    0x01
#define SAMPLERATE_10US   0x02
#define SAMPLERATE_50US   0x03
#define SAMPLERATE_100US  0x04
#define SAMPLERATE_1MS    0x05
#define SAMPLERATE_10MS   0x06
#define SAMPLERATE_100MS  0x07

#define CMD_SETMODE	  0x01
#define CMD_STARTSCOPE	  0x02
#define CMD_STOPSCOPE	  0x03
#define CMD_GETSCOPEMODE  0x04
#define CMD_GETSCOPESTATE 0x05
#define CMD_GETFIFOLOAD	  0x06
#define CMD_SETSAMPLERATE 0x07
#define CMD_GETDATA	  0x08

#define CMD_SETEDGETRIG   0x09
#define CMD_SETPATTRIG    0x0A
#define CMD_DEACTIVTRIG   0x0B
#define CMD_GETSNAPSHOT   0x0C



#define HIGH		  1
#define LOW		  0

typedef struct logic Logic;

struct logic {
  struct usb_dev_handle *logic_handle;
//  struct usb_device *logic_device;
};

Logic* openLogic();
void closeLogic(Logic* self);
int sendLogicCommand(Logic* self,char *command);
int readLogicData(Logic* self, char* data,int length,int samplerate);
int readLogicResults(Logic* self,char *data);


void SetLogicMode(Logic* self,char state);
void SetLogicSampleRate(Logic* self,char samplemode);
void StartLogic(Logic* self);
void StopLogic(Logic* self);
int GetLogicState(Logic* self);
int GetLogicMode(Logic* self);
int GetLogicFIFOLoad(Logic* self);



//************* user functions are here

void Recording(Logic* self,char samplerate,int numbers,char* data);

char TakeSnapshot(Logic* self);

void RecordingInternal(Logic* self,char samplerate);
void GetRecordInternal(Logic* self,char*data,int length,int samplerate);

void ActivateEdgeTrigger(Logic* self,int channel,int value);
void ActivatePatternTrigger(Logic* self,char pattern,char ignore);
void DeActivateTrigger(Logic* self);


