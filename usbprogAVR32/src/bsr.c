#include "ioport.h"

#include <string.h>
#include "jtag.h"	// for BSR_DW

U32 bsr_in[BSR_DW];
U32 bsr_out[BSR_DW];
U32 bsr_sav[BSR_DW];

iop_t led_sys = {
	"PA16",				// name
	"LED_SYS", 			// alias
	"N2",				// pin
	PA16,				// data bit
	PA16/32,			// data word
	1UL<<(PA16%32),		// data bit mask
	(PA16+1),			// controller bit
	(PA16+1)/32,		// controller word
	1UL<<((PA16+1)%32),	// controller bit mask
	IOP_OUT, 			// bit io
	IOP_HI          	// bit level
};
iop_t led_a = { "PA19",	"LED_A", "P2", PA19, PA19/32, 1UL<<(PA19%32),
				(PA19+1), (PA19+1)/32, 1UL<<((PA19+1)%32), IOP_OUT, IOP_HI };
iop_t led_b = { "PE19", "LED_B", "T11", PE19, PE19/32, 1UL<<(PE19%32),
				(PE19+1), (PE19+1)/32, 1UL<<((PE19+1)%32), IOP_OUT, IOP_HI };
iop_t J15   = { "PB30", "BOOT SELECT", "P13", PB30, PB30/32, 1UL<<(PB30%32),
				(PB30+1), (PB30+1)/32, 1UL<<((PB30+1)%32), IOP_IN, IOP_HI };

/* External Static Memory Bus */
iop_t addr[ADDR_BUS_WIDTH] = {
	{"PX16", "A0",  "N16", PX16, PX16/32, 1UL<<(PX16%32), (PX16+1), (PX16+1)/32, 1UL<<((PX16+1)%32), IOP_OUT, IOP_LO },
	{"PX17", "A1",  "N15", PX17, PX17/32, 1UL<<(PX17%32), (PX17+1), (PX17+1)/32, 1UL<<((PX17+1)%32), IOP_OUT, IOP_LO },
	{"PX18", "A2",  "L13", PX18, PX18/32, 1UL<<(PX18%32), (PX18+1), (PX18+1)/32, 1UL<<((PX18+1)%32), IOP_OUT, IOP_LO },
	{"PX19", "A3",  "L16", PX19, PX19/32, 1UL<<(PX19%32), (PX19+1), (PX19+1)/32, 1UL<<((PX19+1)%32), IOP_OUT, IOP_LO },
	{"PX20", "A4",  "L14", PX20, PX20/32, 1UL<<(PX20%32), (PX20+1), (PX20+1)/32, 1UL<<((PX20+1)%32), IOP_OUT, IOP_LO },
	{"PX21", "A5",  "L15", PX21, PX21/32, 1UL<<(PX21%32), (PX21+1), (PX21+1)/32, 1UL<<((PX21+1)%32), IOP_OUT, IOP_LO },
	{"PX22", "A6",  "K11", PX22, PX22/32, 1UL<<(PX22%32), (PX22+1), (PX22+1)/32, 1UL<<((PX22+1)%32), IOP_OUT, IOP_LO },
	{"PX23", "A7",  "K12", PX23, PX23/32, 1UL<<(PX23%32), (PX23+1), (PX23+1)/32, 1UL<<((PX23+1)%32), IOP_OUT, IOP_LO },
	{"PX24", "A8",  "K13", PX24, PX24/32, 1UL<<(PX24%32), (PX24+1), (PX24+1)/32, 1UL<<((PX24+1)%32), IOP_OUT, IOP_LO },
	{"PX25", "A9",  "K16", PX25, PX25/32, 1UL<<(PX25%32), (PX25+1), (PX25+1)/32, 1UL<<((PX25+1)%32), IOP_OUT, IOP_LO },
	{"PX26", "A10", "K14", PX26, PX26/32, 1UL<<(PX26%32), (PX26+1), (PX26+1)/32, 1UL<<((PX26+1)%32), IOP_OUT, IOP_LO },
	{"PX27", "A11", "J10", PX27, PX27/32, 1UL<<(PX27%32), (PX27+1), (PX27+1)/32, 1UL<<((PX27+1)%32), IOP_OUT, IOP_LO },
	{"PX28", "A12", "J11", PX28, PX28/32, 1UL<<(PX28%32), (PX28+1), (PX28+1)/32, 1UL<<((PX28+1)%32), IOP_OUT, IOP_LO },
	{"PX29", "A13", "J12", PX29, PX29/32, 1UL<<(PX29%32), (PX29+1), (PX29+1)/32, 1UL<<((PX29+1)%32), IOP_OUT, IOP_LO },
	{"PX30", "A14", "J13", PX30, PX30/32, 1UL<<(PX30%32), (PX30+1), (PX30+1)/32, 1UL<<((PX30+1)%32), IOP_OUT, IOP_LO },
	{"PX31", "A15", "J16", PX31, PX31/32, 1UL<<(PX31%32), (PX31+1), (PX31+1)/32, 1UL<<((PX31+1)%32), IOP_OUT, IOP_LO },
	{"PX32", "A16", "F1",  PX32, PX32/32, 1UL<<(PX32%32), (PX32+1), (PX32+1)/32, 1UL<<((PX32+1)%32), IOP_OUT, IOP_LO },
	{"PX33", "A17", "F3",  PX33, PX33/32, 1UL<<(PX33%32), (PX33+1), (PX33+1)/32, 1UL<<((PX33+1)%32), IOP_OUT, IOP_LO },
	{"PX34", "A18", "M13", PX34, PX34/32, 1UL<<(PX34%32), (PX34+1), (PX34+1)/32, 1UL<<((PX34+1)%32), IOP_OUT, IOP_LO },
	{"PX35", "A19", "M16", PX35, PX35/32, 1UL<<(PX35%32), (PX35+1), (PX35+1)/32, 1UL<<((PX35+1)%32), IOP_OUT, IOP_LO },
	{"PX36", "A20", "M14", PX36, PX36/32, 1UL<<(PX36%32), (PX36+1), (PX36+1)/32, 1UL<<((PX36+1)%32), IOP_OUT, IOP_LO },
	{"PX37", "A21", "M15", PX37, PX37/32, 1UL<<(PX37%32), (PX37+1), (PX37+1)/32, 1UL<<((PX37+1)%32), IOP_OUT, IOP_LO },
	{"PX38", "A22", "L12", PX38, PX38/32, 1UL<<(PX38%32), (PX38+1), (PX38+1)/32, 1UL<<((PX38+1)%32), IOP_OUT, IOP_LO },
};

iop_t data[DATA_BUS_WIDTH] = {
	{"PX00", "D0",  "F2",  PX00, PX00/32, 1UL<<(PX00%32), (PX00+1), (PX00+1)/32, 1UL<<((PX00+1)%32), IOP_IN, IOP_HI },
	{"PX01", "D1",  "G6",  PX01, PX01/32, 1UL<<(PX01%32), (PX01+1), (PX01+1)/32, 1UL<<((PX01+1)%32), IOP_IN, IOP_HI },
	{"PX02", "D2",  "G5",  PX02, PX02/32, 1UL<<(PX02%32), (PX02+1), (PX02+1)/32, 1UL<<((PX02+1)%32), IOP_IN, IOP_HI },
	{"PX03", "D3",  "G4",  PX03, PX03/32, 1UL<<(PX03%32), (PX03+1), (PX03+1)/32, 1UL<<((PX03+1)%32), IOP_IN, IOP_HI },
	{"PX04", "D4",  "G1",  PX04, PX04/32, 1UL<<(PX04%32), (PX04+1), (PX04+1)/32, 1UL<<((PX04+1)%32), IOP_IN, IOP_HI },
	{"PX05", "D5",  "G3",  PX05, PX05/32, 1UL<<(PX05%32), (PX05+1), (PX05+1)/32, 1UL<<((PX05+1)%32), IOP_IN, IOP_HI },
	{"PX06", "D6",  "N12", PX06, PX06/32, 1UL<<(PX06%32), (PX06+1), (PX06+1)/32, 1UL<<((PX06+1)%32), IOP_IN, IOP_HI },
	{"PX07", "D7",  "R12", PX07, PX07/32, 1UL<<(PX07%32), (PX07+1), (PX07+1)/32, 1UL<<((PX07+1)%32), IOP_IN, IOP_HI },
	{"PX08", "D8",  "M12", PX08, PX08/32, 1UL<<(PX08%32), (PX08+1), (PX08+1)/32, 1UL<<((PX08+1)%32), IOP_IN, IOP_HI },
	{"PX09", "D9",  "P12", PX09, PX09/32, 1UL<<(PX09%32), (PX09+1), (PX09+1)/32, 1UL<<((PX09+1)%32), IOP_IN, IOP_HI },
	{"PX10", "D10", "T12", PX10, PX10/32, 1UL<<(PX10%32), (PX10+1), (PX10+1)/32, 1UL<<((PX10+1)%32), IOP_IN, IOP_HI },
	{"PX11", "D11", "N13", PX11, PX11/32, 1UL<<(PX11%32), (PX11+1), (PX11+1)/32, 1UL<<((PX11+1)%32), IOP_IN, IOP_HI },
	{"PX12", "D12", "T13", PX12, PX12/32, 1UL<<(PX12%32), (PX12+1), (PX12+1)/32, 1UL<<((PX12+1)%32), IOP_IN, IOP_HI },
	{"PX13", "D13", "P15", PX13, PX13/32, 1UL<<(PX13%32), (PX13+1), (PX13+1)/32, 1UL<<((PX13+1)%32), IOP_IN, IOP_HI },
	{"PX14", "D14", "P16", PX14, PX14/32, 1UL<<(PX14%32), (PX14+1), (PX14+1)/32, 1UL<<((PX14+1)%32), IOP_IN, IOP_HI },
	{"PX15", "D15", "N14", PX15, PX15/32, 1UL<<(PX15%32), (PX15+1), (PX15+1)/32, 1UL<<((PX15+1)%32), IOP_IN, IOP_HI },
};

iop_t ncs[NCS_NUM] ={
	{"PX39", "NCS0","H15", PX39, PX39/32, 1UL<<(PX39%32), (PX39+1), (PX39+1)/32, 1UL<<((PX39+1)%32), IOP_OUT, IOP_HI },
	{"PX40", "NCS1","F15", PX40, PX40/32, 1UL<<(PX40%32), (PX40+1), (PX40+1)/32, 1UL<<((PX40+1)%32), IOP_OUT, IOP_HI },
	{"PX41", "TP18","P10", PX41, PX41/32, 1UL<<(PX41%32), (PX41+1), (PX41+1)/32, 1UL<<((PX41+1)%32), IOP_OUT, IOP_HI },
};

iop_t nrd =
	{"PX42", "NRD", "F13", PX42, PX42/32, 1UL<<(PX42%32), (PX42+1), (PX42+1)/32, 1UL<<((PX42+1)%32), IOP_OUT, IOP_HI };

iop_t nwe[NWE_NUM] ={
	{"PX43", "NWE0","F14", PX43, PX43/32, 1UL<<(PX43%32), (PX43+1), (PX43+1)/32, 1UL<<((PX43+1)%32), IOP_OUT, IOP_HI },
	{"PX44", "NWE1","F12", PX44, PX44/32, 1UL<<(PX44%32), (PX44+1), (PX44+1)/32, 1UL<<((PX44+1)%32), IOP_OUT, IOP_HI },
	{"PX45", "TP19","F16", PX45, PX45/32, 1UL<<(PX45%32), (PX45+1), (PX45+1)/32, 1UL<<((PX45+1)%32), IOP_OUT, IOP_HI },
};


void iop_get(const U32* bsr, iop_t* iop)
{
	iop->io = (bsr[iop->cw] & iop->cmask) ? 1: 0;
	iop->d  = (bsr[iop->dw] & iop->dmask) ? 1: 0;
}


void iop_set(U32* bsr, iop_t iop)
{
	if (iop.io)
		bsr[iop.cw] |= iop.cmask;	// set bit
	else
		bsr[iop.cw] &= ~iop.cmask;	// clear bir

	if (iop.d)
		bsr[iop.dw] |= iop.dmask;	// set bit
	else
		bsr[iop.dw] &= ~iop.dmask;	// clear bir
}


void iop_set_iod(U32* bsr, iop_t* iop, int io, int d)
{
	iop->io = io;
	if (io) {
		bsr[iop->cw] |= iop->cmask;	// set bit
	} else {
		bsr[iop->cw] &= ~iop->cmask;// clear bir
	}

	iop->d = d;
	if (d) {
		bsr[iop->dw] |= iop->dmask;	// set bit
	} else {
		bsr[iop->dw] &= ~iop->dmask;// clear bir
	}
}

void bus_set_addr(U32* bsr, U32 address)
{
	int i;
	U32 bit_mask;

	for (i = 0, bit_mask = 1; i < ADDR_BUS_WIDTH; i++, bit_mask <<= 1) {
		addr[i].io = IOP_OUT;
		addr[i].d  = (address & bit_mask) ? 1 : 0;
		iop_set(bsr, addr[i]);
		i=i;
	}
}


void bus_set_data_in(U32* bsr)
{
	int i;

	for (i = 0; i < DATA_BUS_WIDTH; i++) {
		data[i].io = 1;
		data[i].d  = 0;
		iop_set(bsr, data[i]);
		i=i;
	}
}


U16 bus_get_data(U32* bsr)
{
	int i;
	U16 bit_mask, value;

	for (i = 0, bit_mask = 1; i < DATA_BUS_WIDTH; i++, bit_mask <<= 1) {
		iop_get(bsr, &data[i]);
		if (data[i].d)
			value |= bit_mask;
		else
			value &= ~bit_mask;
	}

	return value;
}


void bus_set_data_out(U32* bsr, U16 value)
{
	int i;
	U16 bit_mask;

	for (i = 0, bit_mask = 1; i < DATA_BUS_WIDTH; i++, bit_mask <<= 1) {
		data[i].io = 0;
		data[i].d  = (value & bit_mask) ? 1 : 0;
		iop_set(bsr, data[i]);
	}
}


void set_addr(U32 address)
{
	int i;
	U32 bit_mask;

	for (i = 0, bit_mask = 1; i < 23; i++, bit_mask <<= 1) {
		addr[i].io = IOP_OUT;
		addr[i].d  = (address & bit_mask) ? 1 : 0;
		iop_set(bsr_in, addr[i]);
		i=i;
	}
}


void set_data_in(void)
{
	int i;

	for (i = 0; i < 16; i++) {
		data[i].io = 1;
		data[i].d  = 0;
		iop_set(bsr_in, data[i]);
		i=i;
	}
}


U16 get_data(void)
{
	int i;
	U16 bit_mask, value;

	for (i = 0, bit_mask = 1; i < 16; i++, bit_mask <<= 1) {
		iop_get(bsr_out, &data[i]);
		if (data[i].d)
			value |= bit_mask;
		else
			value &= ~bit_mask;
	}

	return value;
}


void set_data(U16 value)
{
	int i;
	U16 bit_mask;

	for (i = 0, bit_mask = 1; i < 16; i++, bit_mask <<= 1) {
		data[i].io = 0;
		data[i].d  = (value & bit_mask) ? 1 : 0;
		iop_set(bsr_in, data[i]);
	}
}



