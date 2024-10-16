/*
 * RTOS_Porting.c
 *
 *  Created on: May 4, 2024
 *      Author: Abdelrahman Ibrahim
 */


#include"RTOS_Porting.h"



/**
 * test if thread mode or handler mode
 * set R0 PSP or MSP
 */


__attribute ((naked)) void SVC_Handler(){
	__asm("tst lr , #4\n\t"
			"ITE EQ \n\t"
			"mrseq r0,MSP\n\t"
			"mrsne r0,PSP\n\t"
			"B RTOS_svc_handler");
}



void HW_init(void)
{
	//Initialize Clock Tree (RCC -> SysTick Timer & CPU) 8MHZ
	//init HW
	//Clock tree
	//RCC Default Values makes CPU Clock & SySTick Timer clock = 8 M HZ

	//	8 MHZ
	//	1 count -> 0.125 us
	//	X count -> 1 ms
	//	X = 8000 Count



	//	decrease PenSV  interrupt priority to be  smaller than or equal  SySTICK Timer
	//SysTICK have a priority 14
	__NVIC_SetPriority(PendSV_IRQn, 15) ;

}



void trigger_OS_PendSV(void)
{
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk ;

}

void Start_Ticker(void)
{
	//	This clock tree should be defined in HW_init()
	//	8 MHZ
	//	1 count -> 0.125 us
	//	X count -> 1 ms
	//	X = 8000 Count

	 SysTick_Config(8000);


}


void SysTick_Handler(void)
{
	decide_what_next();
	//Switch Context & restore
	trigger_OS_PendSV();

}
