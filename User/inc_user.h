/**
 * @file    ${HEADER_FILENAME}
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/10
 */

#ifndef INC_USER_H
#define INC_USER_H

#include "splib.h"
#include "cmsis_os.h"
#include <stdbool.h>

#define MAP                 1   // 1:红方     2:蓝方

#define DEBUG_PRINTF        1

// 事件组
extern osEventFlagsId_t KFQEventHandle;
#define EVT_INIT_TEST           (1U<<0) // 设备间检测
#define EVT_INIT_MAP_GEN        (1U<<1) // MF路线生成
#define EVT_TASK_START          (1U<<2) // 开启任务
#define EVT_CHASSIS_ARRIVAL     (1U<<3) // 底盘到达目标状态
#define EVT_MC_SPEAR_DETECT     (1U<<4) // [MC] 武器头检测
#define EVT_MC_SPEAR_CATCH      (1U<<5) // [MC] 武器头抓取
#define EVT_MC_SPEAR_ASSEMBLE   (1U<<6) // [MC] 武器头对接
#define EVT_MF_LIFT             (1U<<7) // [MF] 抬升动作
#define EVT_MF_KFS_GRAB         (1U<<8) // [MF] 吸取KFS
#define EVT_ARENA_VISUAL        (1U<<9) // [Arena] 视觉指导动作
#define EVT_ARENA_KFS_PUT      (1U<<10) // [Arena] KFS放置

// 计数信号量
extern osSemaphoreId_t SpearheadsSemHandle;
extern osSemaphoreId_t KFSSemHandle;

typedef enum {
    KFS_TYPE_R1    = 0b00,  // R1KFS
    KFS_TYPE_R2    = 0b01,  // R2KFS
    KFS_TYPE_FAKE  = 0b10,  // 假KFS
    KFS_TYPE_EMPTY = 0b11   // 空格子
} KFS_Type_t;

#define MFNum           18
#define MAX_PATH_STEPS  32  // 最大步数

typedef struct {
    int8_t id;
    uint8_t r, c;
    uint8_t height;
} MF_Node; // MF节点数据

typedef struct {
    int8_t id;
    bool grap;
} MF_Path; // MF路径

typedef enum {
    STAY,
    FORWARD,
    LEFT,
    RIGHT,
    BACKWARD
} MF_MoveAction; // MF水平运动状态

typedef enum {
    LEVEL,
    UP,
    DOWN
} MF_LiftAction; // MF垂直运动状态

typedef struct {
    int8_t target_id;
    MF_MoveAction move;
    MF_LiftAction lift;
    uint8_t target_height;
    bool grap;
} MF_ActionStep; // MF路径动作

extern KFS_Type_t KFS_status[12];
extern uint8_t MF_PATH[MAX_PATH_STEPS];
extern MF_ActionStep Action_Buffer[MAX_PATH_STEPS];
extern uint8_t Action_Buffer_Size;

void user_init(void);

#endif //INC_USER_H
