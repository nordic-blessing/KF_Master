/**
 * @file    CommandTask.h
 * @author  Icol_Lee (icolboom4@gmail.com)
 * @brief   
 * @version 0.1
 * @date    2026/5/25
 */

#ifndef COMMANDTASK_H
#define COMMANDTASK_H

#include <stdint.h>

void CommandSendPoint(float x, float y, float yaw, uint8_t start);

void CommandSendCatchPrepare(void);
void CommandSendCatch(void);
void CommandSendRelease(void);

void CommandSendGrab(uint8_t grab_flag, uint8_t grab_num);
void CommandSendLift(uint8_t lift_flag);

void CommandSendPut(uint8_t flag);
void CommandSendLiftR1(void);

#endif //COMMANDTASK_H
