/*
    Hardware Interface Layer (HIL).

    The functions within this file implement the Hardware Interface Layer
    required by the MSP430 debug DLL/so.

    $Id: HIL.c,v 1.12 2006/11/15 20:28:50 cliechti Exp $
*/
// #includes. -----------------------------------------------------------------

#include <time.h>
#if defined(HIL_PPDEV)  ||  defined(HIL_DIRECTIO)
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if defined (__linux__)
#warning Building for Linux!!!
#include <linux/ppdev.h>
#include <linux/parport.h>
#elif defined (__FreeBSD__)
#warning Building for FreeBSD!!!
#include <sys/fcntl.h>
#include <machine/cpufunc.h>
#include <machine/sysarch.h>
#include <dev/ppbus/ppi.h>

#define PPWDATA	        PPISDATA
#define PPWCONTROL      PPISCTRL
#define PPRSTATUS       PPIGSTATUS
/* Usually /dev/ppi0 will do all the job. But...
   using HIL_DIRECTIO might work better...*/
#endif
#elif defined WIN32
#warning Buiding for WIN32!!!
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include "Basic_Types.h"

#define DRIVERNAME      "giveio.sys"
#define SERVICENAME     "giveio"
#define DRIVERDEVICE    "\\\\.\\giveio"

/* Parallel port register offsets. */
#define DATAOFF 0
#define STATOFF 1
#define CTRLOFF 2
#else
#error unknown arch.
#endif

#include "HIL.h"

// #defines. ------------------------------------------------------------------

/* Parallel port data register bits (Write-only). */
#define TDI     0x01
#define TMS     0x02
#define TCK     0x04
#define TCLK    TDI    // Same as TDI
#define VCC     0x10
#define OSC32   0x20
#define OSCPRG  0x40
#define PWR     (0x80 + 0x40 + 0x20)    // Needed for FET430x110

/* Parallel port status register bits (Read-only). */
#define TDO     0x20

/* Parallel port control register bits (Write-only). */
#define RST     0x01
#define EN_TCLK 0x02
#define TST     0x04
#define EN_JTAG 0x08

#define DEFAULT_VCC         3000    // Default Vcc to 3V (3000mV).
#define DEFAULT_RSTDELAY    10      // Default RST/NMI delay to 10mSec.

// (File) Global variables. ---------------------------------------------------

static char useTDI = TRUE;          // Shift intructions using TDI (else use TDO [while securing device]).

static unsigned char portData = 0;  // Copy of write-only parallel port data register.
static unsigned char portCtrl = 0;  // Copy of write-only parallel port control register.

#if defined(HIL_PPDEV)  ||  defined(HIL_REDIRECTIO)
static int fd = 0;                  //file handle for parport/ppdev

static struct timeval _tstart;
static struct timeval _tend;
static struct timezone tz;
#else
static LARGE_INTEGER _tstart;
static LARGE_INTEGER _tend;
static LARGE_INTEGER freq;
#endif

static unsigned int protocol = JTAG;
static unsigned long slowdown_value = 0;

// (Local) Function prototypes. -----------------------------------------------

static BYTE ReadTDO(void);
static void PrepTCLK(void);

#define StoreTCLK()     ((portData  &   TCLK))
#define RestoreTCLK(x)  ((x == 0)  ?  ClrTCLK()  :  SetTCLK())

// Functions. -----------------------------------------------------------------

#if defined(WIN32)  ||  defined(HIL_DIRECTIO)
/* The printer port base address for environments which perform direct hardware
   access. */
static int port_base = 0x378;
#endif

#if defined(__FreeBSD__)  &&  defined(HIL_DIRECTIO)

static int set_ioperm(int from, int num, int turn_on)
{
#if defined(__linux__)
	return ioperm(from, num, turn_on);
#elif defined(__FreeBSD__)
	return i386_set_ioperm(from, num, turn_on);
#else
	#error No ioperm defined. Check OS setting.
#endif
}
#endif

#if defined(WIN32)
static __inline__ int in_byte(int port)
{
    return _inp(port);
}

static __inline__ void out_byte(int port, int data)
{
    _outp(port, data);
}
#elif defined(HIL_DIRECTIO)
static __inline__ int in_byte(int port)
{
    return inb(port);
}

static __inline__ void out_byte(int port, int data)
{
    outb(data, port);
}
#endif

static __inline__ void ClkTCK(void)
{
    ClrTCK();
    SetTCK();
}
#include <stdio.h>
// configurable timer called after changing one of the outputs, used to
// manually slow down the communication
static void slowdown(void)
{
    if (slowdown_value) {
        #ifdef WIN32
        LARGE_INTEGER target;
        LARGE_INTEGER now;
        QueryPerformanceCounter(&target);
        //we're lazy, use a double so that we don't have to care about overflows
        //~ printf("*** %e\n", (double) target.QuadPart);
        target.QuadPart += (double)freq.QuadPart*slowdown_value/1000000;
        //~ printf("*** %e\n",  (double)target.QuadPart);
        do {
            QueryPerformanceCounter(&now);
        } while (now.QuadPart < target.QuadPart);
        #else
        usleep(slowdown_value);
        #endif
    }
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Initialize(CHAR const *port);

Description:
 Initialize the interface.

Parameters:
 port:    Interface port reference (application specific).

Returns:
 STATUS_OK:    The interface was initialized.
 STATUS_ERROR: The interface was not initialized.

Notes:
 1. port is the parameter provided to MSP430_Initialize().
*/
STATUS_T WINAPI HIL_Initialize(CHAR const *port)
{
#if defined(HIL_PPDEV)  ||  defined(HIL_DIRECTIO)
    int mode;

#if defined(__FreeBSD__)  &&  defined(HIL_DIRECTIO)
    if (set_ioperm(port_base + DATAOFF, 3, 1))
    {
        perror("set_ioperm");
        fd = -1;
        return (STATUS_ERROR);
    }
    out_byte(port_base + CTRLOFF, 0x20);	// is this a correct direction?
    return (STATUS_OK);
#endif

    if ((fd = open(port, O_RDWR)) < 0)
    {
        perror("open");
        return (STATUS_ERROR); // Cannot open device
    }
#if defined(__linux__)
    if (ioctl(fd, PPCLAIM))
    {
        perror("ioctl");
        close(fd);
        fd = -1;
        return (STATUS_ERROR); // PPCLAIM failed - cannot access port
    }

    mode = 0;
    if (ioctl(fd, PPDATADIR, &mode))
    {
        perror("ioctl");
        close(fd);
        fd = -1;
        return (STATUS_ERROR); // PPDATADIR failed - cannot access port
    }
#elif defined(__FreeBSD__)
    mode = 0;
    if (ioctl(fd, PPISCTRL, &mode))
    {
        perror("ioctl");
        close(fd);
        fd = -1;
        return (STATUS_ERROR);
    }
#endif
    
#else
    WORD nPort;
    /* LPT port base addresses */
    DWORD portAddress[] =
    {
        0,
        0x378,
        0x278,
        0x3BC
    };
    OSVERSIONINFO vi;
    
    /* Configure the parallel port. */
    if (strstr(port, "0x") == port) {
        port_base = strtol(port, NULL, 0);
    } else {
        nPort = atoi(port); // Convert port string to integer.
        if ((nPort < 1)  ||  (nPort > 3))
            return (STATUS_ERROR); // Invalid port specifier.
    
        port_base = portAddress[nPort];
    }

    QueryPerformanceFrequency(&freq);           //store resolution of timing function
    
    /* Detect the OS. On NT, 2K, or XP the driver needs to be loaded */
    vi.dwOSVersionInfoSize = sizeof(vi);
    GetVersionEx(&vi);
    if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        HANDLE h;
        /* Try to open the driver */
        h = CreateFile(DRIVERDEVICE,
                       GENERIC_READ,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
        if (h == INVALID_HANDLE_VALUE)
            return (STATUS_ERROR); // Could not start the device driver.
        /* Close again immediately.
           The process is now tagged to have the rights it needs.
           The giveio driver remembers that. */
        if (h != NULL)
            CloseHandle(h);          //close the driver's file
    }
#endif    
    return (STATUS_OK);
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Open(void);

Description:
 Enable the JTAG interface to the device.

Parameters:

Returns:
 STATUS_OK:    The JTAG interface was opened.
 STATUS_ERROR: The JTAG interface was not opened.

Notes:
 1. The setting of Vpp to 0 is dependent upon the interface hardware.
 2. HIL_Open() calls HIL_Connect().
*/
STATUS_T WINAPI HIL_Open(void)
{
    HIL_Release(); // Negate control signals before applying power.
    HIL_VPP(0);
    HIL_Connect();
    
    return (STATUS_OK);
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Connect(void);

Description:
 Enable the JTAG connection to the device.

Parameters:

Returns:
 STATUS_OK:    The JTAG connection to the device was enabled.
 STATUS_ERROR: The JTAG connection to the device was not enabled.

Notes:
*/
STATUS_T WINAPI HIL_Connect(void)
{
    portCtrl |= EN_JTAG; // Enable Jtag signal (TDI, TDO, TMS, TCK) buffer
#if defined(HIL_PPDEV)
#if defined(__linux__)
    if (ioctl(fd, PPWCONTROL, &portCtrl))
#elif defined(__FreeBSD__)
    if (ioctl(fd, PPISCTRL, &portCtrl))
#endif
    {
        perror("ioctl");
        return (STATUS_ERROR);
    }
#else
    out_byte(port_base + CTRLOFF, portCtrl);
#endif
    HIL_TST(1);             // Select JTAG pin functions on F11x and F2xx devices.
    
    return (STATUS_OK);
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Release(void);

Description:
 Release the JTAG interface to the device.

Parameters:

Returns:
 STATUS_OK:    The interface was released.
 STATUS_ERROR: The interface was not released.

Notes:
 1. All JTAG interface signals should be tristated and negated.
*/
STATUS_T WINAPI HIL_Release(void)
{
    // Disable Jtag signal (TDI, TDO, TMS, TCK) buffers.
    portCtrl &= ~(EN_JTAG | EN_TCLK);
    // Disable RST signal buffer.
    portCtrl &= ~EN_TCLK;
#if defined(HIL_PPDEV)
#if defined(__linux__)
    if (ioctl(fd, PPWCONTROL, &portCtrl))
#elif defined(__FreeBSD__)
    if (ioctl(fd, PPISCTRL, &portCtrl))
#endif
    {
        perror("ioctl");
        return (STATUS_ERROR);
    }
#else
    out_byte(port_base + CTRLOFF, portCtrl);
#endif
    HIL_TDI(0);
    HIL_TMS(1);
    HIL_TCK(1);
    HIL_TCLK(1);
    HIL_RST(1);
    HIL_TST(0);

    return (STATUS_OK);
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Close(LONG vccOff);

Description:
 Close the interface.

Parameters:
 vccOff: Turn off the device Vcc (0 volts) if TRUE.

Returns:
 STATUS_OK:    The interface was closed.
 STATUS_ERROR: The interface was not closed.

Notes:
*/
STATUS_T WINAPI HIL_Close(LONG vccOff)
{
#if defined(HIL_PPDEV)
    /* Only disconnect from the device and unload the driver if the driver was infact loaded. */
    if (fd >= 0)
    {
        // Turn off device Vcc, negate control signals, and cleanup parallel port.
        if (vccOff)
            HIL_VCC(0); // Turn off device Vcc.
        HIL_Release(); // Disable (tri-state) control signals.

        // Cleanup parallel port driver.
        close(fd);
    }
#elif defined(HIL_DIRECTIO)
    // Turn off device Vcc, negate control signals, and cleanup parallel port.
    if (vccOff)
        HIL_VCC(0); // Turn off device Vcc.
    HIL_Release(); // Disable (tri-state) control signals.
	if (set_ioperm(0x378, 3, 0))
	{
	    perror("set_ioperm");
	    fd = -1;
	    return (STATUS_ERROR);
	}
#else
    // Turn off device Vcc, negate control signals, and cleanup parallel port.
    if (vccOff)
        HIL_VCC(0); // Turn off device Vcc.
    HIL_Release(); // Disable (tri-state) control signals.
#endif
    return (STATUS_OK);
}

#define INSTRUCTION_LEN 8
#define BITS_LEN        32

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_JTAG_IR(LONG instruction);

Description:
 The specified JTAG instruction is shifted into the device.

Parameters:
 instruction: The JTAG instruction to be shifted into the device.

Returns:
 The byte shifted out from the device (on TDO).

Notes:
 1. The byte instruction is passed as a LONG
 2. The byte result is returned as a LONG.
 3. This function must operate in conjunction with HIL_TEST_VPP(). When the parameter to
    HIL_TEST_VPP is FALSE, shift instructions into the device via TDO. No results are shifted out.
    When the parameter to HIL_TEST_VPP is TRUE, shift instructions into the device via TDI/VPP and
    shift results out of the device via TDO. 
*/
LONG WINAPI HIL_JTAG_IR(LONG instruction)
{
    WORD i;
    int tdo;
    int tclk;

    tclk = portData & TCLK; // Preserve TCLK (pin shared with TDI).

    // Jtag state machine: Run Test Idle.
    SetTMS();
    // JTAG FSM state = Select DR Scan
    ClkTCK();
    // JTAG FSM state = Select IR Scan
    ClkTCK();
    
    ClrTMS();
    // JTAG FSM state = Capture IR
    ClkTCK();
    // JTAG FSM state = Shift IR
    ClkTCK();
    
    tdo = 0;
    for (i = 0;  i < INSTRUCTION_LEN;  i++) // Shift in instruction on TDI (LSB first).
    {
        // Normally shift-in the instruction using TDI. However, while securing a device which takes Vpp via the
        // TDI/VPP pin, shift-in the instruction using TDO. There is no value shifted out of TDO at that time.
        if ((instruction & 1))
        {
            if (useTDI)
                SetTDI();
            else
                SetTDO();
        }
        else
        {
            if (useTDI)
                ClrTDI();
            else
                ClrTDO();
        }
        instruction >>= 1;
        if (i == INSTRUCTION_LEN - 1)
            SetTMS(); // Prepare to exit state.
        ClkTCK();

        // Capture TDO. Expect JTAG version (unless shift-in is using TDO).
        tdo <<= 1;
        if (useTDI  &&  ReadTDO())
            tdo |= 1;
    }
    RestoreTCLK(tclk);
    PrepTCLK();         // Set JTAG FSM back into Run-Test/Idle
    SetTMS();           // Set TMS to default state (minimize power consumption).
    return tdo;
}

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_TEST_VPP(LONG mode);

Description:
 Set the operational mode of HIL_JTAG_IR().

Parameters:
 mode: FALSE: JTAG instructions are shifted into the device via TDO. No results are shifted out.
              During secure operations, Vpp is applied on TDI/VPP.
       TRUE:  JTAG instructions are shifted into the device via TDI/VPP and results are shifted out
              via TDO. During secure operations, Vpp is applied on TEST.

Returns:
 The previous mode (FALSE: TDO, TRUE: TDI/VPP).

Notes:
 1. This function operates in conjunction with HIL_JTAG_IR() and HIL_VPP().
 2. Since the FET Interface Module does not support routing the shift-in bit stream to TDO, this
    function has no significant effect (other than setting the associated file global variable).
*/
LONG WINAPI HIL_TEST_VPP(LONG mode)
{
    LONG oldMode = (LONG) useTDI;

    useTDI = (char) mode;
    return (oldMode);
}

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_JTAG_DR(LONG data, LONG bits);

Description:
 The specified JTAG data is shifted into the device.

Parameters:
 data: The JTAG data to be shifted into the device.
 bits: The number of JTAG data bits to be shifted into the device (8 or 16).

Returns:
 "bits" bits shifted out from the device (on TDO).

Notes:
 1. The byte or word data is passed as a LONG.
 2. The byte or word result is returned as a LONG.
*/
LONG WINAPI HIL_JTAG_DR(LONG data, LONG bits)
{
    WORD tdo;
    DWORD read[BITS_LEN];
    int i;
    int tclk;

    tclk = portData & TCLK; // Preserve TCLK (pin shared with TDI).
    tdo = 0;
    bits = (bits > BITS_LEN)  ?  BITS_LEN  :  bits; // Limit the number of bits supported.

    /* Jtag state machine: Run Test Idle. */
    SetTMS();
    ClkTCK(); /* Jtag state machine: Select DR Scan. */
    ClrTMS();
    ClkTCK(); /* Jtag state machine: Capture DR. */
    ClkTCK(); /* Jtag state machine: Shift DR. */

    for (i = bits - 1;  i >= 0;  i--) // Shift in data on TDI (MSB first).
    {
        if ((data & (1 << i)))
            SetTDI();
        else
            ClrTDI();
        if (i == 0)
            SetTMS(); // Prepare to exit state.
        ClkTCK();

        /* Capture data on TDO. */
        read[bits - 1 - i] = ReadTDO();
    }
    RestoreTCLK(tclk);
    // Jtag state machine: Exit 1 DR.
    ClkTCK(); // Jtag state machine: Update DR.
    ClrTMS();
    ClkTCK(); // Jtag state machine: Run Test Idle.
    SetTMS(); // Set TMS to default state (minimize power consumption).

    for (i = tdo = 0;  i < bits;  i++)
        tdo = (tdo << 1) | ((read[i] & TDO) == TDO);

    return (tdo);
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_VCC(LONG voltage);

Description:
 Set the device Vcc pin to voltage/1000 volts.

Parameters:
 voltage: The device Vcc pin is set to voltage/1000 volts.

Returns:
 STATUS_OK:    The Vcc was set to voltage.
 STATUS_ERROR: The Vcc was not set to voltage.

Notes:
 1. This function is dependant upon the interface hardware. The FET interface module does not
    support this functionality.
 2. A "voltage" of zero (0) turns off voltage to the device.
 3. If the interface hardware does not support setting the device voltage to a specific value,
    a non-zero value should cause the device voltage to be set to a value within the device
    Vcc specification (i.e., a default Vcc). Insure that the default Vcc voltage supports FLASH
    operations.
 4. Insure that Vcc is stable before returning from this function.
*/
STATUS_T WINAPI HIL_VCC(LONG voltage)
{
#if defined(HIL_PPDEV)
    if (voltage)
    {
        // Power on
        portData |= PWR; // Apply power to the regulator.
        ioctl(fd, PPWDATA, &portData);
        portData |= VCC; // Enable the regulator.
        ioctl(fd, PPWDATA, &portData);
    }
    else
    {
        // Power off
        portData &= ~(VCC | PWR);
        ioctl(fd, PPWDATA, &portData);
    }
#else
    if (voltage)
    {
        // Power on
        out_byte(port_base + DATAOFF, portData |= PWR);
        out_byte(port_base + DATAOFF, portData |= VCC);
    }
    else
    {
        // Power off
        out_byte(port_base + DATAOFF, portData &= ~(VCC | PWR));
    }
#endif

    HIL_DelayMSec(40); // Delay to give the device time to power-up.
    return (STATUS_OK);
}

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TST(LONG state);

Description:
 Set the state of the device TST pin.

Parameters:
 state: The device TST pin is set to state (0/1).
        Special values: if one of the bits in the mask 0xff00 is set, then
        is the extra delay disabled, allowinf for very quick changes on the
        TST pin (default is to make a delay in the millisecond range)
        
        Examples:
            HIL_TST(1) ->  test on vcc, 10 ms delay before and after pin change
            HIL_TST(0) ->  test on gnd, 10 ms delay before and after pin change

            HIL_TST(0xff01) ->  test on vcc, as fast as possible
            HIL_TST(0xff00) ->  test on gnd, as fast as possible

Returns:

Notes:
 1. Not all MSP430 devices have a TST pin.
*/
void WINAPI HIL_TST(LONG state)
{
    if ((state & 0xff00) == 0) HIL_DelayMSec(10);           //for slow hardware
    if (state & 1)
        portCtrl |= TST;
    else
        portCtrl &= ~TST;
#if defined(HIL_PPDEV)
    ioctl(fd, PPWCONTROL, &portCtrl);
#else
    out_byte(port_base + CTRLOFF, portCtrl);
#endif
    if ((state & 0xff00) == 0) HIL_DelayMSec(10);          //for slow hardware
}

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TCK(LONG state);

Description:
 Set the state of the device TCK pin.

Parameters:
 state: The device TCLK pin is set to state (0/1/POS_EDGE (0->1)/NEG_EDGE (1->0)).

Returns:

Notes:
*/
void WINAPI HIL_TCK(LONG state)
{
#if defined(HIL_PPDEV)
    switch (state)
    {
    case 0:
        portData &= ~TCK;
        ioctl(fd, PPWDATA, &portData);
        break;
    case POS_EDGE:
        portData &= ~TCK;
        ioctl(fd, PPWDATA, &portData);
        slowdown();
        portData |=  TCK;
        ioctl(fd, PPWDATA, &portData);
        break;
    case NEG_EDGE:
        portData |=  TCK;
        ioctl(fd, PPWDATA, &portData);
        slowdown();
        portData &= ~TCK;
        ioctl(fd, PPWDATA, &portData);
        break;
    default:
        portData |=  TCK;
        ioctl(fd, PPWDATA, &portData);
        break;
    }
#else
    switch (state)
    {
    case 0:
        out_byte(port_base + DATAOFF, portData &= ~TCK);
        break;
    case POS_EDGE:
        out_byte(port_base + DATAOFF, portData &= ~TCK);
        slowdown();
        out_byte(port_base + DATAOFF, portData |=  TCK);
        break;
    case NEG_EDGE:
        out_byte(port_base + DATAOFF, portData |=  TCK);
        slowdown();
        out_byte(port_base + DATAOFF, portData &= ~TCK);
        break;
    default:
        out_byte(port_base + DATAOFF, portData |=  TCK);
        break;
    }
#endif
    slowdown();
}

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TMS(LONG state);

Description:
 Set the state of the device TMS pin.

Parameters:
 state: The device TMS pin is set to state (0/1).

Returns:

Notes:
*/
void WINAPI HIL_TMS(LONG state)
{
    if (state)
        portData |= TMS;
    else
        portData &= ~TMS;
#if defined(HIL_PPDEV)
    ioctl(fd, PPWDATA, &portData);
#else
    out_byte(port_base + DATAOFF, portData);
#endif
    slowdown();
}

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TDI(LONG state);

Description:
 Set the state of the device TDI pin.

Parameters:
 state: The device TDI pin is set to state (0/1).

Returns:

Notes:
*/
void WINAPI HIL_TDI(LONG state)
{
    if (state)
        portData |= TDI;
    else
        portData &= ~TDI;
#if (HIL_PPDEV)
    ioctl(fd, PPWDATA, &portData);
#else
    out_byte(port_base + DATAOFF, portData);
#endif
    slowdown();
}

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TDO(LONG state);

Description:
 Set the state of the device TDO pin.

Parameters:
 state: The device TDO pin is set to state (0/1).

Returns:

Notes:
*/
void WINAPI HIL_TDO(LONG state)
{
    if (state)
        portData |= TDO;
    else
        portData &= ~TDO;
#if (HIL_PPDEV)
    ioctl(fd, PPWDATA, &portData);
#else
    out_byte(port_base + DATAOFF, portData);
#endif
    slowdown();
}

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_TCLK(LONG state);

Description:
 Set the state of the device TCLK pin.

Parameters:
 state: The device TCLK pin is set to state (0/1/POS_EDGE (0->1)/NEG_EDGE (1->0)).

Returns:

Notes:
*/
void WINAPI HIL_TCLK(LONG state)
{
#if defined(HIL_PPDEV)
    switch (state)
    {
	case 0:
	    portData &= ~TCLK;
	    ioctl(fd, PPWDATA, &portData);
	    break;
	case POS_EDGE:
	    portData &= ~TCLK;
	    ioctl(fd, PPWDATA, &portData);
            slowdown();
	    portData |=  TCLK;
	    ioctl(fd, PPWDATA, &portData);
	    break;
	case NEG_EDGE:
	    portData |=  TCLK;
	    ioctl(fd, PPWDATA, &portData);
            slowdown();
	    portData &= ~TCLK;
	    ioctl(fd, PPWDATA, &portData);
	    break;
	default:
	    portData |=  TCLK;
	    ioctl(fd, PPWDATA, &portData);
	    break;
    }
#else
    switch (state)
    {
    case 0:
        out_byte(port_base + DATAOFF, portData &= ~TCLK);
        break;
    case POS_EDGE:
        out_byte(port_base + DATAOFF, portData &= ~TCLK);
        slowdown();
        out_byte(port_base + DATAOFF, portData |=  TCLK);
        break;
    case NEG_EDGE:
        out_byte(port_base + DATAOFF, portData |=  TCLK);
        slowdown();
        out_byte(port_base + DATAOFF, portData &= ~TCLK);
        break;
    default:
        out_byte(port_base + DATAOFF, portData |=  TCLK);
        break;
    }
#endif
    slowdown();
}

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_RST(LONG state);

Description:
 Set the state of the device RST pin.

Parameters:
 state: The device RST pin is set to state (0, 1, 2, 3).

Returns:

Notes:
 1. As implemented, a state == 0 asserts RST (active low), while a state == 1 tri-states RST.
    Thus, a pull-up resistor on RST is assumed.
 2. As implemented, EN_TCLK and RST are negative logic.
*/
void WINAPI HIL_RST(LONG state)
{
    switch (state)
    {
    case 0:
        // Enable Reset Driver
        portCtrl |= RST;
        portCtrl |= EN_TCLK;
        break;
    case 1:
        // Disable Reset Driver
        portCtrl &= ~RST;
        portCtrl &= ~EN_TCLK;
        break;
    case 2:
        // Drive RST high
        portCtrl &= ~RST;
        portCtrl |= EN_TCLK;
        break;
    default:
        portCtrl &= ~RST;
        portCtrl |= EN_TCLK;
        break;
    }
#if defined(HIL_PPDEV)
    ioctl(fd, PPWCONTROL, &portCtrl);
#else
    out_byte(port_base + CTRLOFF, portCtrl);
#endif
    slowdown();
    /* Delay to allow for voltage supervisor/reset delay (if present). */
    if (state) {
        HIL_DelayMSec(DEFAULT_RSTDELAY);
    }
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_VPP(LONG voltage);

Description:
 Set the device Vpp pin to voltage/1000 volts.

Parameters:
 voltage: The device Vpp pin is set to voltage/1000 volts.

Returns:
 STATUS_OK:    The Vpp was set to voltage.
 STATUS_ERROR: The Vpp was not set to voltage.

Notes:
 1. This function is dependant upon the interface hardware. The FET interface module does not
    support this functionality.
 2. A "voltage" of zero (0) turns off Vpp voltage to the device.
 3. If the interface hardware does not support setting the device Vpp voltage to a specific value,
    a non-zero value should cause the device Vpp voltage to be set to a value within the device
    Vpp specification (i.e., a default Vpp). Insure that the default Vpp voltage supports FLASH
    operations.
 4. The parameter to HIL_TEST_VPP() can be used to determine if VPP is applied to TDI/VPP (FALSE)
    or to TEST/VPP (TRUE).
 5. Insure that Vpp is stable before returning from this function.
*/
STATUS_T WINAPI HIL_VPP(LONG voltage)
{
    /* TODO: not implemented on the parallel port JTAG device */
    return (STATUS_OK);
}

// Time delay and timer functions ---------------------------------------------

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_DelayMSec(LONG mSeconds);

Description:
 Delay for mSeconds milliseconds.

Parameters:
 mSeconds: The delay time (milliseconds).

Returns:

Notes:
 1. The precision of this delay function does not have to be high; "approximate" milliseconds delay is
    sufficient. Rather, the length of the delay needs to be determined precisely. The length of the delay
    is determined precisely by computing the difference of a timer value read before the delay and the
    timer value read after the delay.
*/
void WINAPI HIL_DelayMSec(LONG mSeconds)
{
#if defined(HIL_PPDEV)  ||  defined(HIL_DIRECTIO)
    usleep(mSeconds*1000);
#else
    clock_t goal;
    for (goal = clock() + mSeconds;  clock() < goal;  )
        __asm__ volatile(" ");
#endif
}

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_StartTimer(void);

Description:
 Start the (precision) timer.

Parameters:

Returns:

Notes:
 The timer should have a resolution of at least one millisecond.
*/
void WINAPI HIL_StartTimer(void)
{
#ifdef WIN32
    QueryPerformanceCounter(&_tstart);
#else
    gettimeofday(&_tstart, &tz);
#endif
}

/* ----------------------------------------------------------------------------
Function:
 ULONG WINAPI HIL_ReadTimer(void);

Description:
 Read the (precision) timer.

Parameters:

Returns:
 The value of the timer.

Notes:
 The timer should have a resolution of at least one millisecond.
*/
ULONG WINAPI HIL_ReadTimer(void)
{
#ifdef WIN32
    QueryPerformanceCounter(&_tend);
    return (1000*(_tend.QuadPart - _tstart.QuadPart))/(freq.QuadPart);
#else
    long long t1;
    long long t2;

    gettimeofday(&_tend, &tz);
    t1 = ((long long) _tstart.tv_sec)*1000 + _tstart.tv_usec/1000;
    t2 = ((long long) _tend.tv_sec)*1000 + _tend.tv_usec/1000;
    return t2 - t1;
#endif
}

/* ----------------------------------------------------------------------------
Function:
 void WINAPI HIL_StopTimer(void);

Description:
 Stop the (precision) timer.

Parameters:

Returns:

Notes:
*/
void WINAPI HIL_StopTimer(void)
{
    //nop
}

// HIL local support functions. -----------------------------------------------

/* Parallel port status register functions. */
static BYTE ReadTDO(void)
{
    BYTE ret;

#if defined(HIL_PPDEV)
    ioctl(fd, PPRSTATUS, &ret);
#else
    ret = in_byte(port_base + STATOFF);
#endif
    return ret & TDO;
}

//----------------------------------------------------------------------------
/* This function sets the target JTAG state machine (JTAG FSM) back into the 
   Run-Test/Idle state after a shift access.
*/
static void PrepTCLK(void)
{
    // JTAG FSM = Exit-DR
    ClkTCK();
    // JTAG FSM = Update-DR
    ClrTMS();
    ClkTCK();
    // JTAG FSM = Run-Test/Idle
}

void WINAPI HIL_CheckJtagFuse(void)
{
    /* Slow fuse check */
    HIL_TDI(1);  // be sure that all JTAG inputs are high
    HIL_TMS(1);
    HIL_TCK(1);
    /* check fuse */
    HIL_TMS(0);
    HIL_DelayMSec(1);
    HIL_TMS(1);
    HIL_TMS(0);
    HIL_DelayMSec(1);
    HIL_TMS(1);
}

void WINAPI HIL_ResetJtagTap(void)
{
    // Reset Jtag state machine.
    HIL_TMS(1);  // be sure that TMS is high on entry
    HIL_TCK(POS_EDGE);
    HIL_TCK(POS_EDGE);
    HIL_TCK(POS_EDGE);
    HIL_TCK(POS_EDGE);
    HIL_TCK(POS_EDGE);
    HIL_TCK(POS_EDGE); // Jtag state machine: Test Logic Reset.
    HIL_TCK(0);
    HIL_TMS(0);
    HIL_TCK(1); // Jtag state machine: Run Test Idle.
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_Trace(BOOL OnOff, char *str);

Description:
 Switches Debug Trace on/off when compiled for DEBUG_TRACE.

Parameters:

Returns:

Notes:
*/
STATUS_T WINAPI HIL_Trace(BOOL OnOff, char *str)
{
    return STATUS_ERROR;
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_SetProtocol(int protocol_id);

Description:
 Sets the Protocol type (actual: JTAG, SPYBYWIRE)

Parameters:
 protocol_id:  Protocol type (Default: JTAG)

Returns:
 STATUS_OK:    The interface was initialized.
 STATUS_ERROR: The interface was not initialized.

Notes:
*/
STATUS_T WINAPI HIL_SetProtocol(int protocol_id)
{
    if (protocol_id == SPYBIWIREJTAG)
        return STATUS_ERROR;
    protocol = protocol_id;

    return STATUS_OK;
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_sbw_ExecuteFuseBlow();

Description:
 Blow the JTAG fuse on Spy-Bi-Wire devices.
 NOT supported with the simple parallel port interface!

Parameters:
 None

Returns:
 STATUS_OK:    Success
 STATUS_ERROR: failure

Notes:
*/
WINAPI void HIL_sbw_ExecuteFuseBlow(void)
{
}

/* ----------------------------------------------------------------------------
Function:
 STATUS_T WINAPI HIL_sbw_StepPSA(Length);

Description:
 Run the checksum algorith over Length bytes?
 NOT supported with the simple parallel port interface!

Parameters:
 Length number of bytes? to check

Returns:
 STATUS_OK:    Success
 STATUS_ERROR: failure

Notes:
*/
WINAPI void HIL_sbw_StepPSA(LONG Length)
{
}

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_ReadTDO();

Description:
 Read state of TDO line.

Parameters:
 None

Returns:
 true/false for high/low

Notes:
*/
LONG WINAPI HIL_ReadTDO(void)
{
    return ReadTDO() != 0;
}

/* ----------------------------------------------------------------------------
Function:
 LONG WINAPI HIL_SetSlowdown();

Description:
 Configure additional delay after each output change. USed to slow down the
 JTAG communication.
 Use 0 to disable and try at fast as possible.

Parameters:
 Additional delay per edge in microseconds.

Returns:
 void

Notes:
 Very small values may not work as expected when the system doesn't support
 timers at such small resolutions.
*/
void WINAPI HIL_SetSlowdown(LONG microseconds)
{
    slowdown_value = microseconds;
}
