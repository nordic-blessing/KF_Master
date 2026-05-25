/**
 * @file    CommandTask.h
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/25
 */

#include "inc_user.h"
#include "CommandTask.h"

#define DEBUG 1

/**
 * @brief 发送坐标（CMD_1）
 * @param x     X坐标 (float)
 * @param y     Y坐标 (float)
 * @param yaw   角度 (float)
 * @param start 启动标志 (uint8_t)
 */
void CommandSendPoint(float x, float y, float yaw, uint8_t start) {
    uint8_t data[8] = {0};

    if (MAP == 2) {
        // 蓝方
        x = -x;
    }

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

    command_transmit(COMMAND_CMD_0, 0x1, data);

    // 等待底盘到达指定位置
#if DEBUG
    osEventFlagsSet(KFQEventHandle,EVT_CHASSIS_ARRIVAL);
#endif

    osEventFlagsWait(KFQEventHandle,
                    EVT_CHASSIS_ARRIVAL,
                    osFlagsWaitAny,
                    osWaitForever);
}

/**
 * 底盘抬升
 * @param lift_flag
 */
void CommandSendLift(uint8_t lift_flag) {
    uint8_t data[8] = {0};

    data[0] = lift_flag;
    data[1] = 0xFA;

    command_transmit(COMMAND_CMD_1, 0x1, data);

#if DEBUG
    osEventFlagsSet(KFQEventHandle,EVT_MF_LIFT);
#endif
    osEventFlagsWait(KFQEventHandle,
                    EVT_MF_LIFT,
                    osFlagsWaitAny,
                    osWaitForever);
}



/**
 * 机构抓取Spear
 */
void CommandSendCatch(void) {
    uint8_t data[8] = {0};

    bool catch_flag = true;
    data[0] = catch_flag;
    data[1] = 0xFA;

    command_transmit(COMMAND_CMD_2, 0x1, data);

#if DEBUG
    osEventFlagsSet(KFQEventHandle,EVT_MC_SPEAR_CATCH);
#endif
    osEventFlagsWait(KFQEventHandle,
                    EVT_MC_SPEAR_CATCH,
                    osFlagsWaitAny,
                    osWaitForever);
}

/**
 * 机构抓取KFS
 * @param grab_flag
 */
void CommandSendGrab(uint8_t grab_flag, uint8_t grab_num) {
    uint8_t data[8] = {0};

    data[0] = grab_flag;
    data[1] = grab_num;
    data[2] = 0xFA;

    command_transmit(COMMAND_CMD_3, 0x1, data);

#if DEBUG
    osEventFlagsSet(KFQEventHandle,EVT_MF_KFS_GRAB);
#endif
    osEventFlagsWait(KFQEventHandle,
                    EVT_MF_KFS_GRAB,
                    osFlagsWaitAny,
                    osWaitForever);
}


/**
 * 机构放置KFS
 */
void CommandSendPut(void) {
    uint8_t data[8] = {0};

    data[0] = 0;
    data[1] = 0xFA;

    command_transmit(COMMAND_CMD_4, 0x1, data);

#if DEBUG
    osEventFlagsSet(KFQEventHandle,EVT_ARENA_KFS_PUT);
#endif
    osEventFlagsWait(KFQEventHandle,
                    EVT_ARENA_KFS_PUT,
                    osFlagsWaitAny,
                    osWaitForever);
}