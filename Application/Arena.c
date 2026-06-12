/**
 * @file    Arena.h
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/25
 */

#include "inc_user.h"
#include "Arena.h"

uint8_t KFS_left = 2;

void Arena_Task(void) {
    uart_printf("[Arena] 🚀start Arena task\r\n");

    // 斜坡下方
    uart_printf("[MF] go ramp down\r\n");
    CommandSendPoint(RAMP_DOWN_X, RAMP_DOWN_Y, 0.0f, 1);

    // 上坡
    uart_printf("[Arena] enter Arena\r\n");
    CommandSendPoint(RAMP_UP_X, RAMP_UP_Y, 0.0f, 1);

    // 等待音响设备
    uart_printf("[Arena] Arena wait\r\n");
    CommandSendPoint(ARENA_WAIT_X, ARENA_WAIT_Y, 0.0f, 1);

    osEventFlagsWait(KFQEventHandle,
                    EVT_ARENA_VISUAL,
                    osFlagsWaitAny,
                    osWaitForever);

    // 第一次走点
    uart_printf("[Arena] put %d\r\n", visualData.data[1]);
    switch (visualData.data[1]) {
        case 1:
            CommandSendPoint(TTT_RACK_1_X, TTT_RACK_1_Y, 0.0f, 1);
        break;
        case 2:
            CommandSendPoint(TTT_RACK_2_X, TTT_RACK_2_Y, 0.0f, 1);
        break;
        case 3:
            CommandSendPoint(TTT_RACK_3_X, TTT_RACK_3_Y, 0.0f, 1);
        break;
        default:
            break;
    }

    // 放第一个KFS
    uart_printf("[Arena] put 1\r\n");
    CommandSendPut(1);
    uart_printf("[Arena] put 1 done\r\n");

    osEventFlagsWait(KFQEventHandle,
                    EVT_ARENA_VISUAL,
                    osFlagsWaitAny,
                    osWaitForever);

    // 上车通知


    uart_printf("[Arena] task done\r\n");
    uart_printf("\r\n");
}
