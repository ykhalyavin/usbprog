/* command format:

1   byte : command number (see below)
n   bytes: command parameters (data)

   data format:

all 16-bit and 32-bit data is transferred in big endian, i.e. MSB on lower address (first) and LSB on higher address (next)
data must be converted properly on intel machines (PCs)

*/

/* cable status bit fields */
#define RESET_DETECTED_MASK   0x0001
#define RSTO_STATE_MASK       0x0002

/* target types */
#define TARGET_TYPE_CF_BDM    0
#define TARGET_TYPE_JTAG      1

/* if command fails, the device responds with command code CMD_FAILED */
/* if command succeeds, the device responds with the same command number followed by any results as appropriate */

#define MAX_DATA_SIZE         127 /* maximum command parameter/result block size in bytes; this is to make sure that response of READ_BLOCK plus the command status fit into 16 frames exactly */

/* System related commands */
#define CMD_FAILED            1  /* command execution failed (incorrect parameters, target not responding, etc.) */
#define CMD_UNKNOWN           2  /* unknown command */

/* TurboBdmLightCF related commands */
#define CMD_GET_VER           10 /* returns 16 bit HW/SW version number, (major & minor revision in BCD in each byte - HW in MSB, SW in LSB; intel endianism) */
#define CMD_GET_LAST_STATUS   11 /* returns status of the previous command */
#define CMD_SET_BOOT          12 /* request bootloader firmware upgrade on next power-up, parameters: 'B','O','O','T', returns: none */
#define CMD_GET_STACK_SIZE    13 /* parameters: none, returns 16-bit stack size required by the application (so far into the execution) */

/* BDM/debugging related commands */
#define CMD_SET_TARGET        20 /* set target, 8bit parameter: 00=ColdFire(default), 01=JTAG */
#define CMD_RESET             21 /* 8bit parameter: 0=reset to BDM Mode, 1=reset to Normal mode */
#define CMD_GET_STATUS        22 /* returns 16bit status word: bit0 - target was reset since last execution of this command (this bit is cleared after reading), bit1 - current state of the RSTO pin, big endian! */
#define CMD_HALT              23 /* stop the CPU and bring it into BDM mode */
#define CMD_GO                24 /* start code execution from current PC address */
#define CMD_STEP              25 /* perform single step */
#define CMD_RESYNCHRONIZE     26 /* resynchronize communication with the target (in case of noise, etc.) */
#define CMD_ASSERT_TA         27 /* parameter: 8-bit number of 10us ticks - duration of the TA assertion */

/* CPU related commands */
#define CMD_READ_MEM8         30 /* parameter 32bit address, returns 8bit value read from address */
#define CMD_READ_MEM16        31 /* parameter 32bit address, returns 16bit value read from address */
#define CMD_READ_MEM32        32 /* parameter 32bit address, returns 32bit value read from address */

#define CMD_READ_MEMBLOCK8    33 /* parameter 32bit address, returns block of 8bit values read from the address and onwards */
#define CMD_READ_MEMBLOCK16   34 /* parameter 32bit address, returns block of 16bit values read from the address and onwards */
#define CMD_READ_MEMBLOCK32   35 /* parameter 32bit address, returns block of 32bit values read from the address and onwards */

#define CMD_WRITE_MEM8        36 /* parameter 32bit address & an 8-bit value to be written to the address */
#define CMD_WRITE_MEM16       37 /* parameter 32bit address & a 16-bit value to be written to the address */
#define CMD_WRITE_MEM32       38 /* parameter 32bit address & a 32-bit value to be written to the address */

#define CMD_WRITE_MEMBLOCK8   39 /* parameter 32bit address & a block of 8-bit values to be written from the address */
#define CMD_WRITE_MEMBLOCK16  40 /* parameter 32bit address & a block of 16-bit values to be written from the address */
#define CMD_WRITE_MEMBLOCK32  41 /* parameter 32bit address & a block of 32-bit values to be written from the address */

#define CMD_READ_REG          42 /* parameter 8-bit register number to read, returns 32-bit register contents */
#define CMD_WRITE_REG         43 /* parameter 8-bit register number to write & the 32-bit register contents to be written */

#define CMD_READ_CREG         44 /* parameter 16-bit register address, returns 32-bit control register contents */
#define CMD_WRITE_CREG        45 /* parameter 16-bit register address & the 32-bit control register contents to be written */

#define CMD_READ_DREG         46 /* parameter 8-bit register number to read, returns 32-bit debug module register contents */
#define CMD_WRITE_DREG        47 /* parameter 8-bit register number to write & the 32-bit debug module register contents to be written */

/* JTAG commands */
#define CMD_JTAG_GOTORESET    80 /* no parameters, takes the TAP to TEST-LOGIC-RESET state, re-select the JTAG target to take TAP back to RUN-TEST/IDLE */
#define CMD_JTAG_GOTOSHIFT    81 /* parameters 8-bit path option; path option ==0 : go to SHIFT-DR, !=0 : go to SHIFT-IR (requires the tap to be in RUN-TEST/IDLE) */
#define CMD_JTAG_WRITE        82 /* parameters 8-bit exit option, 8-bit count of bits to shift in, and the data to be shifted in (shifted in LSB (last byte) first, unused bits (if any) are in the MSB (first) byte; exit option ==0 : stay in SHIFT-xx, !=0 : go to RUN-TEST/IDLE when finished */
#define CMD_JTAG_READ         83 /* parameters 8-bit exit option, 8-bit count of bits to shift out; exit option ==0 : stay in SHIFT-xx, !=0 : go to RUN-TEST/IDLE when finished, returns the data read out of the device (first bit in LSB of the last byte in the buffer) */

/* Comments:


*/
