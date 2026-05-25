/**
 * @file    MC.h
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/25
 */

#include "inc_user.h"
#include "MC.h"

void MC_Task(void) {
    uart_printf("[MC] 🚀start MC task\r\n");

    uart_printf("[MC] catch spearhead\r\n");
    uint8_t SpearheadsNum = 6;
    uint8_t SpearheadsCatch = osSemaphoreGetCount(SpearheadsSemHandle);

    while (SpearheadsNum > 0 && SpearheadsCatch > 0) {

        // 到达夹取位置
        CommandSendPoint(MC_SPEARHEAD_X,
                         MC_SPEARHEAD_Y + (float)(6 - SpearheadsNum) * MC_SPEARHEAD_DY,
                         90.0f,
                         1);

        // 视觉检测是否空
        uart_printf("[MC] spearhead %d\r\n", 7-SpearheadsNum);
        Visual_Send(7-SpearheadsNum);
        osEventFlagsWait(KFQEventHandle,
                        EVT_MC_SPEAR_DETECT,
                        osFlagsWaitAny,
                        osWaitForever);
        if (visualData.flag) {
            uart_printf("[MC] | ");
            for (uint8_t i = 0; i < 6; i++) {
                if (i == 6-SpearheadsNum) {
                    uart_printf(" 🟢|");
                }else {
                    uart_printf(" ⚪|");
                }
            }
            uart_printf("\r\n");
            osSemaphoreAcquire(SpearheadsSemHandle, osWaitForever);
            SpearheadsCatch = osSemaphoreGetCount(SpearheadsSemHandle);
            CommandSendCatch(); // 空闲 抓取


            uart_printf("[MC] assemble\r\n");
            CommandSendPoint(MC_ASSEMBLE_X, MC_ASSEMBLE_Y, 90.0f, 1); // 前往对接点

            osEventFlagsWait(KFQEventHandle,
                        EVT_MC_SPEAR_ASSEMBLE,
                        osFlagsWaitAny,
                        osWaitForever);
        }else {
            uart_printf("[MC] | ");
            for (uint8_t i = 0; i < 6; i++) {
                if (i == 6-SpearheadsNum) {
                    uart_printf(" 🔴|");
                }else {
                    uart_printf(" ⚪|");
                }
            }
            uart_printf("\r\n");
        }
        SpearheadsNum--; // 更新剩余端头总数量
    }

    uart_printf("[MC] task done\r\n");
    uart_printf("\r\n");
    DELAY(5000); // 等待R1离开
}