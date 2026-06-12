/**
 * @file    MC.h
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.3-只完成中间四个武器头的抓取
 * @date    2026/5/25
 */

#include "inc_user.h"
#include "MC.h"

#define DEBUG_Visual    0

void MC_Task(void) {
    uart_printf("[MC] 🚀start MC task\r\n");

    uart_printf("[MC] catch spearhead\r\n");
    uint8_t SpearheadsNum = 6;
    uint8_t SpearheadsCount = 5; // 武器头总数（此处仅考虑抓取除矛头之外的武器）
    uint8_t SpearheadsCatch = 1; // 目标夹取数量

    while (SpearheadsCount > 1 && SpearheadsCatch > 0) {
        // 机械臂准备动作
        CommandSendCatchPrepare();
        DELAY(800);

        // 底盘运动到达夹取位置
        CommandSendPoint(MC_SPEARHEAD_X,
                         MC_SPEARHEAD_Y + (float)(SpearheadsNum - SpearheadsCount) * MC_SPEARHEAD_DY,
                         90.0f,
                         1);

        // 视觉检测是否空
        uart_printf("[MC] spearhead %d\r\n", 7-SpearheadsCount);
        Visual_Send(7-SpearheadsCount);
#if DEBUG_Visual
        osEventFlagsSet(KFQEventHandle, EVT_MC_SPEAR_DETECT);
        DELAY(500);
#endif
        osEventFlagsWait(KFQEventHandle,
                        EVT_MC_SPEAR_DETECT,
                        osFlagsWaitAny,
                        osWaitForever);

        // 拿到视觉检测结果
        if (visualData.flag) {
            // 目标非空，执行抓取
            uart_printf("[MC] | ");
            for (uint8_t i = 0; i < 6; i++) {
                if (i == 6-SpearheadsCount) {
                    uart_printf(" 🟢|");
                }else {
                    uart_printf(" ⚪|");
                }
            }
            uart_printf("\r\n");
            CommandSendCatch(); // 发布抓取命令，并等待动作完成
            SpearheadsCatch--;

            // 执行对接
            uart_printf("[MC] assemble\r\n");
            CommandSendPoint(MC_ASSEMBLE_X, MC_ASSEMBLE_Y, 90.0f, 1); // 前往对接点

            // 等待视觉通知对接完成
#if DEBUG_Visual
            osEventFlagsSet(KFQEventHandle, EVT_MC_SPEAR_ASSEMBLE);
            DELAY(500);
#endif
            osEventFlagsWait(KFQEventHandle,
                        EVT_MC_SPEAR_ASSEMBLE,
                        osFlagsWaitAny,
                        osWaitForever);

            uart_printf("[MC] assemble done\r\n");
            CommandSendRelease(); // 发布释放命令，并等待动作完成
        }else {
            // 目标位置空，跳过抓取
            uart_printf("[MC] | ");
            for (uint8_t i = 0; i < 6; i++) {
                if (i == 6-SpearheadsCount) {
                    uart_printf(" 🔴|");
                }else {
                    uart_printf(" ⚪|");
                }
            }
            uart_printf("\r\n");
        }
        SpearheadsCount--; // 更新剩余端头总数量
    }

    // 等待视觉通知R1已经离开
#if DEBUG_Visual
    osEventFlagsSet(KFQEventHandle, EVT_MC_LEAVE);
    DELAY(500);
#endif
    osEventFlagsWait(KFQEventHandle,
                EVT_MC_LEAVE,
                osFlagsWaitAny,
                osWaitForever);

    uart_printf("[MC] task done\r\n");
    uart_printf("\r\n");
    DELAY(2000); // 延迟提高容错
}
