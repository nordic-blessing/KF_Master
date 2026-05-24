//
// Created by Icol_Lee on 2026/2/2.
//

#ifndef DEVICE_VISUAL_H
#define DEVICE_VISUAL_H

#include <stdint.h>
#include <string.h>

#define VISUAL_UART     huart5

#define VISUAL_HEADER         0xEF
#define VISUAL_HEADER_LENGTH    1u
#define VISUAL_TAIL           0xFF
#define VISUAL_TAIL_LENGTH      1u
#define VISUAL_DATA_LENGTH      11

#define MCU_CMD_Spear1          0x01    // 矛1
#define MCU_CMD_Fist1           0x02    // 拳1
#define MCU_CMD_Palm1           0x03    // 掌1
#define MCU_CMD_Palm2           0x04    // 掌2
#define MCU_CMD_Fist2           0x05    // 拳2
#define MCU_CMD_Spear2          0x06    // 矛2
#define VISUAL_CMD_Spearhead    0x07    // 视觉应答武器头

#define MCU_CMD_Map             0x08    // 标记场地颜色

#define VISUAL_CMD_Path         0x09    // 视觉发送路径

#define VISUAL_CMD_assemble     0x0A    // 视觉发送对接状态

typedef struct {
    uint8_t cmd;
    uint8_t flag;
    uint8_t data[8];
}Visual_t;

extern Visual_t visualData;

void Visual_Receive(uint8_t *data);
void Visual_Send(uint8_t CMD);

#endif //DEVICE_VISUAL_H

/************************ COPYRIGHT(C) Pangolin Robot Lab **************************/
