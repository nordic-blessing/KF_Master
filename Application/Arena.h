/**
 * @file    Arena.h
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/25
 */

#ifndef ARENA_H
#define ARENA_H

// 斜坡
#define RAMP_DOWN_X     5.250f
#define RAMP_DOWN_Y     9.000f
#define RAMP_UP_X       5.250f
#define RAMP_UP_Y      11.000f

// Arena
#define ARENA_WAIT_X    2.400f
#define ARENA_WAIT_Y   10.700f
#define TTT_RACK_1_X    1.000f
#define TTT_RACK_1_Y   10.160f
#define TTT_RACK_2_X    1.000f
#define TTT_RACK_2_Y   10.700f
#define TTT_RACK_3_X    1.000f
#define TTT_RACK_3_Y   11.240f

void Arena_Task(void);

#endif //ARENA_H
