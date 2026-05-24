/**
 * @file    CmdTask.c
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/10
 */

#include "inc_user.h"
#include "cmdtask.h"

void CommandSendPoint(float x, float y, float yaw, uint8_t start);
void CommandSendCatch(void);
void CommandSendGrab(uint8_t grab_flag);
void CommandSendLift(uint8_t lift_flag);
void CommandSendPut(void);

void StartCmdTask(void *argument) {
    osEventFlagsWait(KFQEventHandle,
                     EVT_TASK_START,
                     osFlagsWaitAny | osFlagsNoClear,
                     osWaitForever);

    // ============================= MC =============================
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
        Visual_Send(VISUAL_CMD_Spearhead);
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

    DELAY(5000); // 等待R1离开

    uart_printf("\r\n");

    // ============================= MF =============================
    uart_printf("[MF] 🚀start MF task\r\n");

    float yaw = 0.0f;
    float x = 3.000f;
    float y = MC_ASSEMBLE_Y;
    for (uint8_t i = 0; i < Action_Buffer_Size; i++) {
        switch (Action_Buffer[i].move) {
            case STAY:
            case FORWARD:
                yaw = 0.0f;
            break;
            case LEFT:
                yaw = 90.0f;
            break;
            case RIGHT:
                yaw = -90.0f;
            break;
            case BACKWARD:
                yaw = 180.0f;
            break;
            default:
                break;
        }
        CommandSendPoint(x, y, yaw, 1);
        uart_printf("[MF] yaw %2f\r\n", yaw);

        switch (Action_Buffer[i].lift) {
            case LEVEL: {
                uart_printf("[MF] LEVEL\r\n");
            }
                break;
            case UP: {
                uart_printf("[MF] UP\r\n");
            }
                break;
            case DOWN: {
                uart_printf("[MF] DOWN\r\n");
            }
                break;
            default:
                break;
        }

        switch (Action_Buffer[i].target_id) {
            case -2: {
                x = KFS_N2_X;
                y = KFS_N2_Y;
            }
            break;
            case -1: {
                x = KFS_N1_X;
                y = KFS_N1_Y;
            }
            break;
            case 0: {
                x = KFS_0_X;
                y = KFS_0_Y;
            }
            break;
            case 1: {
                x = KFS_1_X;
                y = KFS_1_Y;
            }
            break;
            case 2: {
                x = KFS_2_X;
                y = KFS_2_Y;
            }
            break;
            case 3: {
                x = KFS_3_X;
                y = KFS_3_Y;
            }
            break;
            case 4: {
                x = KFS_4_X;
                y = KFS_4_Y;
            }
            break;
            case 5: {
                x = KFS_5_X;
                y = KFS_5_Y;
            }
            break;
            case 6: {
                x = KFS_6_X;
                y = KFS_6_Y;
            }
            break;
            case 7: {
                x = KFS_7_X;
                y = KFS_7_Y;
            }
            break;
            case 8: {
                x = KFS_8_X;
                y = KFS_8_Y;
            }
            break;
            case 9: {
                x = KFS_9_X;
                y = KFS_9_Y;
            }
            break;
            case 10: {
                x = KFS_10_X;
                y = KFS_10_Y;
            }
            break;
            case 11: {
                x = KFS_11_X;
                y = KFS_11_Y;
            }
            break;
            case 12: {
                x = KFS_12_X;
                y = KFS_12_Y;
            }
            break;
            case 13: {
                x = KFS_13_X;
                y = KFS_13_Y;
            }
            break;
            case 14: {
                x = KFS_14_X;
                y = KFS_14_Y;
            }
            break;
            case 15: {
                x = KFS_15_X;
                y = KFS_15_Y;
            }
            break;
            default:
                break;
        }
        CommandSendPoint(x, y, yaw, 1);
        uart_printf("[MF] go %d center\r\n", Action_Buffer[i].target_id);

        switch (Action_Buffer[i+1].move) {
            case FORWARD:
                yaw = 0.0f;
                break;
            case LEFT:
                yaw = 90.0f;
                break;
            case RIGHT:
                yaw = -90.0f;
                break;
            default:
                break;
        }
        CommandSendPoint(x, y, yaw, 1);
        uart_printf("[MF] yaw %2f\r\n", yaw);

        switch (Action_Buffer[i+1].move) {
            case FORWARD: {
                y += MF_D;
            }
                break;
            case LEFT: {
                x -= MF_D;
            }
                break;
            case RIGHT: {
                x += MF_D;
            }
                break;
            default:
                break;
        }
        CommandSendPoint(x, y, yaw, 1);
        uart_printf("[MF] go %d edge\r\n", Action_Buffer[i].target_id);

        if (Action_Buffer[i].grap) {
            static uint8_t grap_num = 0;
            grap_num++;
            uart_printf("[MF] grab %d\r\n", grap_num);
            CommandSendGrab(grap_num);
        }else {
            uart_printf("[MF] ungrab\r\n");
        }
        uart_printf("\n");
    }

    uart_printf("[MF] go ramp down\r\n");
    CommandSendPoint(RAMP_DOWN_X, RAMP_DOWN_Y, 0.0f, 1);

    // =========================== Arena ============================
    uart_printf("[Arena] 🚀start Arena task\r\n");

    uart_printf("[Arena] enter Arena\r\n");
    CommandSendPoint(RAMP_UP_X, RAMP_UP_Y, 0.0f, 1);

    osEventFlagsWait(KFQEventHandle,
            EVT_CHASSIS_ARRIVAL,
            osFlagsWaitAny,
            osWaitForever);

    uart_printf("[Arena] wait\r\n");
    CommandSendPoint(ARENA_WAIT_X, ARENA_WAIT_Y, 0.0f, 1);

    // 视觉识别
    osEventFlagsWait(KFQEventHandle,
                EVT_ARENA_VISUAL,
                osFlagsWaitAny,
                osWaitForever);

    switch () {
        case 1:
        case 2:
        case 3: {
            // 上车逻辑
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

    while (1) {
        osDelay(1);
    }
}

/**
 * @brief 发送坐标（CMD_1）
 * @param x     X坐标 (float)
 * @param y     Y坐标 (float)
 * @param yaw   角度 (float)
 * @param start 启动标志 (uint8_t)
 */
void CommandSendPoint(float x, float y, float yaw, uint8_t start) {
    uint8_t data[8] = {0};

    // 数据压缩
    int16_t x_int   = (int16_t)(x * 1000.0f);   // x1000
    int16_t y_int   = (int16_t)(y * 1000.0f);   // x1000
    int16_t yaw_int = (int16_t)(yaw * 10.0f);   // x10

    data[0] = (x_int >> 8) & 0xFF;  // X高8位
    data[1] = x_int & 0xFF;         // X低8位
    data[2] = (y_int >> 8) & 0xFF;  // Y高8位
    data[3] = y_int & 0xFF;         // Y低8位
    data[4] = (yaw_int >> 8) & 0xFF;// Yaw高8位
    data[5] = yaw_int & 0xFF;       // Yaw低8位
    data[6] = start;                // 启动标志
    data[7] = 0xFA;                 // 固定尾帧

    command_transmit(COMMAND_CMD_1, 0x1, data);

    osEventFlagsSet(KFQEventHandle, EVT_CHASSIS_ARRIVAL);
    // 等待底盘到达指定位置
    osEventFlagsWait(KFQEventHandle,
                    EVT_CHASSIS_ARRIVAL,
                    osFlagsWaitAny,
                    osWaitForever);
}



void CommandSendCatch(void) {
    uint8_t data[8] = {0};

    bool catch_flag = true;
    data[0] = catch_flag;
    data[1] = 0xFA;

    command_transmit(COMMAND_CMD_2, 0x1, data);

    osEventFlagsSet(KFQEventHandle, EVT_MC_SPEAR_CATCH);
    osEventFlagsWait(KFQEventHandle,
                    EVT_MC_SPEAR_CATCH,
                    osFlagsWaitAny,
                    osWaitForever);
}

void CommandSendGrab(uint8_t grab_flag) {
    uint8_t data[8] = {0};

    data[0] = grab_flag;
    data[1] = 0xFA;

    command_transmit(COMMAND_CMD_3, 0x1, data);

    osEventFlagsSet(KFQEventHandle, EVT_MF_KFS_GRAB);
    osEventFlagsWait(KFQEventHandle,
                    EVT_MF_KFS_GRAB,
                    osFlagsWaitAny,
                    osWaitForever);
}

void CommandSendLift(uint8_t lift_flag) {
    uint8_t data[8] = {0};

    data[0] = lift_flag;
    data[1] = 0xFA;

    command_transmit(COMMAND_CMD_4, 0x1, data);

    osEventFlagsSet(KFQEventHandle, EVT_MF_LIFT);
    osEventFlagsWait(KFQEventHandle,
                    EVT_MF_LIFT,
                    osFlagsWaitAny,
                    osWaitForever);
}

void CommandSendPut(void) {
    uint8_t data[8] = {0};

    data[0] = 0xFA;
    data[1] = 0xFA;

    command_transmit(COMMAND_CMD_5, 0x1, data);

    osEventFlagsSet(KFQEventHandle, EVT_ARENA_KFS_PUT);
    osEventFlagsWait(KFQEventHandle,
                    EVT_ARENA_KFS_PUT,
                    osFlagsWaitAny,
                    osWaitForever);
}
