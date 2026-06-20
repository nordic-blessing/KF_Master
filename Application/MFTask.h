/**
 * @file    MFTask.h
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/25
 */

#ifndef MFTASK_H
#define MFTASK_H

#include <stdint.h>
#include <stdbool.h>

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

typedef enum {
    KFS_NONE    = 0, // 不执行
    KFS_CLEAN   = 1, // 清理
    KFS_GRAP    = 2  // 抓取
} MF_GrapAction; // 机械手动作状态

typedef struct {
    int8_t id;
    MF_GrapAction grap; // 修改为枚举类型
} MF_Path;

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

typedef enum {
    ARM_FRONT = 0, // 正前方吸取/排障
    ARM_LEFT  = 1, // 向左侧伸出吸取
    ARM_RIGHT = 2  // 向右侧伸出吸取
} MF_ArmDirection; // 机械手朝向

typedef struct {
    int8_t target_id;
    MF_MoveAction move;
    MF_LiftAction lift;
    uint8_t target_height;
    MF_GrapAction grap;
    MF_ArmDirection arm_dir;
} MF_ActionStep;

extern MF_Node Red_map[MFNum];
extern MF_Node Blue_map[MFNum];

extern KFS_Type_t KFS_status[12];
extern MF_Path PATH[MAX_PATH_STEPS];
extern uint8_t path_len;

extern MF_ActionStep Action_Buffer[MAX_PATH_STEPS];
extern uint8_t Action_Buffer_Size;

void KFS_CheatMessage(uint8_t* buffer);
void generate_action_sequence(MF_Node* target_map, MF_Path* input_path, uint8_t path_len);

#endif //MFTASK_H
