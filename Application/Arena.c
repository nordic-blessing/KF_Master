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

    uart_printf("[MF] go ramp down\r\n");
    CommandSendPoint(RAMP_DOWN_X, RAMP_DOWN_Y, 0.0f, 1);

    uart_printf("[Arena] enter Arena\r\n");
    CommandSendPoint(RAMP_UP_X, RAMP_UP_Y, 0.0f, 1);

    for (;KFS_left > 0; KFS_left--) {
        CommandSendPoint(ARENA_WAIT_X, ARENA_WAIT_Y, 0.0f, 1);
        uart_printf("[Arena] wait\r\n");

        // 视觉识别
        Visual_Send(MCU_CMD_Arena);
        osEventFlagsWait(KFQEventHandle,
                    EVT_ARENA_VISUAL,
                    osFlagsWaitAny,
                    osWaitForever);

        uart_printf("[Arena] put %d\r\n", visualData.num);
        switch (visualData.num) {
            case 1:
            case 2:
            case 3: {
                // 上车逻辑
                uart_printf("[Arena] up to R1\r\n]");

                // 放置KFS
                osEventFlagsWait(KFQEventHandle,
                            EVT_ARENA_KFS_PUT,
                            osFlagsWaitAny,
                            osWaitForever);
                CommandSendPut();
            }
            break;

            case 4: {
                CommandSendPoint(TTT_RACK_1_X, TTT_RACK_1_Y, 0.0f, 1);
                CommandSendPut();
            }
            break;

            case 5: {
                CommandSendPoint(TTT_RACK_2_X, TTT_RACK_2_Y, 0.0f, 1);
                CommandSendPut();
            }
            break;

            case 6: {
                CommandSendPoint(TTT_RACK_3_X, TTT_RACK_3_Y, 0.0f, 1);
                CommandSendPut();
            }
            break;
            default:
                break;
        }
        uart_printf("[Arena] put done\r\n");
        KFS_left --;
    }

    uart_printf("[Arena] task done\r\n");
    uart_printf("\r\n");
}
