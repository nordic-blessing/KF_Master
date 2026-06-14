/**
 * @file    CmdTask.c
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/10
 */

#include "inc_user.h"
#include "cmdtask.h"

void StartCmdTask(void *argument) {
    osEventFlagsWait(KFQEventHandle,
                     EVT_TASK_START,
                     osFlagsWaitAny | osFlagsNoClear,
                     osWaitForever);

    // ============================= MC =============================
    MC_Task();

    // ============================= MF =============================
    MF_Task();

    // =========================== Arena ============================
    // Arena_Task();

    uart_printf("KF MASTER\r\n");

    while (1) {
        osDelay(1);
    }
}
