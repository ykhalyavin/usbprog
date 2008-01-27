#ifndef __AVR32_BSR_H__
#define __AVR32_BSR_H__

#include <stdint.h>
#include "../include/basic.h"

#define IO_IN				1
#define IO_OUT				0

/* External Static Memory Bus */
#define ADDR_BUS_WIDTH		23
#define DATA_BUS_WIDTH		16
#define NCS_NUM				3
#define NWE_NUM				3

#define IOP_NCS0			0
#define IOP_NCS1			1
#define IOP_NCS2			2
#define IOP_NCS_NONE		3

#define IOP_LED_SYS			0
#define IOP_LED_A			1
#define IOP_LED_B			2

#define IOP_LED_ON			LO
#define IOP_LED_OFF			HI


#define BSR_SIZE			388									// bits = 48.5 bytes
#define BSR_BUF_SIZE		(BSR_SIZE/8+(BSR_SIZE%8 ? 1:0))		// bytes

typedef struct {
	uint8_t	index;	// data buffer index
	BUF_T	bit;	// data buffer bit mask
} IOP_T;


/* Routines */
void avr32_iop_reset(BUF_T* bsr);

/* Bit routines */
uint8_t avr32_iop_get(const BUF_T* bsr, IOP_T iop);
void    avr32_iop_set(BUF_T* bsr, IOP_T iop, uint8_t io, uint8_t d);

void avr32_iop_led(BUF_T* bsr, uint8_t which, uint8_t status);
uint8_t avr32_iop_get_j15(const BUF_T* bsr);

/* External memory bus routines */
void avr32_iop_cs(BUF_T* bsr, uint8_t which);
void avr32_iop_set_addr(BUF_T* bsr, uint32_t address);
void avr32_iop_set_nwe0(BUF_T* bsr, uint8_t value);
void avr32_iop_set_nrd(BUF_T* bsr, uint8_t value);
void avr32_iop_set_data_out(BUF_T* bsr, uint16_t value);
void avr32_iop_set_data_in(BUF_T* bsr);
uint16_t avr32_iop_get_data(const BUF_T* bsr);


/* IO port in BSR */
#define PE16 0
#define PE15 2
#define PE14 4
#define PE13 6
#define PE12 8
#define PE11 10
#define PE10 12
#define PE09 14
#define PE08 16
#define PE07 18
#define PE06 20
#define PE05 22
#define PE04 24
#define PE03 26
#define PE02 28
#define PE01 30
#define PE00 32
#define PC31 34
#define PC30 36
#define PC29 38
#define PC28 40
#define PC27 42
#define PC26 44
#define PC25 46
#define PC24 48
#define PC23 50
#define PC22 52
#define PC21 54
#define PC20 56
#define PC19 58
#define PA07 60
#define PA06 62
#define PC18 64
#define PB23 66
#define PB22 68
#define PB21 70
#define PB20 72
#define PB19 74
#define PB18 76
#define PB17 78
#define PB16 80
#define PB15 82
#define PB14 84
#define PB13 86
#define PB12 88
#define PB11 90
#define PB10 92
#define PC17 94
#define PC16 96
#define PB09 98
#define PB08 100
#define PB07 102
#define PB06 104
#define PB05 106
#define PB04 108
#define PB03 110
#define PB02 112
#define PB01 114
#define PB00 116
#define PX46 118
#define PX45 120
#define PX44 122
#define PX43 124
#define PX42 126
#define PX40 128
#define PC15 130
#define PC14 132
#define PC13 134
#define PC12 136
#define PC11 138
#define PC10 140
#define PC09 142
#define PC08 144
#define PC07 146
#define PX39 148
#define PE26 150
#define PC06 152
#define PC05 154
#define PX31 156
#define PX30 158
#define PX29 160
#define PX28 162
#define PX27 164
#define PX26 166
#define PX25 168
#define PX24 170
#define PX23 172
#define PX22 174
#define PX21 176
#define PX20 178
#define PX19 180
#define PX18 182
#define PX38 184
#define PX37 186
#define PX36 188
#define PX35 190
#define PX34 192
#define PX17 194
#define PX16 196
#define PX15 198
#define PX14 200
#define PX13 212
#define PC04 204
#define PC03 206
#define PC02 208
#define PC01 210
#define PC00 202
#define PX12 214
#define PB30 216
#define PB29 218
#define PX11 220
#define PX10 222
#define PX09 224
#define PX08 226
#define PX07 228
#define PX06 230
#define PE19 232
#define PE20 234
#define PE21 236
#define PE22 238
#define PE23 240
#define PE24 242
#define PE25 244
#define PX41 246
#define PX52 248
#define PX53 250
#define PB28 252
#define PB27 254
#define PB26 256
#define PA31 258
#define PA30 260
#define PA29 262
#define PA28 264
#define PA27 266
#define PA26 268
#define PA25 270
#define PD17 272
#define PD16 274
#define PD15 276
#define PD14 278
#define PD13 280
#define PD12 282
#define PD11 284
#define PA24 286
#define PA23 288
#define PD10 290
#define PA22 292
#define PA21 294
#define PA20 296
#define PA19 298
#define PA18 300
#define PA17 302
#define PA16 304
#define PA15 306
#define PA14 308
#define PA13 310
#define PA12 312
#define PA11 314
#define PA10 316
#define PA09 318
#define PA08 320
#define PB25 322
#define PB24 324
#define PA05 326
#define PA04 328
#define PA03 330
#define PA02 332
#define PA01 334
#define PA00 336
#define PD09 338
#define PD08 340
#define PD07 342
#define PD06 344
#define PD05 346
#define PD04 348
#define PD03 350
#define PX05 352
#define PX04 354
#define PX03 356
#define PX02 358
#define PX01 360
#define PX00 362
#define PX33 364
#define PX32 366
#define PX51 368
#define PX50 370
#define PX49 372
#define PX48 374
#define PX47 376
#define PE18 378
#define PE17 380
#define PD02 382
#define PD01 384
#define PD00 386

#endif  /* __AVR32_BSR_H__ */
