/* usbn960x.c
* Copyright (C) 2006  Benedikt Sauter
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "usbn960x.h"
#include "../fifo.h"


EPInfo	EP0rx;
EPInfo	EP0tx;

FunctionInfo  USBNFunctionInfo;

void _USBNInitEP0(void)
{
  EP0rx.usbnCommand   = RXC0;
  EP0rx.usbnData      = RXD0;
  EP0rx.usbnControl   = EPC0;
  EP0rx.DataPid	      = 0;
  EP0rx.usbnfifo      = 8;


  EP0tx.usbnCommand   = TXC0;
  EP0tx.usbnData      = TXD0;
  EP0tx.usbnControl   = EPC0;
  EP0tx.DataPid	      = 0;
  EP0tx.usbnfifo      = 8;

} 


// ********************************************************************
// copy data to fifo for sending 
// ********************************************************************
/// copy mem to software fifo for endpoint
void _USBNMemFIFO(fifo_t *fifo,char* data,int size)
{  
  int i;
  for(i=0;i<size;i++)
  {
    fifo_put (fifo, (char)data[i]); 
  } 
}



// ********************************************************************
// Interrupt Event Handler
// ********************************************************************


void _USBNNackEvent(void)
{
  unsigned char event;
  event = USBNRead(NAKEV);
  //USBNWrite(RXC1,FLUSH);	//re-enable the receiver  
  //USBNWrite(RXC1,RX_EN);	//re-enable the receiver  
  USBNDebug("nack event\r\n");
 /* 
  if (EP0tx.Size > EP0tx.usbnfifo)	  //multi-pkt status stage? 
  {
    //USBNDebug("flush");
    USBNWrite(TXC0,FLUSH);	    //flush TX0 and disable   
    //USBNWrite(RXC0,RX_EN);	//re-enable the receiver  
    EP0tx.DataPid	      = 1;
  }
  */
}




void _USBNReceiveEvent(void)
{
  unsigned char event;
  void (*ptr)();
  char buf[64];
  char *bufp=&buf[0];
  event = USBNRead(RXEV);
  char tmp;
  int i=0;
  
  USBNDebug("rx event\r\n");
  if(event & RX_FIFO0) _USBNReceiveFIFO0();
  
  // dynamic function call
  else if(event & RX_FIFO1) 
  {
    USBNRead(RXS1);

    *bufp = USBNRead(RXD1);
    for(i=0;i<63;i++) 
      *(++bufp)=USBNBurstRead(); 
    
    if(rxfifos.rx1==1){
      ptr = rxfifos.func1;
      (*ptr)(&buf);
    }
    USBNWrite(RXC1,FLUSH);   
    USBNWrite(RXC1,RX_EN);    
    return;
  }

  // dynamic function call
  else if(event & RX_FIFO2) 
  {
    USBNRead(RXS2);

    *bufp = USBNRead(RXD2);
    for(i=0;i<63;i++) 
      *(++bufp)=USBNBurstRead(); 
    
    if(rxfifos.rx2==1){
      ptr = rxfifos.func2;
      (*ptr)(&buf);
    }
    USBNWrite(RXC2,FLUSH);   
    USBNWrite(RXC2,RX_EN);    
    return;
  }

  // dynamic function call
  else if(event & RX_FIFO3) 
  {
    USBNRead(RXS3);
    for(i=0;i<64;i++) 
      buf[i]=USBNRead(RXD3); 

    if(rxfifos.rx3==1){
      ptr = rxfifos.func1;
      (*ptr)(&buf);
    }
  }
  else {}
}


void _USBNTransmitEvent(void)
{
  unsigned char event;
  void (*ptr)();
  event = USBNRead(TXEV);
  //USBNDebug("tx event\r\n");
  if(event & TX_FIFO0) _USBNTransmitFIFO0();
  // dynamic function call
  
  else if(event & TX_FIFO1) 
  {
    USBNRead(TXS1);

    if(txfifos.tx1==1){
      ptr = txfifos.func1;
      (*ptr)();
    }
    return;
  }
  else {
    #if DEBUG
      USBNDebug("tx event\r\n");
    #endif

    //USBNRead(TXS1);                        // get transmitter status
    USBNRead(TXS2);                        // get transmitter status
    USBNRead(TXS3);                        // get transmitter status
  }
}

void _USBNAlternateEvent(void)
{
  unsigned char event;
  event = USBNRead(ALTEV);
  USBNDebug("alt event\r\n");

  if(event & ALT_RESET)
  {
    USBNWrite(NFSR,RST_ST);                   // NFS = NodeReset
    USBNWrite(FAR,AD_EN+0); 
    USBNWrite(EPC0,0x00);
    USBNWrite(TXC0,FLUSH);
    USBNWrite(RXC0,RX_EN);                    // allow reception
    USBNWrite(NFSR,OPR_ST);                   // NFS = NodeOperational
  	USBNDebug("reset\r\n");
  }
  else if(event & ALT_SD3)
  {
    USBNWrite(ALTMSK,ALT_RESUME+ALT_RESET);   // adjust interrupts
    USBNWrite(NFSR,SUS_ST);                   // enter suspend state
  	USBNDebug("sd3\r\n");
  }
  else if(event & ALT_RESUME)
  {
    USBNWrite(ALTMSK,ALT_SD3+ALT_RESET);
    USBNWrite(NFSR,OPR_ST);
  	USBNDebug("resume\r\n");
  }
  else if(event & ALT_EOP)
  {
  	USBNDebug("eop\r\n");
  }

  else
  {
  	USBNDebug("else\r\n");
  }
}


// ********************************************************************
// Receive and Transmit functions for EPs          
// ********************************************************************
void _USBNReceiveFIFO0(void)
{
  unsigned char rxstatus;
  char Buf[8];
  DeviceRequest *req;
  int i;

  #if DEBUG
  USBNDebug("rx\r\n");
  #endif
  rxstatus = USBNRead(RXS0);

  if(rxstatus & SETUP_R)
  {
    for(i=0;i<8;i++){ 
      Buf[i] = USBNRead(EP0rx.usbnData);  
    }
    
    #if DEBUG
    for(i=0;i<8;i++)
      SendHex(Buf[i]); // type - get descr or set address
    USBNDebug("\r\n");
    #endif

    req = (DeviceRequest*)(Buf);
   
    USBNWrite(RXC0,FLUSH);		      // make sure the RX is off 
    USBNWrite(TXC0,FLUSH);		      // make sure the TX is off 
    USBNWrite(EPC0,USBNRead(EPC0)&0x7F);      // turn of stall

    switch (req->bmRequestType & 0x60)  // decode request type     
    {
      case DO_STANDARD:			      // standard request 
        switch (req->bRequest)	      // decode request code     
        {
          case CLR_FEATURE:
	  #if DEBUG 
	    USBNDebug("CLR FEATURE\n\r");
	  #endif
            //_USBNClearFeature(req);
          break;
          case GET_CONFIGURATION:
	  #if DEBUG 
            //USBNDebug("GET CONFIG\n\r");
	  #endif
            _USBNGetConfiguration(req);
          break;
          case GET_DESCRIPTOR:
	  #if DEBUG 
            USBNDebug("GET DESCRIPTOR\n\r");
	  #endif
            _USBNGetDescriptor(req);
          break;
          case GET_INTERFACE:
	  #if DEBUG
            USBNDebug("GET INTERFACE\n\r");	
	  #endif
          break;
          case GET_STATUS:
	  #if DEBUG
            USBNDebug("GET STATUS\n\r");	
	  #endif
          break;
          case SET_ADDRESS:
	  #if DEBUG
            USBNDebug("SET ADDRESS ");	
	  #endif
            _USBNSetAddress(req); 
          break;
          case SET_CONFIGURATION:
	  #if DEBUG
            USBNDebug("SET CONFIGURATION\n\r");	
	  #endif
            _USBNSetConfiguration(req); 
          break;
          case SET_FEATURE:
	  #if DEBUG
            USBNDebug("SET FEATURE\n\r");	
	  #endif
          break;
          case SET_INTERFACE:
	  #if DEBUG
            USBNDebug("SET INTERFACE\n\r");	
	  #endif
	    			USBNWrite(TXC0,TX_TOGL+TX_EN);  //enable the TX (DATA1)
          break;
          default:				// unsupported standard req
	  #if DEBUG
            USBNDebug("unsupported standard req\n\r");
	  #endif
            USBNWrite(EPC0,USBNRead(EPC0)+STALL);      // stall the endpoint
          break;
        }      
      break;
      case DO_CLASS:				// class request        
	#if DEBUG
        USBNDebug("Class request\n\r");
	#endif
	//USBNDecodeClassRequest(req);
	USBNWrite(TXC0,TX_TOGL+TX_EN);  //enable the TX (DATA1)
      break;
      case DO_VENDOR:				// vendor request        
	#if DEBUG
        USBNDebug("Vendor request\n\r");
	#endif
	USBNDecodeVendorRequest(req);
	USBNWrite(TXC0,TX_TOGL+TX_EN);  //enable the TX (DATA1)
      break;              
      default:					// unsupported req type    
	#if DEBUG
	USBNDebug("unsupported req type\r\n");
	#endif
        USBNWrite(EPC0,USBNRead(EPC0)+STALL);      // stall the endpoint 
      break;
    }

      //the following is done for all setup packets.  Note that if
      //no data was stuffed into the FIFO, the result of the fol-
      //lowing will be a zero-length response.                   
      
      // only for stage 2 transfers
      if(req->bmRequestType == 0x00)
	USBNWrite(TXC0,TX_TOGL+TX_EN);  //enable the TX (DATA1)
  }
  else                              // if not a setuppacket
  {
    //USBNDebug("error transmit\r\n");
    if (EP0tx.Size > EP0tx.usbnfifo)   // multi-pkt status stage? 
    {
      if ((rxstatus& 0x5F)!=0x10)   // length error??          
      {
	#if DEBUG
	USBNDebug("length error\r\n");
	#endif
      }
      EP0tx.Size=0;                // exit multi-packet mode  
      USBNWrite(TXC0,FLUSH);       // flush TX0 and disable   
      USBNWrite(RXC0,RX_EN);          // re-enable the receiver  
    }
  } 
}




void _USBNTransmitFIFO0(void)
{
  unsigned char txstat;

  txstat = USBNRead(TXS0);                        // get transmitter status
    #if DEBUG
    USBNDebug("t");
    #endif

  if(txstat & TX_DONE)                            // if transmit completed
  {
    USBNWrite(TXC0,FLUSH);                        // flush TX0 and disable
    //USBNDebug("tx done ");

    if(txstat & ACK_STAT)                         // ACK received
    {
      //USBNDebug("tx ack ");
      if(EP0tx.Index < EP0tx.Size)
      {
	//USBNDebug("next packets ");
        _USBNTransmit(&EP0tx);
      }
      else                                        // not in multi-packet mode
      {
        //USBNDebug("no multi packet ");
        //USBNWrite(RXC0,FLUSH);          // re-enable the receiver
        //USBNWrite(TXC0,TX_EN);          // re-enable the receiver
	USBNWrite(RXC0,RX_EN);               // re-enable the receiver
      }
    }
    else                                              
    // this probably means we issued a stall handshake
    {
      USBNDebug("stall handshake ");
      //USBNFunctionInfo.GetDesc=0;          // exit multi-packet mode
      USBNWrite(RXC0,RX_EN);               // re-enable the receiver
    }
  }
  // otherwise something must have gone wrong with the previous
  // transmission, or we got here somehow we shouldn't have
  else
  { 
    USBNDebug("tx0 error\n");
  }
  // we do this stuff for all tx_0 events
}


// ********************************************************************
// Communicate with the FIFOs         
// ********************************************************************


void _USBNReceive(EPInfo* ep)
{
  /*
  int size=ep->FIFOSize;
  int i;
  for(i=0;i<size;i++){
    ep->Buf[i] = USBNRead(ep->usbnData);  
  }
  // cast to device request
  ep->req = (DeviceReq*)(ep->Buf);
  */ 
}


void _USBNTransmit(EPInfo* ep)
{
  int i;
  if(ep->Size > 0)
  {
    if(ep->Index < ep->Size)
    {
      
      USBNWrite(TXC0,FLUSH);       //send data to the FIFO
      //USBNDebug(" ");
      for(i=0;((i < 8) & (ep->Index < ep->Size)); i++)
      {
        USBNWrite(TXD0,ep->Buf[ep->Index]);
	//USBNWrite(TXD0,fifo_get_nowait(EP0tx.fifo));
	//SendHex(ep->Buf[ep->Index]); // type - get descr or set address
        ep->Index++;
      }

      // if end of multipaket
      if(ep->Size<=ep->Index)
	USBNWrite(RXC0,RX_EN);
    }
    else
    {
      USBNWrite(RXC0,RX_EN);
    }

    // toggle mechanism
    if(ep->DataPid == 1)
    {
      USBNWrite(ep->usbnCommand,TX_TOGL+TX_EN);
      ep->DataPid=0;
    }
    else
    {
      USBNWrite(ep->usbnCommand,TX_EN);
      ep->DataPid=1;
    }
  }
}



// ********************************************************************
// standard request from EP0          
// ********************************************************************

/*

void _USBNClearFeature(void)
{
  switch(EP0rx.Buf[0]&0x03)
  {
    case 0: break; // Device
    case 1: break; // interface
    case 2:     // endpoint 
      switch(EP0rx.Buf[4]&0x0F)
      {
        case 0:
          USBNWrite(EPC0,USBNRead(EPC0)&0x7F);  // clear stall
        break;
        case 1:
          USBNWrite(EPC1,USBNRead(EPC1)&0x7F);  // clear stall
        break;
              case 2:
          USBNWrite(EPC2,USBNRead(EPC2)&0x7F);  // clear stall
        break;
              case 3:
          USBNWrite(EPC3,USBNRead(EPC3)&0x7F);  // clear stall
        break;
              case 4:
          USBNWrite(EPC4,USBNRead(EPC4)&0x7F);  // clear stall
        break;
              case 5:
          USBNWrite(EPC5,USBNRead(EPC5)&0x7F);  // clear stall
        break;
              case 6:
          USBNWrite(EPC6,USBNRead(EPC6)&0x7F);  // clear stall
        break;
        default:
        break;
      }
    break;
    default:    // undefined
    break;
  }
}
*/

void _USBNSetAddress(DeviceRequest *req)
{
 // set the address
  USBNWrite(EPC0,DEF);
  USBNWrite(FAR,AD_EN+req->wValue);
  USBNFunctionInfo.Address = req->wValue;
}


void _USBNGetDescriptor(DeviceRequest *req)
{
  unsigned char index =  req->wValue;
  unsigned char type = req->wValue >> 8;

  EP0tx.Index = 0;
  EP0tx.DataPid = 1;
  switch (type)
  {
    case DEVICE:
      #if DEBUG 
      USBNDebug("DEVICE DESCRIPTOR\n\r");  
      #endif
      EP0tx.Size = DeviceDescriptor.bLength;
      EP0tx.Buf = (char*)&(DeviceDescriptor);
      
      // first get descriptor request is
      // always be answered with first 8 unsigned chars of dev descriptor
      if(req->wLength==0x40)
        EP0tx.Size = 8;
    break;
    case CONFIGURATION:
      #if DEBUG 
      USBNDebug("CONFIGURATION DESCRIPTOR ");  
      #endif

      // send complete tree
      EP0tx.Size =req->wLength;
      EP0tx.Buf = &FinalConfigurationArray[index][0];

    break;
    case STRING:
      #if DEBUG 
	USBNDebug("STRING DESCRIPTOR ");  
	SendHex(index);
	USBNDebug("\r\n");
      #endif

      if(index >0)
      {
	EP0tx.Buf = &FinalStringArray[index][0];
	EP0tx.Size = EP0tx.Buf[0];
      }
      else { 
      #if DEBUG 
	USBNDebug("language descriptor"); 
      #endif
	char lang[]={0x04,0x03,0x09,0x04};
	//EP0tx.Buf = &FinalStringArray[0][0];
	//EP0tx.Size = EP0tx.Buf[0];
	EP0tx.Size=4;
	EP0tx.Buf=lang;

      }
    break;
  }
  //if (EP0rx.Buf[7]==0)                  //if less than 256 req'd  
  //  if (EP0tx.Size > EP0rx.Buf[6]) EP0tx.Size = EP0rx.Buf[6];

  _USBNTransmit(&EP0tx);
}



void _USBNSetConfiguration(DeviceRequest *req)
{
  // configure firstly manually but fix it!
  // define EPaddresses
  // save the configuration index
  // load fct pointer list for out eps
  //

USBNWrite(TXC1,FLUSH);
USBNWrite(EPC1,EP_EN+0x02);      // enable EP1 at adr 1


USBNWrite(RXC1,FLUSH);
USBNWrite(EPC2,EP_EN+0x02); 
USBNWrite(RXC1,RX_EN);


//USBNWrite(TXC3,FLUSH);
//USBNWrite(EPC5,EP_EN+0x05); 


	
/* 
  // in endpoints
  USBNWrite(EPC1,EP_EN+0x01);      // enable EP1 at adr 1
  USBNWrite(EPC3,EP_EN+0x03); 
  USBNWrite(EPC5,EP_EN+0x05); 

  // outendpoints
  USBNWrite(EPC2,EP_EN+0x02); 
  USBNWrite(EPC4,EP_EN+0x04); 
  USBNWrite(EPC6,EP_EN+0x06); 

  USBNWrite(TXC1,FLUSH);
  USBNWrite(TXC2,FLUSH);
  USBNWrite(TXC3,FLUSH);

  USBNWrite(RXC1,FLUSH);
  USBNWrite(RXC2,FLUSH);
  USBNWrite(RXC3,FLUSH);
  
  USBNWrite(RXC1,RX_EN);
  USBNWrite(RXC2,RX_EN);
  USBNWrite(RXC3,RX_EN);
*/
  USBNWrite(TXC0,TX_TOGL+TX_EN);  //enable the TX (DATA1)
  
}

void _USBNGetConfiguration(DeviceRequest *req)
{
  USBNWrite(TXD0,USBNFunctionInfo.ConfigurationIndex);
}
           
