/**
 * @file    MFTask.c
 * @brief   完美支持侧向盲吸/排障几何方位自动推导的接收端控制核心
 * @date    2026/6/15
 */

#include "inc_user.h"
#include "MFTask.h"

uint8_t MAP = 1; // 默认为红方
KFS_Type_t KFS_status[12];  // KFS状态表
MF_Path PATH[MAX_PATH_STEPS];
uint8_t path_len = 0;       // 路径长度

MF_ActionStep Action_Buffer[MAX_PATH_STEPS];
uint8_t Action_Buffer_Size = 0;

// 🌟 核心新增：用于记录每一路径步上，机械手真正触碰/吸取的道具物理 ID (1~12)，若无动作为 -1
static int8_t PATH_ACTUAL_TARGET_ID[MAX_PATH_STEPS];

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

ProtocolHandler KFS_CheatHandler = {
    .package_length = 14,
    .header = 0xFE,
    .header_length = 1,
    .tail_flag = false,
    .callback = KFS_CheatMessage,
    .buffer_index = 0,
    .header_found = false,
    .buffer = {0}
};

/**
 * 扫描当前位置前左右辐射列
 */
static int8_t Scan_Front_Left_Right_R2(int8_t walk_node_id) {
    if (walk_node_id < 1 || walk_node_id > 12) return -1;

    uint8_t cur_idx = walk_node_id - 1;
    uint8_t row = cur_idx / 3;
    uint8_t col = cur_idx % 3;

    bool check_cols[3] = {false};
    if (col == 0)      { check_cols[0] = true; check_cols[1] = true; }
    else if (col == 1) { check_cols[0] = true; check_cols[1] = true; check_cols[2] = true; }
    else               { check_cols[1] = true; check_cols[2] = true; }

    for (int c = 0; c < 3; c++) {
        if (check_cols[c]) {
            int target_idx = row * 3 + c;
            if (KFS_status[target_idx] == KFS_TYPE_R2) {
                return (target_idx + 1); // 返回漏网之鱼的物理 ID
            }
        }
    }
    return -1;
}

void KFS_CheatMessage(uint8_t* buffer) {
    static bool flag = false;
    if (flag) return;

    uint8_t checksum = 0;
    for (uint8_t i = 0; i < 13; i++) {
        checksum += buffer[i];
    }

    if (checksum == buffer[13]) {
        MAP = buffer[1];

        // 解包数据
        KFS_status[0]  = (buffer[2] >> 0) & 0x03; KFS_status[1]  = (buffer[2] >> 2) & 0x03;
        KFS_status[2]  = (buffer[2] >> 4) & 0x03; KFS_status[3]  = (buffer[2] >> 6) & 0x03;
        KFS_status[4]  = (buffer[3] >> 0) & 0x03; KFS_status[5]  = (buffer[3] >> 2) & 0x03;
        KFS_status[6]  = (buffer[3] >> 4) & 0x03; KFS_status[7]  = (buffer[3] >> 6) & 0x03;
        KFS_status[8]  = (buffer[4] >> 0) & 0x03; KFS_status[9]  = (buffer[4] >> 2) & 0x03;
        KFS_status[10] = (buffer[4] >> 4) & 0x03; KFS_status[11] = (buffer[4] >> 6) & 0x03;

        uint8_t p[16];
        p[0]  = (buffer[5]  >> 0) & 0x0F; p[1]  = (buffer[5]  >> 4) & 0x0F;
        p[2]  = (buffer[6]  >> 0) & 0x0F; p[3]  = (buffer[6]  >> 4) & 0x0F;
        p[4]  = (buffer[7]  >> 0) & 0x0F; p[5]  = (buffer[7]  >> 4) & 0x0F;
        p[6]  = (buffer[8]  >> 0) & 0x0F; p[7]  = (buffer[8]  >> 4) & 0x0F;
        p[8]  = (buffer[9]  >> 0) & 0x0F; p[9]  = (buffer[9]  >> 4) & 0x0F;
        p[10] = (buffer[10] >> 0) & 0x0F; p[11] = (buffer[10] >> 4) & 0x0F;
        p[12] = (buffer[11] >> 0) & 0x0F; p[13] = (buffer[11] >> 4) & 0x0F;
        p[14] = (buffer[12] >> 0) & 0x0F; p[15] = (buffer[12] >> 4) & 0x0F;

        path_len = 0;
        for (uint8_t i = 0; i < 16; i++) {
            if (p[i] == 0) break;
            int8_t current_id = (i < 3 && p[i] >= 13) ?
                                ((p[i] == 13) ? -2 : (p[i] == 14) ? -1 : 0) : (int8_t)p[i];
            PATH[path_len].id = current_id;
            PATH[path_len].grap = KFS_NONE;
            PATH_ACTUAL_TARGET_ID[path_len] = -1; // 默认没有对齐道具
            path_len++;
        }

        uint8_t grab_count = 0;

        if (path_len >= 2) {

            // 🌟 阶段 A：场外起跑前瞻
            for (uint8_t i = 0; i < path_len; i++) {
                int8_t step_id = PATH[i].id;
                if (step_id > 0) break;

                int8_t front_node = (step_id == -2) ? 1 : ((step_id == -1) ? 2 : 3);
                KFS_Type_t front_item = KFS_status[front_node - 1];

                if (front_item == KFS_TYPE_R2) {
                    PATH[i].grap = KFS_GRAP;
                    PATH_ACTUAL_TARGET_ID[i] = front_node; // 绑定目标道具物理ID
                    KFS_status[front_node - 1] = KFS_TYPE_EMPTY;
                    grab_count++;
                }
                else if (front_item == KFS_TYPE_R1) {
                    PATH[i].grap = KFS_CLEAN;
                    PATH_ACTUAL_TARGET_ID[i] = front_node; // 绑定目标排障物理ID
                    KFS_status[front_node - 1] = KFS_TYPE_EMPTY;
                }
            }

            // 🌟 阶段 B：中场主干道盲踩
            for (int i = 1; i < (path_len - 2); i++) {
                int8_t walk_node = PATH[i].id;
                if (walk_node >= 1 && walk_node <= 12) {
                    KFS_Type_t current_grid_item = KFS_status[walk_node - 1];

                    if (current_grid_item == KFS_TYPE_R1 || current_grid_item == KFS_TYPE_R2) {
                        PATH[i].grap = KFS_CLEAN;
                        PATH_ACTUAL_TARGET_ID[i] = walk_node; // 踩在脚底下，目标就是它自己

                        if (current_grid_item == KFS_TYPE_R2 && walk_node != 4) {
                            PATH[i].grap = KFS_GRAP;
                            grab_count++;
                        }
                        KFS_status[walk_node - 1] = KFS_TYPE_EMPTY;
                    }
                }
            }

            // 🌟 阶段 C：末端绝杀处理
            uint8_t final_field_idx = path_len - 2;
            int8_t final_field_node = PATH[final_field_idx].id;

            if (final_field_node >= 1 && final_field_node <= 12) {
                KFS_Type_t final_grid_item = KFS_status[final_field_node - 1];

                if (final_grid_item == KFS_TYPE_R1 || final_grid_item == KFS_TYPE_R2) {
                    PATH[final_field_idx].grap = KFS_CLEAN;
                    PATH_ACTUAL_TARGET_ID[final_field_idx] = final_field_node;

                    if (final_grid_item == KFS_TYPE_R2 && final_field_node == 10) {
                        PATH[final_field_idx].grap = KFS_GRAP;
                        grab_count++;
                    }
                    KFS_status[final_field_node - 1] = KFS_TYPE_EMPTY;
                }
                else if (grab_count < 2) {
                    int8_t radiate_r2_id = Scan_Front_Left_Right_R2(final_field_node);
                    if (radiate_r2_id != -1) {
                        PATH[final_field_idx].grap = KFS_GRAP;
                        PATH_ACTUAL_TARGET_ID[final_field_idx] = radiate_r2_id; // 🌟 这一步机器人在final_field_node，但吸的是旁边的radiate_r2_id！
                        KFS_status[radiate_r2_id - 1] = KFS_TYPE_EMPTY;
                        grab_count++;
                    }
                }
            }
        }

        osEventFlagsSet(KFQEventHandle, EVT_INIT_MAP_GEN);
        flag = true;
    }
}

MF_Node* find_node(MF_Node* target_map, const int8_t id) {
    for (int i = 0; i < MFNum; i++) {
        if (target_map[i].id == id) return &target_map[i];
    }
    return NULL;
}

void generate_action_sequence(MF_Node* target_map, MF_Path* input_path, uint8_t path_len) {
    if (path_len == 0) { Action_Buffer_Size = 0; return; }

    Action_Buffer_Size = 0;

    // ==========================================
    // 1. 精准生成第 #00 步
    // ==========================================
    int8_t start_id = input_path[0].id;
    const MF_Node* start_node = find_node(target_map, start_id);

    if (start_node != NULL) {
        MF_ActionStep start_step;
        start_step.target_id = start_id;
        start_step.move = STAY;
        start_step.lift = LEVEL;
        start_step.target_height = start_node->height;
        start_step.grap = input_path[0].grap;

        // 🌟【新增相对方向解算】：
        start_step.arm_dir = ARM_FRONT; // 起跑台阶默认正前
        int8_t t_id = PATH_ACTUAL_TARGET_ID[0];
        if (t_id != -1) {
            uint8_t robot_c = start_node->c;
            uint8_t item_c  = (t_id - 1) % 3; // 计算道具所在的实际列号 (0-左, 1-中, 2-右)
            if (item_c < robot_c)       start_step.arm_dir = ARM_LEFT;
            else if (item_c > robot_c)  start_step.arm_dir = ARM_RIGHT;
        }

        Action_Buffer[Action_Buffer_Size++] = start_step;
    }

    // ==========================================
    // 2. 后续轨迹动作链生成
    // ==========================================
    for (int i = 0; i < path_len - 1; i++) {
        if (Action_Buffer_Size >= MAX_PATH_STEPS) break;

        int8_t curr_id = input_path[i].id;
        int8_t next_id = input_path[i+1].id;

        const MF_Node* curr = find_node(target_map, curr_id);
        const MF_Node* next = find_node(target_map, next_id);

        if (curr == NULL || next == NULL) continue;

        MF_ActionStep generic_step;
        generic_step.target_id = next_id;
        generic_step.target_height = next->height;
        generic_step.grap = input_path[i+1].grap;

        // 🌟【新增场内纵向侧吸相对几何方向精细解算】：
        generic_step.arm_dir = ARM_FRONT; // 默认前倾
        int8_t t_id = PATH_ACTUAL_TARGET_ID[i+1];
        if (t_id != -1) {
            uint8_t robot_c = next->c;       // 机器人走到下一个节点时的自身物理列号
            uint8_t item_c  = (t_id - 1) % 3; // 被吸道具的实际物理列号
            if (item_c < robot_c)       generic_step.arm_dir = ARM_LEFT;  // 道具偏左，左侧开火
            else if (item_c > robot_c)  generic_step.arm_dir = ARM_RIGHT; // 道具偏右，右侧开火
        }

        // 水平运动方向推导
        if (next->r > curr->r)       generic_step.move = FORWARD;
        else if (next->r < curr->r)  generic_step.move = BACKWARD;
        else if (next->c > curr->c)  generic_step.move = RIGHT;
        else if (next->c < curr->c)  generic_step.move = LEFT;
        else                         generic_step.move = STAY;

        // 举升结构推导
        if (next->height > curr->height)      generic_step.lift = UP;
        else if (next->height < curr->height) generic_step.lift = DOWN;
        else                                  generic_step.lift = LEVEL;

        Action_Buffer[Action_Buffer_Size++] = generic_step;
    }
}