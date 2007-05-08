/* BDM signals */

#ifdef DEBUG  /* when debugging is required, DSI is moved to PTA7 */
  #define DSI_OUT           PTA_PTA7
  #define DSI_OUT_MASK      PTA_PTA7_MASK
  #define DSI_OUT_BITNUM    PTA_PTA7_BITNUM
#else  				/* DSI is on normally on PTA0 (easier PCB layout) */
  #define DSI_OUT           PTA_PTA0
  #define DSI_OUT_MASK      PTA_PTA0_MASK
  #define DSI_OUT_BITNUM    PTA_PTA0_BITNUM
#endif

#define RSTI_OUT            PTA_PTA1
#define RSTI_OUT_MASK       PTA_PTA1_MASK
#define RSTI_OUT_BITNUM     PTA_PTA1_BITNUM
#define RSTI_DIRECTION      DDRA_DDRA1

#define TA_OUT              PTA_PTA2
#define TA_OUT_MASK         PTA_PTA2_MASK
#define TA_OUT_BITNUM       PTA_PTA2_BITNUM
#define TA_DIRECTION        DDRA_DDRA2

#define DSCLK_OUT           PTA_PTA4
#define DSCLK_OUT_MASK      PTA_PTA4_MASK
#define DSCLK_OUT_BITNUM    PTA_PTA4_BITNUM
#define DSCLK_OUT_PORT      PTA

#define BKPT_OUT            PTA_PTA5
#define BKPT_OUT_MASK       PTA_PTA5_MASK
#define BKPT_OUT_BITNUM     PTA_PTA5_BITNUM

#define RSTO_IN             PTA_PTA6
#define RSTO_IN_MASK        PTA_PTA6_MASK
#define RSTO_IN_BITNUM      PTA_PTA6_BITNUM

#define DSO_IN              PTC_PTC0
#define DSO_IN_MASK         PTC_PTC0_MASK
#define DSO_IN_BITNUM       PTC_PTC0_BITNUM
#define DSO_IN_PORT         PTC

/* JTAG signals */

#define TDI_OUT             DSI_OUT
#define TDI_OUT_MASK        DSI_OUT_MASK
#define TDI_OUT_BITNUM			DSI_OUT_BITNUM

#define TCLK_OUT            PTA_PTA3
#define TCLK_OUT_MASK       PTA_PTA3_MASK
#define TCLK_OUT_BITNUM     PTA_PTA3_BITNUM

#define TMS_OUT             BKPT_OUT
#define TMS_OUT_MASK        BKPT_OUT_MASK       
#define TMS_OUT_BITNUM      BKPT_OUT_BITNUM

#define TRST_OUT            DSCLK_OUT
#define TRST_OUT_MASK       DSCLK_OUT_MASK
#define TRST_OUT_BITNUM     DSCLK_OUT_PORT

#define TDO_IN              DSO_IN
#define TDO_IN_MASK         DSO_IN_MASK
#define TDO_IN_BITNUM 			DSO_IN_BITNUM

/* BDM commands */
#define BDMCF_RETRY         20   /* how many times to retry before giving up */
#define BDMCF_CMD_NOP       0x0000
#define BDMCF_CMD_GO        0x0C00
#define BDMCF_CMD_RDMREG    0x2D80
#define BDMCF_CMD_WDMREG    0x2C80
#define BDMCF_CMD_RCREG     0x2980
#define BDMCF_CMD_WCREG     0x2880
#define BDMCF_CMD_RAREG     0x2180
#define BDMCF_CMD_WAREG     0x2080
#define BDMCF_CMD_READ8     0x1900
#define BDMCF_CMD_READ16    0x1940
#define BDMCF_CMD_READ32    0x1980
#define BDMCF_CMD_WRITE8    0x1800
#define BDMCF_CMD_WRITE16   0x1840
#define BDMCF_CMD_WRITE32   0x1880
#define BDMCF_CMD_DUMP8     0x1D00
#define BDMCF_CMD_DUMP16    0x1D40
#define BDMCF_CMD_DUMP32    0x1D80
#define BDMCF_CMD_FILL8     0x1C00
#define BDMCF_CMD_FILL16    0x1C40
#define BDMCF_CMD_FILL32    0x1C80

#ifdef INVERT
  #define BDMCF_IDLE        (DSI_OUT_MASK+TCLK_OUT_MASK+DSCLK_OUT_MASK)
  #define JTAG_IDLE         (TDI_OUT_MASK+TCLK_OUT_MASK)
  #define TRST_SET()        TRST_OUT = 0
  #define TRST_RESET()      TRST_OUT = 1
  #define TMS_SET()         TMS_OUT = 0
  #define TMS_RESET()       TMS_OUT = 1
  #define TCLK_SET()        TCLK_OUT = 0
  #define TCLK_RESET()      TCLK_OUT = 1
  #define TDI_OUT_SET()     TDI_OUT = 0
  #define TDI_OUT_RESET()   TDI_OUT = 1
  #define TDO_IN_SET        TDO_IN==0
#else
  #define BDMCF_IDLE        (BKPT_OUT_MASK)
  #define JTAG_IDLE         (TRST_OUT_MASK+TMS_OUT_MASK)
  #define TRST_SET()        TRST_OUT = 1
  #define TRST_RESET()      TRST_OUT = 0
  #define TMS_SET()         TMS_OUT = 1
  #define TMS_RESET()       TMS_OUT = 0
  #define TCLK_SET()        TCLK_OUT = 1
  #define TCLK_RESET()      TCLK_OUT = 0
  #define TDI_OUT_SET()     TDI_OUT = 1
  #define TDI_OUT_RESET()   TDI_OUT = 0
  #define TDO_IN_SET        TDO_IN==1
#endif

/* prototypes */
void bdmcf_init(void);
unsigned char bdmcf_tx_msg(unsigned int data);
unsigned char bdmcf_rx_msg(unsigned char *data);
unsigned char bdmcf_txrx_msg(unsigned char *data);
unsigned char bdmcf_resync(void);
void bdmcf_halt(void);
void bdmcf_reset(unsigned char bkpt);
void bdmcf_tx(unsigned char count, unsigned char *data);
unsigned char bdmcf_complete_chk(unsigned int next_cmd);
unsigned char bdmcf_complete_chk_rx(void);
unsigned char bdmcf_tx_msg_half_rx(unsigned int data);
unsigned char bdmcf_rx(unsigned char count, unsigned char *data);
unsigned char bdmcf_rxtx(unsigned char count, unsigned char *data, unsigned int next_cmd);
interrupt void rsto_detect(void);
void jtag_transition_shift(unsigned char mode);
void jtag_init(void);
void jtag_write(unsigned char tap_transition, unsigned char bit_count, unsigned char * datap);
void jtag_read(unsigned char tap_transition, unsigned char bit_count, unsigned char * datap);
void jtag_transition_reset(void);
void bdmcf_ta(unsigned char time_10us);

/* prototypes for the Rx and Tx functions */
void bdmcf_tx8_1(unsigned char data);
unsigned char bdmcf_rx8_1(void);
unsigned char bdmcf_txrx8_1(unsigned char data);
unsigned char bdmcf_txrx_start_1(void);

#ifdef MULTIPLE_SPEEDS
  /* until more than one set of rx/tx functions is needed the speed of operation can be improved by not using the pointers */
  #pragma DATA_SEG Z_RAM

  /* pointers to Tx & Rx functions */
  extern unsigned char near (*bdmcf_rx8_ptr)(void);
  extern void near (*bdmcf_tx8_ptr)(unsigned char);
  extern unsigned char near (*bdmcf_txrx8_ptr)(unsigned char);
  extern unsigned char near (*bdmcf_txrx_start_ptr)(void);

  #pragma DATA_SEG DEFAULT

  /* tables with pointers to Tx & Rx functions */
  extern unsigned char (* const bdmcf_rx8_ptrs[])(void);
  extern void (* const bdmcf_tx8_ptrs[])(unsigned char);
  extern unsigned char (* const bdmcf_txrx8_ptrs[])(unsigned char);
  extern unsigned char (* const bdmcf_txrx_start_ptrs[])(void);
#else
  void bdmcf_rx8_ptr(unsigned char data);
  unsigned char bdmcf_tx8_ptr(void);
  unsigned char bdmcf_txrx8_ptr(unsigned char data);
  unsigned char bdmcf_txrx_start_ptr(void);

  #define bdmcf_rx8_ptr bdmcf_rx8_1
  #define bdmcf_tx8_ptr bdmcf_tx8_1
  #define bdmcf_txrx8_ptr bdmcf_txrx8_1
  #define bdmcf_txrx_start_ptr bdmcf_txrx_start_1
#endif

