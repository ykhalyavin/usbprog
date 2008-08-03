#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include "uart.h"
#include "usbn2mc.h"
#include "usbn2mc/fifo.h"
#include "../../usbprog_base/firmwarelib/avrupdate.h"

/* PIC specific interface functions */
#include "usbprogPIC.h"
#include "pk_comm.h"
#include "utils/eeprom/eeprom.h"
#include "utils/icsp/icsp.h"
#include "utils/scripts/scripts.h"
#include "utils/auxpin/auxpin.h"

/* Definitions */
#define  F_CPU   16000000

void CommandAnswer(int length);
void Commands(char *buf);
void interrupt_ep_send(void);
void rs232_send(void);
void USBToglAndSend(void);

volatile struct usbprog_t {
  int datatogl;
} usbprog;

volatile int tx1togl=0; 		// inital value of togl bit

//char toUSBBuf[100];	defined inbuf[64] in usbprogPIC.c
//char toRS232Buf[100];

//fifo_t* toRS232FIFO;
//fifo_t* toUSBFIFO;

int togl3=0;
int togl1=0;


struct {
	char		dwDTERrate[4];   //data terminal rate, in bits per second
	char    bCharFormat;  //num of stop bits (0=1, 1=1.5, 2=2)
	char    bParityType;  //parity (0=none, 1=odd, 2=even, 3=mark, 4=space)
	char    bDataBits;    //data bits (5,6,7,8 or 16)
} usb_cdc_line_coding;

enum {
	SEND_ENCAPSULATED_COMMAND = 0,
	GET_ENCAPSULATED_RESPONSE,
	SET_COMM_FEATURE,
	GET_COMM_FEATURE,
	CLEAR_COMM_FEATURE,
	SET_LINE_CODING = 0x20,
	GET_LINE_CODING,
	SET_CONTROL_LINE_STATE,
	SEND_BREAK
};


/* Device Descriptor */

const unsigned char usbrs232[] =
{ 
	0x12,       // 18 length of device descriptor
    0x01,       // descriptor type = device descriptor
    0x10,0x01,  // version of usb spec. ( e.g. 1.1)
    0x00,       // device class
    0x00,       // device subclass
    0x00,       // protocol code
    0x08,       // deep of ep0 fifo in byte (e.g. 8)
    0xD8,0x04,  // vendor id
    0x33,0x00,  // product id
    0x01,0x00,  // revision id (e.g 1.02)
    0x01,       // index of manuf. string
    0x02,       // index of product string
    0x03,       // index of ser. number
    0x02        // number of configs
};

/* Configuration descriptor */

const unsigned char usbrs232Conf[] =
{ 
	0x09,       // 9 length of this descriptor
    0x02,       // descriptor type = configuration descriptor
    0x29,0x00,  // total length with first interface ...
    0x01,       // number of interfaces //bene 01
    0x01,       // number if this config. ( arg for setconfig)
    0x00,       // string index for config
    0xA0,       // attrib for this configuration ( bus powerded, remote wakup support)
    0x32,       // power for this configuration in mA (e.g. 50mA)
				//InterfaceDescriptor
    0x09,       // 9 length of this descriptor
    0x04,       // descriptor type = interface descriptor
    0x00,       // interface number
    0x00,       // alternate setting for this interface
    0x02,       // number endpoints without 0
    0x03,       // class code
    0x00,       // sub-class code
    0x00,       // protocoll code
    0x00,       // string index for interface
	0x09,
	0x21,
	0x01,
	0x00,
	0x00,
	0x01,
	0x22,0x1D,0x00,0x07,0x05,0x81,0x03,0x40,0x00,0x01,0x07,0x05,0x01,0x03,0x40,0x00,0x01
};

char ReportDescriptorKeyboard[] =
{
  0x06,0x00,0xff,0x09,0x01,0xa1,0x01,0x19,
  0x01,0x29,0x40,0x15,0x00,0x26,0xff,0x00,
  0x75,0x08,0x95,0x40,0x81,0x02,0x19,0x01,
  0x29,0x40,0x91,0x02,0xc0
};


/* uart interrupt (only for debugging) */

SIGNAL(SIG_UART_RECV)
{
	//UARTWrite("tipp");
	//fifo_put (toUSBFIFO,UARTGetChar());
	USBNWrite(TXC2,FLUSH);
	USBNWrite(TXD2,UARTGetChar());
	rs232_send();	
}

/* interrupt signal from usb controller */

SIGNAL(SIG_INTERRUPT0)
{
	USBNInterrupt();
}

/* central command parser */
void Commands(char *inbuffer)
{
  unsigned char cmd_idx = 0; 
  do
  {   
   PORTA |= (1<<PA4);	//on
   switch(inbuffer[cmd_idx]) 
   {
    /*
	case PORT_DIRECTION:
		set_direction((uint8_t)inbuffer[1]);
		//answer[0] = PORT_DIRECTION; 
		//answer[1] = 0x00;
		//CommandAnswer(2);
		break;
	*/
	case NO_OPERATION:          // Do nothing
		// format:      0x5A
		cmd_idx++;
		break;

    case GETVERSION:			// Get firmware version
		// format: 		0x76
		// response:	<major><minor><dot>
		GetFWVersion();
		CommandAnswer(BUF_SIZE);
        cmd_idx++;
		break;

    case BOOTMODE:				// Enter Bootloader mode
		// format: 		0x42
		// response:	-
		//EnterBootloader();
		cmd_idx++;
		break;

    case ENTER_UART_MODE:       // Puts the firmware in UART Mode
		// format:      0xB3 <BaudValueL><BaudValueH>
        //                   BaudValue = 65536 – [((1/BAUD) – 3e-6) / 1.67e-7]
		// response:	-
        cmd_idx++;
        //EnterUARTMode(&usb_idx);
		break; 

    case EXIT_UART_MODE:        // Exits the firmware from UART Mode
        // format:      0xB4
	    // response:	-
        cmd_idx++;
        //ExitUARTMode();
		break; 

    case WR_INTERNAL_EE:        // write bytes to PIC18F2550 EEPROM
        // format:      0xB1 <address><datalength><data1><data2>....<dataN>
        //                   N = 32 Max
	    // response:	-
        cmd_idx++;
        //WriteInternalEEPROM(&usb_idx);
		break; 

    case RD_INTERNAL_EE:        // read bytes from PIC18F2550 EEPROM
        // format:      0xB2 <address><datalength>
        //                   N = 32 Max
	    // response:	<data1><data2>....<dataN>
        cmd_idx++;
        //ReadInternalEEPROM(&usb_idx);
        break; 

    case SETVDD:
        // format:      0xA0 <CCPL><CCPH><VDDLim>
        //      CCPH:CCPL = ((Vdd * 32) + 10.5) << 6     where Vdd is desired voltage
        //      VDDLim = (Vfault / 5) * 255              where Vdd < VFault is error
        // response:    -
        //CalAndSetCCP1(inbuffer[usb_idx+2], inbuffer[usb_idx+1]);
        //VddVppLevels.VddThreshold = CalThresholdByte(inbuffer[usb_idx+3]);          // Set error threshold
        cmd_idx += 4;
        break;                 

    case SETVPP:
        // format:      0xA1 <CCPR2L><VPPADC><VPPLim>
        //      CCPR2L = duty cycle.  Generally = 0x40
        //      VPPADC = Vpp * 18.61        where Vpp is desired voltage.
        //      VPPlim = Vfault * 18.61              where Vdd < VFault is error
        // response:    -
		//Vpp_PWM.CCPRSetPoint = inbuffer[usb_idx+1];
		//Vpp_PWM.UppperLimit = CalThresholdByte(inbuffer[usb_idx+2])+1;
		//Vpp_PWM.LowerLimit = Vpp_PWM.UppperLimit - 2;   
        //VddVppLevels.VppThreshold = CalThresholdByte(inbuffer[usb_idx+3]);
        cmd_idx += 4;
        break;  

    case SET_VOLTAGE_CALS:
        // format:      0xB0 <adc_calfactorL><adc_calfactorH><vdd_offset><calfactor>
        //      CCPH:CCPL = (((CCP >> 6) + vdd_offset) * vdd_calfactor) >> 7
        //      CalibratedResult = (ADRES * adc_calfactor) >> 8
        // response:    -
		//VoltageCalibration.adc_calfactor = (inbuffer[usb_idx+1] & 0xFF);
		//VoltageCalibration.adc_calfactor += (inbuffer[usb_idx+2] * 0x100);
		//VoltageCalibration.vdd_offset = inbuffer[usb_idx+3];   
        //VoltageCalibration.vdd_calfactor = inbuffer[usb_idx+4];
        //SaveCalFactorsToEE();
        cmd_idx += 5;
        break;  

    case READ_STATUS:
        GetStatus();
		CommandAnswer(BUF_SIZE);
        cmd_idx++;
        break;  

    case READ_VOLTAGES:
        //GetVddVpp();
        cmd_idx++;
        break;     

	case DOWNLOAD_SCRIPT:		// Store a script in the Script Buffer
		// format:		0xA4 <Script#><ScriptLengthN><Script1><Script2>....<ScriptN>
		// response:	-
		cmd_idx++; 				// point to Script#
		StoreScriptInBuffer(&cmd_idx);
		break;	

    case RUN_SCRIPT:            // run a script from the script buffer
        // format:      0xA5 <Script#><iterations>
	    //response:	-
        cmd_idx++;
        RunScript(inbuffer[cmd_idx], inbuffer[cmd_idx + 1]);
        cmd_idx+=2;
        break;  

    case EXECUTE_SCRIPT:        // immediately executes the included script
         // format:      0xA6 <ScriptLengthN><Script1><Script2>....<ScriptN>
	     // response:	-
         cmd_idx+=1; // points to length byte.
         ScriptEngine(&inbuffer[cmd_idx + 1], inbuffer[cmd_idx]);
         cmd_idx += (inbuffer[cmd_idx] + 1);
         break;  

    case CLR_DOWNLOAD_BUFFER:   // empties the download buffer
         // format:      0xA7
    	 // response:	-
         downloadbuf_mgmt.write_index = 0;   // init buffer to enmpty
         downloadbuf_mgmt.read_index = 0;
         downloadbuf_mgmt.used_bytes = 0;
         cmd_idx++;
         break;  

    case DOWNLOAD_DATA:         // add data to download buffer
         // format:      0xA8 <datalength><data1><data2>....<dataN>
		 // response:	-
         cmd_idx++;
         WriteDownloadDataBuffer(&cmd_idx);
         break;  

    case CLR_UPLOAD_BUFFER:   // empties the upload buffer
         // format:      0xA9
		 // response:	-
         uploadbuf_mgmt.write_index = 0;   // init buffer to enmpty
         uploadbuf_mgmt.read_index = 0;
         uploadbuf_mgmt.used_bytes = 0;
         cmd_idx++;
         break;  
    case UPLOAD_DATA:       // reads data from upload buffer   
         // format:      0xAA
         // response:    <DataLengthN><data1><data2>....<dataN>
         ReadUploadDataBuffer();
		 CommandAnswer(BUF_SIZE);
         cmd_idx++;
         break;  

    case CLR_SCRIPT_BUFFER:
         // format:      0xAB
		 // response:	-
         ClearScriptTable();
         cmd_idx++;
         break; 

    case UPLOAD_DATA_NOLEN:   // reads data from upload buffer   
         // format:      0xAC
         // response:    <data1><data2>....<dataN>
         ReadUploadDataBufferNoLength();
         cmd_idx++;;
         break;  

    case RESET:
         // format:      0xAE
         //response:	-
         //Reset();
         cmd_idx++;
         break; 

    case SCRIPT_BUFFER_CHKSM:
         SendScriptChecksums();
		 CommandAnswer(BUF_SIZE);
         cmd_idx++;
         break; 
	
	default:					// End of Buffer or unrecognized command
		cmd_idx = BUF_SIZE;			// Stop processing.
	} // end switch
  }
  while (cmd_idx < BUF_SIZE); // end DO    
}


void CommandAnswer(int length)
{
  int i;

  USBNWrite(TXC1, FLUSH);
  for(i = 0; i < length; i++)
    USBNWrite(TXD1, outbuf[i]);

  /* control togl bit */
  USBToglAndSend();
}


void USBToglAndSend(void)
{
  if(usbprog.datatogl == 1) {
    USBNWrite(TXC1, TX_LAST+TX_EN+TX_TOGL);
    usbprog.datatogl = 0;
  } else {
    USBNWrite(TXC1, TX_LAST+TX_EN);
    usbprog.datatogl = 1;
  }
}


/*************** usb class HID requests  **************/

// reponse for requests on interface
void USBNDecodeClassRequest(DeviceRequest *req,EPInfo* ep)
{
  //UARTWrite("Class\r\n");
  // uncompleted 
  USBNWrite(TXC0,FLUSH);
  USBNWrite(TXD0,0x29);
  USBNWrite(TXD0,0x40);
  USBNWrite(TXD0,0x91);
  USBNWrite(TXD0,0x02);
  USBNWrite(TXD0,0xC0);
  USBNWrite(TXC0,TX_TOGL+TX_EN);
  
}

/* id need for live update of firmware */
void USBNDecodeVendorRequest(DeviceRequest *req)
{
	switch(req->bRequest)
	{
	case STARTAVRUPDATE:
		avrupdate_start();
	break;
	}
}

// class requests
void USBNInterfaceRequests(DeviceRequest *req,EPInfo* ep)
{
        // 81 06 22 get report descriptor
	switch(req->bRequest)
	{
	  case GET_DESCRIPTOR:
	    ep->Index=0;
	    ep->DataPid=1;
	    ep->Size=29;
	    ep->Buf=ReportDescriptorKeyboard;
	  break;
	  default:
		;
	}
}

void store_in_buf(char * buf)
{
	int i;
	for(i=0; i<63; i++)
	{
		inbuf[i] = buf[i];
	}
}

// used
void FromPC(char * buf)
{
	  store_in_buf(buf);
	  Commands(buf);
      //usbprog.datatogl=0;
}

// togl pid for in endpoint
void interrupt_ep_send(void)
{
	if(togl3==1) {
		togl3=0;
		USBNWrite(TXC1,TX_LAST+TX_EN);
	} else {
		togl3=1;
		USBNWrite(TXC1,TX_LAST+TX_EN+TX_TOGL);
	}
}

// togl pid for in endpoint
void rs232_send(void)
{
	if(togl1==1) {
		togl1=0;
		USBNWrite(TXC2,TX_LAST+TX_EN);
	} else {
		togl1=1;
		USBNWrite(TXC2,TX_LAST+TX_EN+TX_TOGL);
	}
}


/*************** main function  **************/

int main(void)
{
	// init fifos
	//fifo_init (toRS232FIFO, toRS232Buf, 100);
	//fifo_init (toUSBFIFO, inbuf, 64);
	
	USBNCallbackFIFORX1(&FromPC);
	//USBNCallbackFIFOTX2Ready(&USBtoRS232);
	
	DDRA = (1 << DDA4);
		
	sei();			// activate global interrupts
	UARTInit();		// only for debugging

	// setup usbstack with your descriptors
	USBNInit(usbrs232,usbrs232Conf);

	_USBNAddStringDescriptor(""); //pseudo lang
	_USBNAddStringDescriptor("Microchip Technology Inc.");
	//_USBNAddStringDescriptor("abcdefgh");
	_USBNAddStringDescriptor("PICkit 2 Microcontroller Programmer     ");
	//_USBNAddStringDescriptor("PICkit 2 Microcontroller Programmer");
	//_USBNAddStringDescriptor("ijklmnop");
	_USBNAddStringDescriptor("PIC18F2550");
	//_USBNAddStringDescriptor("12345678");
	_USBNCreateStringField();


	USBNInitMC();		// start usb controller
	USBNStart();		// start device stack
	
	/* Initialize PIC interface */
	usbprogPICInit();
	while(1);
}






