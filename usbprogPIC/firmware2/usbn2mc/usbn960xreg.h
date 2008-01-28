/*--------------------------------------------------------------------------*/
/*-----------------------   Configuration Registers   ----------------------*/
/*--------------------------------------------------------------------------*/
#define MCNTRL     0x00                 /*Main control register   */
#define CCONF      0x01                 /*Clk. config. register   */    
#define RID        0x03                 /*Rev. ID      register   */  
#define FAR        0x04                 /*Func address register   */  
#define NFSR       0x05                 /*Node func st register   */   
#define MAEV       0x06                 /*Main event   register   */  
#define MAMSK      0x07                 /*Main mask    register   */  
#define ALTEV      0x08                 /*Alt. event   register   */  
#define ALTMSK     0x09                 /*ALT  mask    register   */  
#define TXEV       0x0A                 /*TX   event   register   */ 
#define TXMSK      0x0B                 /*TX   mask    register   */   
#define RXEV       0x0C                 /*RX   event   register   */ 
#define RXMSK      0x0D                 /*RX   mask    register   */   
#define NAKEV      0x0E                 /*NAK  event   register   */  
#define NAKMSK     0x0F                 /*NAK  mask    register   */  
#define FWEV       0x10                 /*FIFO warning register   */   
#define FWMSK      0x11                 /*FIFO warning mask       */  
#define FNH        0x12                 /*Frame nbr hi register   */   
#define FNL        0x13                 /*Frame nbr lo register   */  
#define DMACNTRL   0x14                 /*DMA  control register   */  
#define DMAEV	   0x15			/*DMA  event   register	  */
#define DMAMSK	   0x16			/*DMA  mask	   register   */
#define DMACNT	   0x18			/*DMA  count   register   */
#define DMAERR	   0x19			/*DMA  error counnt register */ 
#define WKUP	   0x1b
 
#define EPC0       0x20                 /*Endpoint0    register   */   
#define TXD0       0x21                 /*TX   data    register 0 */  
#define TXS0       0x22                 /*TX   status  register 0 */ 
#define TXC0       0x23                 /*TX   command register 0 */  
 
#define RXD0       0x25                 /*RX   data    register 0 */   
#define RXS0       0x26                 /*RX   status  register 0 */   
#define RXC0       0x27                 /*RX   command register 0 */   
 
#define EPC1       0x28                 /*Endpoint1    register   */   
#define TXD1       0x29                 /*TX   data    register 1 */  
#define TXS1       0x2A                 /*TX   status  register 1 */ 
#define TXC1       0x2B                 /*TX   command register 1 */  
 
#define EPC2       0x2C                 /*Endpoint2    register   */   
#define RXD1       0x2D                 /*RX   data    register 1 */   
#define RXS1       0x2E                 /*RX   status  register 1 */   
#define RXC1       0x2F                 /*RX   command register 1 */   
 
#define EPC3       0x30                 /*Endpoint3    register   */   
#define TXD2       0x31                 /*TX   data    register 2 */  
#define TXS2       0x32                 /*TX   status  register 2 */ 
#define TXC2       0x33                 /*TX   command register 2 */  
 
#define EPC4       0x34                 /*Endpoint4    register   */   
#define RXD2       0x35                 /*RX   data    register 2 */   
#define RXS2       0x36                 /*RX   status  register 2 */   
#define RXC2       0x37                 /*RX   command register 2 */   
 
#define EPC5       0x38                 /*Endpoint5    register   */   
#define TXD3       0x39                 /*TX   data    register 3 */  
#define TXS3       0x3A                 /*TX   status  register 3 */ 
#define TXC3       0x3B                 /*TX   command register 3 */  
 
#define EPC6       0x3C                 /*Endpoint6    register   */   
#define RXD3       0x3D                 /*RX   data    register 3 */   
#define RXS3       0x3E                 /*RX   status  register 3 */   
#define RXC3       0x3F                 /*RX   command register 3 */   
 
/*--------------------------------------------------------------------*/
/*-------------------   Configuration Register Bitmaps   -------------*/
/*--------------------------------------------------------------------*/

/*-------------------------------- MCNTRL ------------------------*/
#define SRST       0x01                 /*software reset          */   
#define DBG        0x02                 /*debug mode              */  
#define VGE        0x04                 /*voltage regulator enable*/    
#define NAT        0x08                 /*node attached           */  
#define INT_DIS    0x00                 /*interrupts disabled     */  
#define INT_L_O    0x40                 /*act lo ints, open drain */  
#define INT_H_P    0x80                 /*act hi in	ts, push pull  */  
#define INT_L_P    0xC0                 /*act lo ints, push pull  */  
 
/*---------------------------------- FAR -------------------------*/
#define AD_EN      0x80                 /*address enable          */   
 
/*--------------------------------- EPCX -------------------------*/
#define DEF        0x40                 /*force def. adr (0 only) */   
#define STALL      0x80                 /*force stall handshakes  */  
#define ISO        0x20                 /*set for isochr. (1-3)   */ 
#define EP_EN      0x10                 /*enables endpt.  (1-3)   */  
 
/*--------------------------------- NFSR -------------------------*/
#define RST_ST     0x00                 /*reset       state       */   
#define RSM_ST     0x01                 /*resume      state       */  
#define OPR_ST     0x02                 /*operational state       */  
#define SUS_ST     0x03                 /*suspend     state       */  
 
/*---------------------------- MAEV, MAMSK -----------------------*/
#define WARN       0x01                 /*warning bit has been set*/   
#define ALT        0x02                 /*warning bit has been set*/   
#define TX_EV      0x04                 /*transmit event          */  
#define FRAME      0x08                 /*SOF packet received     */  
#define NAK        0x10                 /*alternate event         */  
#define ULD        0x20                 /*unlock locked detected  */   
#define RX_EV      0x40                 /*receive event           */   
#define INTR_E     0x80                 /*master interrupt enable */  
 
/*---------------------------- ALTEV, ALTMSK ---------------------*/
#define ALT_DMA	   0x04					/*DMA event				  */
#define ALT_EOP    0x08                 /*end of packet           */   
#define ALT_SD3    0x10                 /*3 ms suspend            */  
#define ALT_SD5    0x20                 /*5 ms suspend            */  
#define ALT_RESET  0x40                 /*reset  detected         */  
#define ALT_RESUME 0x80                 /*resume detected         */ 
 
/*---------------------------- TXEV, TXMSK -----------------------*/
#define TX_FIFO0    0x01                /*TX_DONE, FIFO 0         */   
#define TX_FIFO1    0x02                /*TX_DONE, FIFO 1         */   
#define TX_FIFO2    0x04                /*TX_DONE, FIFO 2         */   
#define TX_FIFO3    0x08                /*TX_DONE, FIFO 3         */   
#define TX_UDRN0    0x10                /*TX_URUN, FIFO 0         */   
#define TX_UDRN1    0x20                /*TX_URUN, FIFO 1         */  
#define TX_UDRN2    0x40                /*TX_URUN, FIFO 2         */  
#define TX_UDRN3    0x80                /*TX_URUN, FIFO 3         */  
 
/*---------------------------- RXEV, RXMSK -----------------------*/
#define RX_FIFO0    0x01                /*RX_DONE, FIFO 0         */   
#define RX_FIFO1    0x02                /*RX_DONE, FIFO 1         */   
#define RX_FIFO2    0x04                /*RX_DONE, FIFO 2         */   
#define RX_FIFO3    0x08                /*RX_DONE, FIFO 3         */   
#define RX_OVRN0    0x10                /*RX_OVRN, FIFO 0         */   
#define RX_OVRN1    0x20                /*RX_OVRN, FIFO 1         */  
#define RX_OVRN2    0x40                /*RX_OVRN, FIFO 2         */  
#define RX_OVRN3    0x80                /*RX_OVRN, FIFO 3         */  
 
/*-------------------------- NAKEV, NAKMSK -----------------------*/
#define NAK_IN0	    0x01                /*IN  NAK, FIFO 0         */   
#define NAK_IN1     0x02                /*IN  NAK, FIFO 1         */  
#define NAK_IN2     0x04                /*IN  NAK, FIFO 2         */  
#define NAK_IN3     0x08                /*IN  NAK, FIFO 3         */  
#define NAK_OUT0    0x10                /*OUT NAK, FIFO 0         */  
#define NAK_OUT1    0x20                /*OUT NAK, FIFO 1         */ 
#define NAK_OUT2    0x40                /*OUT NAK, FIFO 2         */ 
#define NAK_OUT3    0x80                /*OUT NAK, FIFO 3         */ 

/*-------------------------- DMAEV, DMAMSK ------------------------*/
#define DMA_DSHLT	0x01		/*DMA Software Halt	   */
#define DMA_DERR	0x02		/*DMA Error		   */
#define DMA_DCNT	0x04		/*DMA Count		   */
#define DMA_DSIZ	0x08		/*DMA Size		   */
#define DMA_NTGL	0x20		/*Next Toggle		   */

/*------------------------------ DMACNTRL -------------------------*/
#define DMA_DMOD	0x08		/*DMA Mode		   */
#define DMA_ADMA	0x10		/*Automatic DMA	   */
#define DMA_DTGL	0x20		/*DMA Toggle		   */
#define IGNRXTGL	0x40		/*Ignore RX Toggle	   */
#define	DMA_DEN		0x80		/*DMA Enable		   */

/*------------------------------ DMAERR ---------------------------*/
#define DMA_AEH		0x80		/*Automatic Error Handling*/ 

/*------------------------------- TXC0 ---------------------------*/
#define TX_EN      0x01			/*transmit enable         */   
#define TX_TOGL    0x04                 /*specifies PID used      */ 
#define FLUSH      0x08                 /*flushes all FIFO data   */
#define IGN_IN     0x10                 /*ignore in  tokens       */   

/*------------------------------- TXCx ---------------------------*/
#define TX_EN      	0x01            /*transmit enable         */   
#define TX_LAST    	0x02            /*specifies last byte     */ 
#define TX_TOGL    	0x04            /*specifies PID used      */ 
#define RFF	  	0x10	 	/*refill FIFO		  */
#define IGN_ISOMSK	0x80		/*ignore ISO Mask	  */

/*------------------------------- TXS0 ---------------------------*/
#define TX_EBYTES  0x1F			/*available empty bytes   */
#define TX_DONE    0x20                 /*transmit done           */   
#define ACK_STAT   0x40                 /*ack status of xmission  */
 
/*------------------------------- RXC0 ---------------------------*/
#define RX_EN      0x01                 /*receive  enable         */   
#define IGN_OUT    0x02                 /*ignore out tokens       */   
#define IGN_SETUP  0x04                 /*ignore setup tokens     */  

/*------------------------------- RXSx ---------------------------*/
#define RX_LAST    0x10                 /*indicates RCOUNT valid  */   
#define RX_TOGL    0x20                 /*last pkt was DATA1 PID  */   
#define SETUP_R    0x40                 /*setup packet received   */  
#define RX_ERR     0x80                 /*last packet had an error*/   



