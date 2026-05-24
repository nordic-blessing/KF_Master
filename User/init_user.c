/**
 * @file    init_user.c
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/10
 */

#include "inc_user.h"

KFS_Type_t KFS_status[12];  // KFS状态表
int8_t path[16];            // 路线节点表
uint8_t path_len = 0;       // 路径长度

MF_ActionStep Action_Buffer[MAX_PATH_STEPS];
uint8_t Action_Buffer_Size = 0;
MF_Path PATH[MAX_PATH_STEPS];

const char* move_names[] = {"S", "F", "L", "R", "B"};
const char* lift_names[] = {"-", "▲", "▼"};

MF_Node Red_map[MFNum] = {
    {-2,0, 2, 0}, {-1,0, 1, 0}, {0, 0, 0, 0},
    {1, 1, 2, 2}, {2, 1, 1, 1}, {3, 1, 0, 2},
    {4, 2, 2, 1}, {5, 2, 1, 2}, {6, 2, 0, 3},
    {7, 3, 2, 2}, {8, 3, 1, 3}, {9, 3, 0, 2},
    {10,4, 2, 1}, {11,4, 1, 2}, {12,4, 0, 1},
    {13,5, 2, 0}, {14,5, 1, 0}, {15,5, 0, 0}
};

MF_Node Blue_map[MFNum] = {
    {0, 0, 2, 0}, {-1,0, 1, 0}, {-2,0, 0, 0},
    {3, 1, 2, 2}, {2, 1, 1, 1}, {1, 1, 0, 2},
    {6, 2, 2, 3}, {5, 2, 1, 2}, {4, 2, 0, 1},
    {9, 3, 2, 2}, {8, 3, 1, 3}, {7, 3, 0, 2},
    {12,4, 2, 1}, {11,4, 1, 2}, {10,4, 0, 1},
    {15,5, 2, 0}, {14,5, 1, 0}, {13,5, 0, 0}
};

void func_fdcan1(CAN_RxBuffer* rxBuffer);
void KFS_CheatMessage(uint8_t* buffer);
void generate_action_sequence(MF_Node* target_map, MF_Path* input_path, uint8_t path_len);

void user_init(void) {
    uart_printf("KF_Master\r\n");
    uart_IT_protocol_init();

    CAN_Filter_Mask_Config(&COMMAND_CAN, CanFilter_0|CanFifo_0|Can_StdId|Can_DataType, 0x000, 0x400);
    CAN_Start_IT(&COMMAND_CAN, CanFifo_0, func_fdcan1);

    wait: uint32_t flag = osEventFlagsWait(KFQEventHandle,
        EVT_INIT_MAP_GEN,
        osFlagsWaitAny,
        200);
    if (flag == osErrorTimeout) {
        uart_printf("[Init] waiting path\r\n");
        goto wait;
    }else {
        uart_printf("[Init] get path");

        uint8_t grab_count = 0; // 局部历史抓取计数器

        for (uint8_t i = 0; i < path_len; i++) {
            int8_t curr_id = PATH[i].id;

            // 场外前瞻远程抓取（-2拿1，-1拿2，0拿3）
            if (curr_id == -2) {
                if (KFS_status[1 - 1] == KFS_TYPE_R2 && grab_count < 2) {
                    PATH[i].grap = true; // 直接在 -2 号位下爪抓 1 号位！
                    grab_count++;
                }
                continue;
            }
            if (curr_id == -1) {
                if (KFS_status[2 - 1] == KFS_TYPE_R2 && grab_count < 2) {
                    PATH[i].grap = true; // 直接在 -1 号位下爪抓 2 号位！
                    grab_count++;
                }
                continue;
            }
            if (curr_id == 0) {
                if (KFS_status[3 - 1] == KFS_TYPE_R2 && grab_count < 2) {
                    PATH[i].grap = true; // 直接在 0 号位下爪抓 3 号位！
                    grab_count++;
                }
                continue;
            }

            // 进入MF后,在当前位号拿取下一个路径位号的 KFS
            if (curr_id >= 1 && curr_id <= 12) {
                if (i + 1 < path_len) {
                    int8_t next_id = PATH[i + 1].id;
                    if (next_id >= 1 && next_id <= 12) {
                        if (KFS_status[next_id - 1] == KFS_TYPE_R2 && grab_count < 2) {
                            PATH[i].grap = true;
                            grab_count++;
                        }
                    }
                }
            }
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
        DELAY(500);
    }

}

/**
 * CAN1接受中断回调
 * @param rxBuffer
 */
void func_fdcan1(CAN_RxBuffer* rxBuffer) {
    uint16_t id = rxBuffer->header.StdId;
}

ProtocolHandler KFS_CheatHandler = {
    .package_length = 14,
    .header =0xFE,
    .header_length = 1,
    .tail_flag = false,
    .callback = KFS_CheatMessage,

    .buffer_index = 0,
    .header_found = false,
    .buffer = {0}
};

/**
 * KFS状态输入结果
 * @param buffer
 */
void KFS_CheatMessage(uint8_t* buffer) {
    uint8_t checksum = 0;
    for (uint8_t i = 0;i<13;i++) {
        checksum += buffer[i];
    }

    if (checksum == buffer[13]) {
        KFS_status[0] = (buffer[2]>>0) & 0x03;
        KFS_status[1] = (buffer[2]>>2) & 0x03;
        KFS_status[2] = (buffer[2]>>4) & 0x03;
        KFS_status[3] = (buffer[2]>>6) & 0x03;
        KFS_status[4] = (buffer[3]>>0) & 0x03;
        KFS_status[5] = (buffer[3]>>2) & 0x03;
        KFS_status[6] = (buffer[3]>>4) & 0x03;
        KFS_status[7] = (buffer[3]>>6) & 0x03;
        KFS_status[8] = (buffer[4]>>0) & 0x03;
        KFS_status[9] = (buffer[4]>>2) & 0x03;
        KFS_status[10] = (buffer[4]>>4) & 0x03;
        KFS_status[11] = (buffer[4]>>6) & 0x03;

        uint8_t p[16];
        p[0]  = (buffer[5] >> 0) & 0x0F;  p[1]  = (buffer[5] >> 4) & 0x0F;
        p[2]  = (buffer[6] >> 0) & 0x0F;  p[3]  = (buffer[6] >> 4) & 0x0F;
        p[4]  = (buffer[7] >> 0) & 0x0F;  p[5]  = (buffer[7] >> 4) & 0x0F;
        p[6]  = (buffer[8] >> 0) & 0x0F;  p[7]  = (buffer[8] >> 4) & 0x0F;
        p[8]  = (buffer[9] >> 0) & 0x0F;  p[9]  = (buffer[9] >> 4) & 0x0F;
        p[10] = (buffer[10] >> 0) & 0x0F; p[11] = (buffer[10] >> 4) & 0x0F;
        p[12] = (buffer[11] >> 0) & 0x0F; p[13] = (buffer[11] >> 4) & 0x0F;
        p[14] = (buffer[12] >> 0) & 0x0F; p[15] = (buffer[12] >> 4) & 0x0F;

        path_len = 0; // 重置当前接收到的有效路线长度

        for (uint8_t i = 0; i < 16; i++) {
            if (p[i] == 0) break; // 提前截断
            int8_t current_id = (i < 3 && p[i] >= 13) ?
                                ((p[i] == 13) ? -2 : (p[i] == 14) ? -1 : 0) : (int8_t)p[i];
            PATH[path_len].id = current_id;
            PATH[path_len].grap = false; // 默认初始化为不下爪
            path_len++;
        }

        osEventFlagsSet(KFQEventHandle, EVT_INIT_MAP_GEN);

    }
}

/**
 * 从地图中寻找目标节点状态信息
 * @param target_map 目标地图
 * @param id         目标节点
 * @return           返回节点MF状态信息
 */
MF_Node* find_node(MF_Node* target_map, const int8_t id) {
    for (int i = 0; i < MFNum; i++) {
        if (target_map[i].id == id) return &target_map[i];
    }
    return NULL;
}

/**
 * 转义规划路径为标准的物理动作序列 (动态节点检索安全版)
 * @param target_map 地图 (Red_map 或 Blue_map)
 * @param input_path 路径 (rx_mf_path)
 * @param path_len   路径长度 (rx_path_len)
 */
void generate_action_sequence(MF_Node* target_map, MF_Path* input_path, uint8_t path_len) {
    if (path_len == 0) {
        Action_Buffer_Size = 0;
        return;
    }

    Action_Buffer_Size = 0;

    // ===========================================================================
    // 1. ⚡ 显式注入第 0 步：绝对初始起点帧
    // ===========================================================================
    int8_t start_id = input_path[0].id;
    // 🛡️ 核心修复：改用动态 find_node 查找，彻底免疫地图数组乱序隐患
    const MF_Node* start_node = find_node(target_map, start_id);

    if (start_node != NULL) {
        MF_ActionStep start_step;
        start_step.target_id = start_id;
        start_step.move = STAY;
        start_step.lift = LEVEL;
        start_step.target_height = start_node->height; // 精准抓取起点高度
        start_step.grap = input_path[0].grap;

        Action_Buffer[Action_Buffer_Size] = start_step;
        Action_Buffer_Size++;
    }

    // ===========================================================================
    // 2. ⚡ 注入后续移动帧
    // ===========================================================================
    for (int i = 0; i < path_len - 1; i++) {
        if (Action_Buffer_Size >= MAX_PATH_STEPS)
            break;

        int8_t curr_id = input_path[i].id;
        int8_t next_id = input_path[i+1].id;

        // 🛡️ 核心修复：全面换装安全动态查找，彻底终结错位
        const MF_Node* curr = find_node(target_map, curr_id);
        const MF_Node* next = find_node(target_map, next_id);

        // 容错安全防御：如果拓扑地图查无此点，直接跳过保护内存
        if (curr == NULL || next == NULL)
            continue;

        MF_ActionStep generic_step;
        generic_step.target_id = next_id;
        generic_step.target_height = next->height; // 精准绑定真正目的地的高度，拒绝乱序污染
        generic_step.grap = input_path[i+1].grap;

        // 🤖 麦克纳姆轮运动状态精准解算
        if (next->r > curr->r) {
            generic_step.move = FORWARD;
        } else if (next->r < curr->r) {
            generic_step.move = BACKWARD;
        } else if (next->c > curr->c) {
            generic_step.move = RIGHT;
        } else if (next->c < curr->c) {
            generic_step.move = LEFT;
        } else {
            generic_step.move = STAY;
        }

        // 提升轴垂直运动状态解算
        if (next->height > curr->height) {
            generic_step.lift = UP;       // 5到6高度变大，现在可以百分之百正确触发 ▲ UP 了！
        } else if (next->height < curr->height) {
            generic_step.lift = DOWN;
        } else {
            generic_step.lift = LEVEL;
        }

        Action_Buffer[Action_Buffer_Size] = generic_step;
        Action_Buffer_Size++;
    }
}