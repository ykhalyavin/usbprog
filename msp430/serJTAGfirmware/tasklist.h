#include "taskhandler.h"

//Entries in the task table of the taskhandler module
//edit you tasks here
//
//part of serJTAGfirmware
//http://mspgcc.sf.net
//chris <cliechti@gmx.net>


// Format of tasktable entries:
// Task function (name only, its called in assembler)
//   #define TASKnn          appTask
//
// to get bitposition on for a task in "taskreg" (see taskhandler.s43)
// the "TASKxx_bits" defines from "taskhandler.h" should be used.

//reference tasks by name not by number, so that moving them becomes simpler

#define TASK_serTask    TASK00_bits
#define TASK_keyHandler TASK01_bits
#define TASK_watch      TASK14_bits
#define TASK_powerDown  TASK15_bits

#define TASK00          serTask
#define TASK01          keyTask
#define TASK02          idleTask
#define TASK03          idleTask
#define TASK04          idleTask
#define TASK05          idleTask
#define TASK06          idleTask
#define TASK07          idleTask
#define TASK08          idleTask
#define TASK09          idleTask
#define TASK10          idleTask
#define TASK11          idleTask
#define TASK12          idleTask
#define TASK12          idleTask
#define TASK13          idleTask
#define TASK14          watchTask
#define TASK15          powerDownTask
