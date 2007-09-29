#ifndef HARDWARE_H
#define HARDWARE_H

//Hardware description of the Serial-JTAG adapter
//part of serJTAGfirmware
//http://mspgcc.sf.net
//chris <cliechti@gmx.net>

#include <io.h>
#include <signal.h>

//#define LFXTCLK_FREQUENCY    7372800
#define LFXTCLK_FREQUENCY    8000000

#define POWER_DOWN_TIME  10  //auto power off time in seconds

//PINS
//--- PORT1 ---
//#define TCLK            BIT0
#define TXBSL           BIT1
#define TEST            BIT2
#define TDO             BIT3
#define TDI             BIT4
#define TMS             BIT5
#define TCK             BIT6
#define RST             BIT7

#define TCLK            TDI                     //TCLK is provided on TDI

#define P1OUT_INIT      TXBSL|TCLK|TDI|TMS|TCK|RST|TEST
#define P1SEL_INIT      0
#define P1DIR_INIT      0

#define P1IE_INIT       0
#define P1IES_INIT      0

#define JTAGIN          P1IN
#define JTAGOUT         P1OUT
#define JTAGDIR         P1DIR
#define JTAGSEL         P1SEL


//--- PORT2 ---
#define VPPONTDI        BIT0
#define VPPONTEST       BIT1
#define RXBSL           BIT2
#define TDIENABLE       BIT3
#define SPARE24         BIT4
#define LEDRT           BIT5
#define VCC26           BIT6
#define STARTKEY        BIT7

#define P2OUT_INIT      TDIENABLE|VCC26
#define P2SEL_INIT      0
#define P2DIR_INIT      VPPONTDI|VPPONTEST|TDIENABLE|LEDRT|VCC26

#define CAPD_INIT       P2DIR_INIT

#define P2IE_INIT       STARTKEY
#define P2IES_INIT      STARTKEY

#define VPPOUT          P2OUT   // VPP ports are P2.x
#define VPPDIR          P2DIR
#define VPPSEL          P2SEL

//--- PORT3 ---
#define JMP3            BIT0
#define JMP4            BIT1
#define CTS             BIT2
#define RTS             BIT3
#define TX              BIT4
#define RX              BIT5
#define JMP1            BIT6
#define JMP2            BIT7

#define P3OUT_INIT      RTS
#define P3SEL_INIT      TX|RX
#define P3DIR_INIT      TX|RTS

//--- PORT4 ---
#define VCC40           BIT0
#define GND41           BIT1
#define GND42           BIT2
#define GND43           BIT3
#define GND44           BIT4
#define GND45           BIT5
#define GND46           BIT6
#define GND47           BIT7

#define P4OUT_INIT      VCC40
#define P4SEL_INIT      0
#define P4DIR_INIT      VCC40|GND41|GND42|GND43|GND44|GND45|GND46|GND47

//--- PORT5 ---
#define GND50           BIT0
#define GND51           BIT1
#define GND52           BIT2
#define DEBUG53         BIT3
#define DEBUG54         BIT4
#define DEBUG55         BIT5
#define DEBUG56         BIT6
#define DEBUG57         BIT7

#define P5OUT_INIT      0
#define P5SEL_INIT      0
#define P5DIR_INIT      GND50|GND51|GND52|DEBUG53|DEBUG54|DEBUG55|DEBUG56|DEBUG57

//--- PORT6 ---
#define GND60           BIT0
#define GND61           BIT1
#define GND62           BIT2
#define GND63           BIT3
#define LEDGN           BIT4
#define GND65           BIT5
#define GND66           BIT6
#define GND67           BIT7

#define P6OUT_INIT      0
#define P6SEL_INIT      0
#define P6DIR_INIT      GND60|GND61|GND62|GND63|LEDGN|GND65|GND66|GND67

#define IE1_INIT        0
#define IE2_INIT        0
#define ME1_INIT        0
#define ME2_INIT        0

#define WDTCTL_INIT     WDTPW|WDTHOLD
#define DCOCTL_INIT     DCO0|DCO1|DCO2
#define BCSCTL1_INIT    XT2OFF|XTS|DIVA_DIV8|RSEL2|RSEL1|RSEL0
#define BCSCTL2_INIT    SELM_LFXTCLK|DIVS_DIV8

#define DCOCLK_FREQUENCY 4900000        // According to datasheet at VCC=3.0V

#define MCLK_FREQUENCY  LFXTCLK_FREQUENCY
#define SMCLK_FREQUENCY (DCOCLK_FREQUENCY/8)
#define ACLK_FREQUENCY  (LFXTCLK_FREQUENCY/8)

#define TACTL_INIT      TASSEL_ACLK|TACLR|ID_DIV1|TAIE

#define TA_FREQUENCY    (ACLK_FREQUENCY/1)

#define U0CTL_INIT      CHAR
#define U0RCTL_INIT     0
#define U0TCTL_INIT     SSEL_ACLK|TXEPT

#define U0_FREQUENCY    ACLK_FREQUENCY

#if ((U0_FREQUENCY)==1000000)
//115200
#define U0BR1_INIT      0               //Baud rate 1 register init 'U0BR1' 
#define U0BR0_INIT      0x08            //Baud rate 0 register init 'U0BR0'
#define U0MCTL_INIT     0x5b            //Modulation Control Register init 'U0MCTL':
#elif ((U0_FREQUENCY)==921600)
//115200
#define U0BR1_INIT      0               //Baud rate 1 register init 'U0BR1' 
#define U0BR0_INIT      0x08            //Baud rate 0 register init 'U0BR0'
#define U0MCTL_INIT     0x00            //Modulation Control Register init 'U0MCTL':
#else
#error  // If you have a different system crystal, calculate your values and enter here
#endif


//Module reg is different on F12x and F13x/F14x
#ifdef __msp430x12x
  #define U0ME ME2                      //ME2.1 UTXE0 USART0 transmit enable (UART mode)
  #define U0IE IE2                      //RX and TX interrupt of UART0
#else
  #define U0ME ME1                      //ME1.7 UTXE0 USART0 tx enable (UART mode)
  #define U0IE IE1                      //RX and TX interrupt of UART0
#endif

#endif //HARDWARE_H
