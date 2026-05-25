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

#include "CommandTask.h"

#include "MC.h"
#include "MFTask.h"
#include "MF.h"
#include "Arena.h"

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

void user_init(void);

#endif //INC_USER_H
