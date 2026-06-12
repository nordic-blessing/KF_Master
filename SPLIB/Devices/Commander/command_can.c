/**
******************************************************************************
  @file     command_can.c
  @brief    私有CAN协议
  @author   Icol Boom <icolboom4@gmail.com>
  @date     2025-01-30 (Created) | 2026-04-28 (Last modified)
  @version  v1.0
  ------------------------------------------------------------------------------
  CHANGE LOG :
    - 2026-01-30 [v1.0] Icol Boom: 创建初始版本，完成初步测试
  ------------------------------------------------------------------------------
  @example
    - 关于CAN ID: DEVICE_ID(4bit, (COMMAND_SLAVE << 3)|DEVICE_ADDR) | CMD ID(5bit) | para(2bit)
  ------------------------------------------------------------------------------
  @attention
    - 代码未经测试，谨慎使用
    - 修改代码后需同步更新版本号、最后修改日期及CHANGE LOG，请务必保证注释清晰明确地
    让后人知晓如何使用该驱动
  ******************************************************************************
  Copyright (c) 2026 ~ -, Sichuan University Pangolin Robot Lab.
  All rights reserved.
  ******************************************************************************
*/

#include "splib.h"

#if USE_SPLIB_CONMMAND

/* Includes ------------------------------------------------------------------*/
#include "command_can.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private type --------------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
/* function prototypes -------------------------------------------------------*/

/**
 * 发送数据
 * @param CMD_ID    消息ID COMMAND_CMD_x (5 bits)
 * @param para      保留位 (2 bits)
 * @param pData
 */
void command_transmit(uint8_t CMD_ID, uint8_t para, uint8_t *pData) {
    uint16_t std_id = (DEVICE_ID<<7) | (CMD_ID << 2) | (para);
    CAN_SendStdData(&COMMAND_CAN, std_id, pData, 8);
}

/**
 * 接收数据
 * @param pData
 */
void command_receive(CAN_RxBuffer* rxBuffer) {
    // 处理主控制器信息
    bool isMaster = (rxBuffer->header.StdId >> 10)&0x1;
    if (isMaster) {
        // 主控制器
        return;
    }else {
        // 从控制器
        uint8_t DeviceAddr = (rxBuffer->header.StdId >> 7) & 0x7;
        uint8_t CmdID = (rxBuffer->header.StdId >> 2) & 0x1F;
        uint8_t echo_data[8]={0};

        switch (DeviceAddr) {
            // Slave1 底盘
            case 0b001:{
                switch (CmdID) {
                    // CMD_0 底盘到达
                    case COMMAND_CMD_0: {
                        uart_printf("get arrive flag\r\n");
                        command_transmit(COMMAND_CMD_8, 0x01, echo_data);
                        osEventFlagsSet(KFQEventHandle, EVT_CHASSIS_ARRIVAL);
                    }
                        break;

                    // CMD_1 抬升完成
                    case COMMAND_CMD_1: {
                        uart_printf("get lift flag\r\n");
                        command_transmit(COMMAND_CMD_8, 0x01, echo_data);
                        osEventFlagsSet(KFQEventHandle, EVT_MF_LIFT);
                    }
                        break;

                    // CMD_2 接收应答
                    case COMMAND_CMD_2: {
                        uart_printf("get echo\r\n");
                        osEventFlagsSet(KFQEventHandle, EVT_CHASSIS_ECHO);
                    }
                        break;
                    default:
                        break;
                }
            }
                break;

            // Slave2 机构
            case 0b010: {
                switch (CmdID) {
                    // CMD_0 Spear准备动作完成
                    case COMMAND_CMD_0: {
                        osEventFlagsSet(KFQEventHandle, EVT_MC_SPEAR_PREPARE);
                    }
                        break;
                    // CMD_1 Spear夹取完成
                    case COMMAND_CMD_1: {
                        osEventFlagsSet(KFQEventHandle, EVT_MC_SPEAR_CATCH);
                    }
                        break;

                    // CMD_2 KFS抓取完成
                    case COMMAND_CMD_2: {
                        osEventFlagsSet(KFQEventHandle, EVT_MF_KFS_GRAB);
                    }
                        break;

                    // 命令3 KFS放置完成
                    case COMMAND_CMD_3: {
                        if (rxBuffer->data[0] == 1) {
                            // 放第一个KFS
                            osEventFlagsSet(KFQEventHandle, EVT_ARENA_KFS_PUT_1);
                        }else if (rxBuffer->data[0] == 2) {
                            // 放第二个KFS
                            osEventFlagsSet(KFQEventHandle, EVT_ARENA_KFS_PUT_2);
                        }
                    }
                        break;

                    // 命令4 命令接收回传
                    case COMMAND_CMD_4: {
                        osEventFlagsSet(KFQEventHandle, EVT_MECHANISM_ECHO);
                    }
                        break;

                    // 命令5 释放Spear
                    case COMMAND_CMD_5: {
                        osEventFlagsSet(KFQEventHandle, EVT_MC_SPEAR_RELEASE);
                    }
                        break;
                    default:
                        break;
                }
            }
                break;
            default:
                break;
        }
    }
}

#endif

/************************ COPYRIGHT(C) Pangolin Robot Lab **************************/
