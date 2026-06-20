/**
 * @file    CommandTask.h
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief
 *      CMD_0   发送坐标
 *      CMD_1   底盘抬升
 *
 *      CMD_2   机构准备抓取Spear
 *      CMD_3   机构抓取Spear
 *      CMD_4   机构释放Spear
 *      CMD_5   机构抓取KFS
 *      CMD_6   机构放置KFS
 *
 * @version 0.1
 * @date    2026/5/25
 */

#include "inc_user.h"
#include "CommandTask.h"

#define DEBUG_1 0   // 底盘 1:用于逻辑测试，不发送CAN消息，不做等待 0:发送CAN消息，并等待回复
#define DEBUG_2 0   // 机构 1:用于逻辑测试，不发送CAN消息，不做等待 0:发送CAN消息，并等待回复

/**
 * @brief 向底盘发送目标坐标（CMD_0）
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

    // 等待底盘到达指定位置
#if DEBUG_1
    osEventFlagsSet(KFQEventHandle,EVT_CHASSIS_ARRIVAL);
    uart_printf("[MF] point:%f, %f, %f\r\n", x, y, yaw);
#else
    // 等待底盘接收回复
    osEventFlagsClear(KFQEventHandle,EVT_CHASSIS_ECHO);
    static uint32_t flag;
    do{
        command_transmit(COMMAND_CMD_0, 0x1, data);

        // 等待200ms若超时未回复，重复发送并等待
        flag = osEventFlagsWait(KFQEventHandle,
                         EVT_CHASSIS_ECHO,
                         osFlagsWaitAny,
                         200);
        uart_printf("re point\r\n");
    }while (flag == osFlagsErrorTimeout);
    uart_printf("[MF] point:%f, %f, %f\r\n", x, y, yaw);
#endif
    /* 等待底盘到达目标位置 */
    osEventFlagsWait(KFQEventHandle,
                    EVT_CHASSIS_ARRIVAL,
                    osFlagsWaitAny,
                    osWaitForever);
    uart_printf("point done\r\n");
    DELAY(2);
}

/**
 * @brief 底盘抬升（CMD_1）
 * @param lift_flag
 */
void CommandSendLift(uint8_t lift_flag) {
    uint8_t data[8] = {0};

    data[0] = lift_flag;

    // 等待底盘抬升结束
#if DEBUG_1
    osEventFlagsSet(KFQEventHandle,EVT_MF_LIFT);
#else
    osEventFlagsClear(KFQEventHandle,EVT_CHASSIS_ECHO);
    static uint32_t flag;
    do{
        command_transmit(COMMAND_CMD_1, 0x1, data);

        // 等待200ms若超时未回复，重复发送并等待
        flag = osEventFlagsWait(KFQEventHandle,
                         EVT_CHASSIS_ECHO,
                         osFlagsWaitAny,
                         200);
        uart_printf("re lift\r\n");
    }while (flag == osFlagsErrorTimeout);
#endif
    uart_printf("[MF] lift_flag: %d\r\n", lift_flag);

    /* 等待底盘抬升动作完成 */
    osEventFlagsWait(KFQEventHandle,
                    EVT_MF_LIFT,
                    osFlagsWaitAny,
                    osWaitForever);
    uart_printf("lift done\r\n");
    DELAY(2);
}

/**
 * @brief 机构抓取Spear准备（CMD_2）
 */
void CommandSendCatchPrepare(void) {
    uint8_t data[8] = {0};

#if DEBUG_2
    osEventFlagsSet(KFQEventHandle,EVT_MC_SPEAR_PREPARE);
#else
    osEventFlagsClear(KFQEventHandle,EVT_MECHANISM_ECHO);
    static uint32_t flag;

    // 等待200ms若超时未回复，重复发送并等待
    do{
        command_transmit(COMMAND_CMD_2, 0x1, data);
        flag = osEventFlagsWait(KFQEventHandle,
                         EVT_MECHANISM_ECHO,
                         osFlagsWaitAny,
                         200);
        uart_printf("re prepare\r\n");
    }while (flag == osFlagsErrorTimeout);
#endif
    // 等待机构抓取准备结束
    // osEventFlagsWait(KFQEventHandle,
    //                 EVT_MC_SPEAR_PREPARE,
    //                 osFlagsWaitAny,
    //                 osWaitForever);
    uart_printf("prepare done\r\n");
    DELAY(2);
}

/**
 * @brief 机构抓取Spear（CMD_3）
 */
void CommandSendCatch(void) {
    uint8_t data[8] = {0};

    bool catch_flag = true;
    data[0] = catch_flag;

    // 等待机构抓取完成
#if DEBUG_2
    osEventFlagsSet(KFQEventHandle,EVT_MC_SPEAR_CATCH);
#else
    osEventFlagsClear(KFQEventHandle,EVT_MECHANISM_ECHO);
    static uint32_t flag;
    // 等待200ms若超时未回复，重复发送并等待
    do{
        command_transmit(COMMAND_CMD_3, 0x1, data);
        flag = osEventFlagsWait(KFQEventHandle,
                         EVT_MECHANISM_ECHO,
                         osFlagsWaitAny,
                         200);
        uart_printf("re catch\r\n");
    }while (flag == osFlagsErrorTimeout);
#endif
    // 等待抓取动作完成
    osEventFlagsWait(KFQEventHandle,
                    EVT_MC_SPEAR_CATCH,
                    osFlagsWaitAny,
                    osWaitForever);
    uart_printf("catch done\r\n");
    DELAY(2);
}

/**
 * @brief 释放Spear（CMD_4）
 */
void CommandSendRelease(void) {
    uint8_t data[8] = {0};

#if DEBUG_2
    osEventFlagsSet(KFQEventHandle,EVT_MC_SPEAR_RELEASE);
#else
    osEventFlagsClear(KFQEventHandle,EVT_MECHANISM_ECHO);
    static uint32_t flag;

    // 等待200ms若超时未回复，重复发送并等待
    do{
        command_transmit(COMMAND_CMD_4, 0x1, data);
        flag = osEventFlagsWait(KFQEventHandle,
                         EVT_MECHANISM_ECHO,
                         osFlagsWaitAny,
                         200);
        uart_printf("re release\r\n");
    }while (flag == osFlagsErrorTimeout);
#endif
    // 等待释放动作完成
    // osEventFlagsWait(KFQEventHandle,
    //                 EVT_MC_SPEAR_RELEASE,
    //                 osFlagsWaitAny,
    //                 osWaitForever);
    uart_printf("release done\r\n");
    DELAY(2);
}

/**
 * 抓取KFS（CMD_5）
 * @param grab_flag 1:通道 2:200 3:-200
 * @param grab_num  抓取序号
 */
void CommandSendGrab(uint8_t grab_flag, uint8_t grab_num) {
    uint8_t data[8] = {0};

    data[0] = grab_flag;
    data[1] = grab_num;

#if DEBUG_2
    osEventFlagsSet(KFQEventHandle,EVT_MF_KFS_GRAB);
#else
    osEventFlagsClear(KFQEventHandle,EVT_MECHANISM_ECHO);
    static uint32_t flag;

    // 等待200ms若超时未回复，重复发送并等待
    do{
        command_transmit(COMMAND_CMD_5, 0x1, data);
        flag = osEventFlagsWait(KFQEventHandle,
                         EVT_MECHANISM_ECHO,
                         osFlagsWaitAny,
                         200);
        uart_printf("re grab\r\n");
    }while (flag == osFlagsErrorTimeout);
#endif
    // 等待吸取动作完成
    osEventFlagsWait(KFQEventHandle,
                    EVT_MF_KFS_GRAB,
                    osFlagsWaitAny,
                    osWaitForever);
    uart_printf("grab done\r\n");
    DELAY(2);
}


/* ----------------------- 以下动作未使用过 ----------------------- */

/**
 * @brief 机构放置KFS（CMD_6）
 */
void CommandSendPut(uint8_t put_flag) {
    uint8_t data[8] = {0};
    data[0] = put_flag;

#if DEBUG_2
    osEventFlagsSet(KFQEventHandle,EVT_ARENA_KFS_PUT_1);
#else
    osEventFlagsClear(KFQEventHandle,EVT_MECHANISM_ECHO);
    static uint32_t flag;
    // 等待200ms若超时未回复，重复发送并等待
    do{
        command_transmit(COMMAND_CMD_6, 0x1, data);
        flag = osEventFlagsWait(KFQEventHandle,
                         EVT_MECHANISM_ECHO,
                         osFlagsWaitAny,
                         200);
        uart_printf("re prepare\r\n");
    }while (flag == osFlagsErrorTimeout);
#endif
    // 等待放置动作完成
    osEventFlagsWait(KFQEventHandle,
                    EVT_ARENA_KFS_PUT_1,
                    osFlagsWaitAny,
                    osWaitForever);
    DELAY(2);
}

/**
 * @brief 底盘抬升上R1（CMD_7）
 */
void CommandSendLiftR1(void) {
    uint8_t data[8] = {0};

#if DEBUG_2
    osEventFlagsSet(KFQEventHandle,EVT_ARENA_LIFT);
#else
    osEventFlagsClear(KFQEventHandle,EVT_MECHANISM_ECHO);
    static uint32_t flag;
    do{
        command_transmit(COMMAND_CMD_7, 0x1, data);
        flag = osEventFlagsWait(KFQEventHandle,
                         EVT_MECHANISM_ECHO,
                         osFlagsWaitAny,
                         200);
        uart_printf("re lift R1\r\n");
    }while (flag == osFlagsErrorTimeout);
#endif
    osEventFlagsWait(KFQEventHandle,
                    EVT_ARENA_LIFT,
                    osFlagsWaitAny,
                    osWaitForever);
    DELAY(2);
}