/*
 * porting.h
 *
 *  Created on: May 4, 2024
 *      Author: Abdelrahman Ibrahim
 */

#ifndef MYRTOS_MYRTOS_INC_RTOS_PORTING_H_
#define MYRTOS_MYRTOS_INC_RTOS_PORTING_H_


#include "../CMSIS/INC/ARMCM3.h"

extern int _estack ;
extern int _eheap  ;
#define MainStackSize 	3072




#define OS_Set_PSP(Add)     __asm volatile("mov r0, %[C_VAL] \n\t msr PSP, r0" : : [C_VAL] "r" (Add) )
#define OS_Get_PSP(Get)     __asm volatile("mrs r0, PSP \n\t mov %[C_VAL], r0"   : [C_VAL] "=r" (Get) )
#define Switch_MSP_to_PSP() __asm volatile("mrs r0, CONTROL \n\t  mov r1, #0x02 \n\t orr r0, r0, r1 \n\t msr CONTROL, r0")
#define Switch_PSP_to_MSP() __asm volatile("mrs r0, CONTROL \n\t  mov r1, #0x05 \n\t and r0, r0, r1 \n\t msr CONTROL, r0")

//Manipulate with mode
#define Switch_Privileged_to_unPrivileged() __asm volatile("mrs r3, CONTROL \n\t orr r3, r3, #0x1 \n\t msr CONTROL, r3")
#define Switch_unPrivileged_to_Privileged() __asm volatile("mrs r3, CONTROL \n\t lsr r3, #0x1 \n\t lsl r3, #0x1 \n\t msr CONTROL, r3")

void HW_init(void);

void trigger_OS_PendSV(void);

void Start_Ticker(void);

#endif /* MYRTOS_MYRTOS_INC_RTOS_PORTING_H_ */
