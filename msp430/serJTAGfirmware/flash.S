/**
Flash manipulating of the host processor.

part of serJTAGfirmware
http://mspgcc.sf.net
chris <cliechti@gmx.net>
*/

#include "hardware.h"

.text

/**
write a single word value to the flash
*/
.global flashWriteDW
        .type flashWriteDW,@function
flashWriteDW:
        mov    #0xA501, &FCTL2          ; select ACLK/2 for flash timing
        push   r2                       ; save settings (interrupt)
        dint                            ; disable all interrupts to not disturb flash programming
        mov    #0xA500, &FCTL3          ; Lock = 0        
        mov    #0xA540, &FCTL1          ; WRT = 1
        mov    r14, 0(r15)              ; programm value
        mov    #0xA500, &FCTL1          ; disable flash writing
        pop    r2                       ; restore IRQ settings
        ret

/**
write a complete block to the flash
  void flashWriteBlock(void *sourceaddr, void *dest_addr, int numbytes);
*/
.global flashWriteBlock
        .type flashWriteBlock,@function
flashWriteBlock:
        tst    r13                      ; size == 0?
        jz     .fwbEx                   ; yes -> nothing to do, exit
        mov    #0xA501, &FCTL2          ; select ACLK/2 for flash timing
        push   r2                       ; save settings (interrupt)
        dint                            ; disable all interrupts to not disturb flash programming
        mov    #0xA500, &FCTL3          ; Lock = 0
        mov    #0xA540, &FCTL1          ; WRT = 1
.fwbL0: mov.b  @r15, 0(r14)             ; programm value
        inc    r14                      ; adjust dst pointer
        inc    r15                      ; adjust src pointer
        dec    r13                      ; count bytes
        jnz    .fwbL0                   ; loop until all bytes are written
        mov    #0xA500, &FCTL1          ; disable flash writing
        pop    r2                       ; restore IRQ settings
.fwbEx: ret

/**
erase the flash bank containing the specified address
*/
.global flashErase
        .type flashErase,@function
flashErase:
        mov    #0xA501, &FCTL2          ; select ACLK/2 for flash timing
        push   r2                       ; save settings (interrupt)
        dint                            ; disable all interrupts to not disturb flash programming
        mov    #0xA500, &FCTL3          ; Lock = 0        
        mov    #0xA502, &FCTL1          ; ERASE = 1
        clr    0(r15)                   ; erase destination flash segment
        mov    #0xA500, &FCTL1          ; disable flash writing
        pop    r2                       ; restore IRQ settings
        ret
