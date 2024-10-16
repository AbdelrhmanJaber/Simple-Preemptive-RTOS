/*
 * scheduler.h
 *
 *  Created on: May 4, 2024
 *      Author: Abdelrahman Ibrahim
 */

#ifndef MYRTOS_MYRTOS_INC_RTOS_SCHEDULER_H_
#define MYRTOS_MYRTOS_INC_RTOS_SCHEDULER_H_


#include"RTOS_Porting.h"
#include <stdint.h>


#define MAX_NUMBER_OF_TASKS            10

/*create tcp for each task*/

typedef enum taskEnum{
	RUUNING,
	SUSPEND,
	WAITING,
	READY
}task_current_status;


typedef enum task_wait_helper{
	ENABLE_BLOCK,
	DISABLE_BLOCK
}blocking_task_status;

typedef enum myErrorStruct{
	NO_ERROR,
	EXCEED_STACK_SIZE,
	READY_QUEUE_ERROR,
	MAX_MUTEX_USERS
}RTOS_error_t;


typedef struct myStruct_task_wait{
	uint32_t tick_count;
	blocking_task_status status;
}task_wait_t;



typedef struct muStruct_task{
	uint32_t stack_size;
	uint8_t priority;
	uint32_t _s_PSP_task;
	uint32_t _e_PSP_task;
	uint32_t * current_psp_task;
	void(*pf_task_entry)(void);
	uint8_t autostart;
	char task_name[20]; //for debugging
	task_wait_t task_wait;
	task_current_status task_status;
}tcp;

typedef struct
{
	uint8_t*   Ppayload;
	uint32_t   PayloadSize;
	tcp*  CurrentUser;
	tcp*  NextUser;
	uint8_t    MutexName[30];

}mutex;

typedef enum{
	SVC_Activatetask,
	SVC_terminateTask,
	SVC_TaskWaitingTime,
	SVC_AquireMutex,
	SVC_ReleaseMutex
}SVC_ID_t;


RTOS_error_t RTOS_init(void);

void RTOS_create_main_stack(void);

void RTOS_create_idle_task(void);

RTOS_error_t RTOS_create_task(tcp* task_ref);

RTOS_error_t RTOS_create_task_stack(tcp* task_ref);

void sort_tasks_priority(void);

void activate_task(tcp * task_ref);

void terminate_task(tcp * task_ref);

void RTOS_set_svc(SVC_ID_t id);

void RTOS_svc_handler(int * frame);

void decide_what_next(void);

void RTOS_update_schedule_table(void);

void Update_TaskWaitingTime (void);

void RTOS_TaskWaitingTime (uint32_t ticksNumber, tcp * Tref);

void RTOS_StartOS (void);

RTOS_error_t RTOS_AcquireMutex (tcp* Tref, mutex* Mref);

void RTOS_ReleaseMutex (mutex* Mref);


#endif /* MYRTOS_MYRTOS_INC_RTOS_SCHEDULER_H_ */
