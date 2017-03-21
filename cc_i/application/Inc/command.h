#ifndef __COMMAND_H__
#define __COMMAND_H__
#include "cmsis_os.h"

#define PARAMETER_SIZE 100
#define MAX_ARGS      88

typedef struct _MONITOR_COMMAND MONITOR_COMMAND, *PMONITOR_COMMAND;
typedef int     (*PFUNC_COMMAND)(int, char *argv[]);
typedef void    (*PFUNC_HELP)(const unsigned char *, unsigned int);

struct _MONITOR_COMMAND {
    unsigned char   *command;
    PFUNC_COMMAND   pFunc;
};


void cmd_init();

int cmd_help(int argc, char * argv[]);
osMessageQId get_queue_commd(void);
osThreadId get_commd_task_handle(void);

#endif
