/* conditional compilation for DSI_OUT on PTA7 instead of PTA0: DEBUG */
#undef DEBUG
//#define DEBUG

/* conditional compilation for inverting interface: INVERT */
//#undef INVERT
#define INVERT

/* conditional compilation for multiple Tx/Rx routines (different speeds): MULTIPLE_SPEEDS */
#undef MULTIPLE_SPEEDS

/* conditional compilation to check command completion after executing commands which only write and do not read results: CMD_COMPLETE_CHECK */
#define CMD_COMPLETE_CHECK

/* conditional compilation to include support for measurement of required stack size: STACK_SIZE_EVALUATION */
#define STACK_SIZE_EVALUATION
