/**
 * @file    init_user.c
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/10
 */

#include "inc_user.h"

int8_t path[16];            // 路线节点表
const char* move_names[] = {"S", "F", "L", "R", "B"};
const char* lift_names[] = {"-", "▲", "▼"};

void func_fdcan2(CAN_RxBuffer* rxBuffer);

void user_init(void) {
    DELAY(560);
    uart_printf("KF_Master\r\n");
    uart_IT_protocol_init();

    CAN_Filter_Mask_Config(&COMMAND_CAN, CanFilter_14|CanFifo_0|Can_StdId|Can_DataType, 0x001, 0x603);
    CAN_Start_IT(&COMMAND_CAN, CanFifo_0, func_fdcan2);

    uart_printf("[Init] waiting path...\r\n");
    uint32_t flag;
    do {
        flag = osEventFlagsWait(KFQEventHandle,
        EVT_INIT_MAP_GEN,
        osFlagsWaitAny,
        50);
    }while (flag == osFlagsErrorTimeout);
    wait:

    uart_printf("\r\n");
    switch (MAP) {
        case 1:
            uart_printf("[Init] RED MAP\r\n");
        break;
        case 2:
            uart_printf("[Init] BLUE MAP\r\n");
        break;
        default:
            uart_printf("[Init] UNKNOWN MAP\r\n");
        break;
    }
    uart_printf("\r\n");

    generate_action_sequence((MAP == 1) ? Red_map : Blue_map, PATH, path_len);

    uart_printf("STEP | TARGET | MOVE | LIFT | GRAP\n");
    uart_printf("----------------------------------\n");
    for (int i = 0; i < Action_Buffer_Size; i++) {
        // 抓取：1 / 0
        uart_printf(" #%02d |  %3d   |  %s   |  %s   |  %d\n",
               i,
               Action_Buffer[i].target_id,
               move_names[Action_Buffer[i].move],
               lift_names[Action_Buffer[i].lift],
               Action_Buffer[i].grap ? 1 : 0);
    }
    uart_printf("----------------------------------\n");
    uart_printf("\r\n");

    osEventFlagsSet(KFQEventHandle, EVT_TASK_START);
}

/**
 * CAN1接受中断回调
 * @param rxBuffer
 */
void func_fdcan2(CAN_RxBuffer* rxBuffer) {
    command_receive(rxBuffer);
}
