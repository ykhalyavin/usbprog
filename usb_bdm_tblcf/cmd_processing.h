unsigned char command_exec(void);

typedef enum {
  CF_BDM=TARGET_TYPE_CF_BDM,
  JTAG=TARGET_TYPE_JTAG
} target_type_e;

typedef enum {
  NO_RESET_ACTIVITY=0,
  RESET_DETECTED=1
} reset_e;

typedef struct {
  unsigned char target_type:3;  /* target_type_e */
  unsigned char reset:1;        /* reset_e */
} cable_status_t;

#pragma DATA_SEG Z_RAM
extern cable_status_t near cable_status;
#pragma DATA_SEG DEFAULT