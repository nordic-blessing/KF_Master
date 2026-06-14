//
// Created by Icol_Lee on 2026/2/2.
//

#include "splib.h"

#if USE_SPLIB_VISUAL_UART

/* Includes ------------------------------------------------------------------*/
#include "visual_uart.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
Visual_t visualData;

ProtocolHandler visual_uart={
    .package_length =VISUAL_DATA_LENGTH,
    .header = VISUAL_HEADER,
    .header_length = VISUAL_HEADER_LENGTH,
    .tail_flag = true,
    .tail = VISUAL_TAIL,
    .tail_length = VISUAL_TAIL_LENGTH,
    .callback = Visual_Receive,
    
    .buffer_index = 0,
    .header_found = false,
    .buffer = {0}
};

/* Private type --------------------------------------------------------------*/
/* Private function declarations ---------------------------------------------*/
/* function prototypes -------------------------------------------------------*/

uint16_t calculateCRC16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 8; j--;) {
            crc = (crc << 1) ^ ((crc & 0x8000) ? 0x1021 : 0);
        }
    }

    return crc;
}

/**
 * PC -> MCU
 * @param data
 */
void Visual_Receive(uint8_t *data) {
    visualData.cmd = data[1];
    for (uint8_t i = 0; i < 8; i++) {
        visualData.data[i] = data[i+2];
    }

    switch (visualData.cmd) {
        // 视觉检测Spear
        case VISUAL_CMD_Spearhead: {
            visualData.flag = visualData.data[0];
            osEventFlagsSet(KFQEventHandle, EVT_MC_SPEAR_DETECT);
        }
            break;

        // 对接检测
        case VISUAL_CMD_assemble: {
            switch (visualData.data[0]) {
                case 0x01:
                    osEventFlagsSet(KFQEventHandle, EVT_MC_SPEAR_ASSEMBLE);
                break;
                case 0x02:
                    osEventFlagsSet(KFQEventHandle, EVT_MC_LEAVE);
                break;
                default:
                    break;
            }
        }
            break;

        //
        case VISUAL_CMD_Arena_1:
        case VISUAL_CMD_Arena_2:{
            visualData.num = visualData.data[0];
            osEventFlagsSet(KFQEventHandle, EVT_ARENA_VISUAL);
        }
            break;
        case VISUAL_CMD_Arena_Put: {
            osEventFlagsSet(KFQEventHandle, EVT_ARENA_KFS_PUT_1);
        }
            break;
        default:
            break;
    }

//    uart_printf("%f, %f\r\n", visualData.data1, visualData.data2);
}

/**
 * MCU -> PC
 * @param CMD
 */
void Visual_Send(uint8_t CMD) {
    uint8_t tx_data[13]={0};

    tx_data[0] = VISUAL_HEADER;
    tx_data[1] = CMD;
    switch (CMD) {
        case MCU_CMD_Spear1:
        case MCU_CMD_Fist1:
        case MCU_CMD_Palm1:
        case MCU_CMD_Palm2:
        case MCU_CMD_Fist2:
        case MCU_CMD_Spear2: {
            tx_data[2] = 0x00;
        }
        break;
        case MCU_CMD_Arena: {
            tx_data[2] = 0x00;
        }
        break;
        default:
            break;
    }

    tx_data[10] = (uint8_t)(calculateCRC16(tx_data, 10) >> 8) & 0xFF;
    tx_data[11] = (uint8_t)(calculateCRC16(tx_data, 10) & 0xFF);

    tx_data[12] = VISUAL_TAIL;

    HAL_UART_Transmit(&VISUAL_UART, tx_data, 13, 100);
}

#endif

/************************ COPYRIGHT(C) Pangolin Robot Lab **************************/
