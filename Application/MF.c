/**
 * @file    MF.h
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/25
 */

#include "inc_user.h"
#include "MF.h"

void MF_Task(void) {
    uart_printf("[MF] 🚀start MF task\r\n");

    float yaw = 0.0f;
    float x = 3.000f;
    float y = MC_ASSEMBLE_Y;
    uart_printf("[MF] total %d step\r\n", Action_Buffer_Size);
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
        uart_printf("[MF] yaw %2f\r\n", yaw);
        CommandSendPoint(x, y, yaw, 1);

        switch (Action_Buffer[i].lift) {
            case LEVEL: {
                uart_printf("[MF] LEVEL\r\n");
                CommandSendLift(LEVEL);
            }
                break;
            case UP: {
                uart_printf("[MF] UP\r\n");
                CommandSendLift(UP);
            }
                break;
            case DOWN: {
                uart_printf("[MF] DOWN\r\n");
                CommandSendLift(DOWN);
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
        uart_printf("[MF] go %d center\r\n", Action_Buffer[i].target_id);
        CommandSendPoint(x, y, yaw, 1);

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
        uart_printf("[MF] yaw %2f\r\n", yaw);
        CommandSendPoint(x, y, yaw, 1);

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
        uart_printf("[MF] go %d edge\r\n", Action_Buffer[i].target_id);
        CommandSendPoint(x, y, yaw, 1);

        if (Action_Buffer[i].grap) {
            static uint8_t grap_num = 0;
            uint8_t grab_flag = 0;
            grap_num++;
            switch (Action_Buffer[i].target_id) {
                case -2:
                case 0:
                    grab_flag = 1;
                    break;

                default: {
                    switch (Action_Buffer[i+1].lift) {
                        case UP:
                            grab_flag = 2;
                            break;
                        case DOWN:
                            grab_flag = 3;
                            break;
                        default:
                            break;
                    }
                }
                    break;
            }
            uart_printf("[MF] grab %d, %d\r\n", grap_num, grab_flag);
            CommandSendGrab(grab_flag, grap_num);
        }else {
            uart_printf("[MF] ungrab\r\n");
        }
        uart_printf("\n");
    }
    uart_printf("[MF] task done\r\n");
    uart_printf("\r\n");
}
