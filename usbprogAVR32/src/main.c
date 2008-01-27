/*
 * usbprog - AVR32 programmer
 * Copyright (C) 2006 Benedikt Sauter 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <avr/interrupt.h>
#include "../../usbprog_base/firmwarelib/avrupdate.h"
#include "usbn2mc.h"
#include "cmd.h"


SIGNAL(SIG_INTERRUPT0)
{
	USBNInterrupt();
}


/* id need for live update of firmware */
void USBNDecodeVendorRequest(DeviceRequest *req)
{
	if (req->bRequest == STARTAVRUPDATE)
		avrupdate_start();
}

int main(void)
{
    int conf, interf;

	cmd_init();

    USBNInit();

    USBNDeviceVendorID(0x1786);
    USBNDeviceProductID(0x0C63);
    USBNDeviceBCDDevice(0x0007);

    char lang[]={0x09,0x04};
    _USBNAddStringDescriptor(lang); // language descriptor

    USBNDeviceManufacture("EmbeddedProjects");
    USBNDeviceProduct("usbprogAVR32 programmer ");
    USBNDeviceSerialNumber("ZhuYun");

    conf = USBNAddConfiguration();

    USBNConfigurationPower(conf,50);

    interf = USBNAddInterface(conf,0);
    USBNAlternateSetting(conf,interf,0);

    USBNAddInEndpoint(conf,interf,1,0x02,BULK,64,0,cmd_answer_rest);
    USBNAddOutEndpoint(conf,interf,1,0x02,BULK,64,0,&cmd_parser);

    USBNInitMC();
    sei();

    USBNStart();    // start usb chip

    while(1);
}


