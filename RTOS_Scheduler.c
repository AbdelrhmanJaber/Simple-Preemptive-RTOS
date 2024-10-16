/*
 * RTOS_Scheduler.c
 *
 *  Created on: oct 10, 2024
 *      Author: Abdelrahman Ibrahim
 */



#include"RTOS_Scheduler.h"
#include"RTOS_Queue.h"
#include<string.h>




/*KERNEL STATUS ENUM*/

typedef enum{
		RTOS_RUNNING,
		RTOS_SUSPEND
}kernel_status_t;


struct {
	tcp * rtos_task[MAX_NUMBER_OF_TASKS];
	uint64_t _s_MSP_kernel; //end of the descending stack
	uint64_t _e_MSP_kernel; //end of kernel stack
	uint32_t psp_task;
	uint8_t number_of_active_tasks;
	tcp * current_task; //current running task
	tcp * next_task;  	//next task for context switching
	kernel_status_t status;
}RTOS_kernel;


//create queue for tasks
queue_node_t ready_queue_node;
//ready queue for static memory management fot tasks
//OS tasks will allocate in the stack of kernel
tcp* readyQueue[MAX_NUMBER_OF_TASKS];
tcp idle_task;


/*init RTOS
 * Create Main stack
 * Create idle task
 * HW INIT*/

RTOS_error_t RTOS_init(void){
	RTOS_error_t error = NO_ERROR;
	/*set os mode -> suspend*/
	RTOS_kernel.status = RTOS_SUSPEND;
	RTOS_kernel.number_of_active_tasks = 0;
	/*Create Main stack*/
	RTOS_create_main_stack();
	/*create ready queue*/
	if(RTOS_queue_init(&ready_queue_node , readyQueue , MAX_NUMBER_OF_TASKS) != QUEUE_NO_ERROR){
		error = READY_QUEUE_ERROR;
	}
	/*config and create idle task*/
	idle_task.pf_task_entry = RTOS_create_idle_task;
	idle_task.stack_size = 300;
	idle_task.priority = 255;
	strcpy(idle_task.task_name , "idleTask");

	RTOS_create_task(&idle_task);
	HW_init();
	return error;
}

/*Main stack for the kernel
 * owns ready queue and scheduler queue
 **/

void RTOS_create_main_stack(void){
	RTOS_kernel._s_MSP_kernel = (uint64_t *)&_estack ;
	RTOS_kernel._e_MSP_kernel = (RTOS_kernel._s_MSP_kernel - MainStackSize) ;
	RTOS_kernel.psp_task = (RTOS_kernel._e_MSP_kernel - 8) ; //POINT TO FIRST TASK
}

/*idle stack to run when no task runs*/
void RTOS_create_idle_task(void){
	while(1){
		__asm("NOP");
	}
}

/*Create new task
 * set its start and its end statically
 * set its shadow psp to access it by the actual PSP in its turn
 * */

RTOS_error_t RTOS_create_task(tcp* task_ref){
	RTOS_error_t error = NO_ERROR;
	task_ref->_s_PSP_task = RTOS_kernel.psp_task;
	task_ref->_e_PSP_task = (task_ref->_s_PSP_task - task_ref->stack_size);
	/*check stack overflow*/
	if(task_ref->_e_PSP_task < (uint32_t) _estack){
		error = EXCEED_STACK_SIZE;
	}
	RTOS_kernel.psp_task = (task_ref->_e_PSP_task - 8); //address of next task
	RTOS_create_task_stack(task_ref);
	/*task state*/
	task_ref->task_status = SUSPEND;
	/*update task table*/
	RTOS_kernel.rtos_task[RTOS_kernel.number_of_active_tasks++] = task_ref;
	return error;
}


/*Create a stack for the created task
 *init the non memory mapped registers required for each task */

RTOS_error_t RTOS_create_task_stack(tcp* task_ref){

	RTOS_error_t error = NO_ERROR;

	/*set dummy values for stack of task*/
	task_ref->current_psp_task = (uint32_t*)(&(task_ref->_s_PSP_task)) ;

	task_ref->current_psp_task--;
	*(task_ref->current_psp_task) = 0x01000000 ; //T=1 for XPSR

	task_ref->current_psp_task--;
	*(task_ref->current_psp_task) = (uint32_t)task_ref->pf_task_entry; //pc = address of task to execute it

	task_ref->current_psp_task--;
	*(task_ref->current_psp_task) = 0xFFFFFFFD ; //for LR (thread mode)

	for(uint8_t i = 0 ; i < 13 ; i++){
		task_ref->current_psp_task--;
		*(task_ref->current_psp_task) = 0;
	}

	return error;

}




/*re arrange tasks according to their priority
 * the lowest number is the highest priority*/

void sort_tasks_priority(void)
{
	tcp* temp ;
	uint8_t n = RTOS_kernel.number_of_active_tasks ;
	for (uint8_t i = 0; i < n - 1; i++)
		for (uint8_t j = 0; j < n - i - 1; j++)
			if (RTOS_kernel.rtos_task[j]->priority > RTOS_kernel.rtos_task[j + 1]->priority)
			{
				temp = RTOS_kernel.rtos_task[j] ;
				RTOS_kernel.rtos_task[j] = RTOS_kernel.rtos_task[j + 1 ] ;
				RTOS_kernel.rtos_task[j + 1] = temp ;
			}

}



void RTOS_set_svc(SVC_ID_t id){
	switch(id){
		case SVC_Activatetask:
			__asm("svc #0x00");
		break;
		case SVC_terminateTask:
			__asm("svc #0x01");
		break;
		case SVC_TaskWaitingTime:
			__asm("svc #0x02");
		break;
		case SVC_AquireMutex:
			__asm("svc #0x03");
		break;
		case SVC_ReleaseMutex:
			__asm("svc #0x04");
		break;
	}
}



void RTOS_svc_handler(int * frame){
	/*GET THE SVC INSTRUCTION FROM PC*/
	/* R0
	 * R1
	 * R2
	 * R3
	 * R4
	 * LR
	 * PC
	 * xPSR*/
	uint8_t svc_id = *((uint8_t *)((uint8_t *)frame[6] - 2)) ;
	switch(svc_id){
		case SVC_Activatetask:
		case SVC_terminateTask:
			RTOS_update_schedule_table();
			if(RTOS_kernel.status == RTOS_RUNNING){
				/*IF NOT IDLE TASK*/
				if(strcmp(RTOS_kernel.current_task->task_name , "idleTask") != 0){
					/*decide what next task to run*/
					decide_what_next();
					/*set pendsv*/
					trigger_OS_PendSV();
				}
			}
		break;
		case SVC_TaskWaitingTime:
			RTOS_update_schedule_table(); //--------->/* Update Scheduler Table */
		break;

	}
}


void decide_what_next(void){
	/*if ready queue is empty*/
	/*enqueue current task*/
	if(ready_queue_node.size == 0 && RTOS_kernel.current_task->task_status == SUSPEND){
		RTOS_kernel.current_task->task_status = RUUNING;
		RTOS_queue_enqueue(&ready_queue_node, RTOS_kernel.current_task);
		RTOS_kernel.next_task = RTOS_kernel.current_task;
	}
	else{
		RTOS_queue_dequeue(&ready_queue_node, &RTOS_kernel.next_task);
		RTOS_kernel.next_task = RTOS_kernel.current_task;
		if(RTOS_kernel.next_task->priority == RTOS_kernel.current_task->priority &&
				RTOS_kernel.current_task->task_status != SUSPEND){
			RTOS_queue_enqueue(&ready_queue_node, RTOS_kernel.current_task);
			RTOS_kernel.current_task->task_status = READY;
		}
	}
}



void RTOS_update_schedule_table(void){
	/*update list of os tasks*/
	tcp  * temp = NULL;
	tcp * current_task;
	tcp * next_task;
	/*sort list according to priority*/
	sort_tasks_priority();
	/*free ready queue*/
	while(RTOS_queue_dequeue(&ready_queue_node, &temp) != QUEUE_EMPTY);
	/*update ready queue*/
	for(uint8_t i = 0 ; i < RTOS_kernel.number_of_active_tasks ; i++){
		current_task = RTOS_kernel.rtos_task[i];
		next_task = RTOS_kernel.rtos_task[i+1];
		if(current_task->task_status != SUSPEND){
			if(next_task->task_status == SUSPEND){
				RTOS_queue_enqueue(&ready_queue_node, current_task);
				current_task->task_status = READY;
				break;
			}
			/*higher priority*/
		    if(current_task->priority < next_task->priority){
		    	RTOS_queue_enqueue(&ready_queue_node, current_task);
		    	current_task->task_status = READY;
		    	break;
		    }
		    else if(current_task->priority == next_task->priority){
		    	RTOS_queue_enqueue(&ready_queue_node, current_task);
		    	current_task->task_status = READY;
		    }
		}

	}
}



/*activate task*/

void activate_task(tcp * task_ref){
	task_ref->task_status = WAITING ;
	RTOS_set_svc(SVC_Activatetask);
}

/*terminate task*/


void terminate_task(tcp * task_ref){
	task_ref->task_status = SUSPEND ;
	RTOS_set_svc(SVC_terminateTask);
}


__attribute ((naked)) void PendSV_Handler()
{
	/*GET THE PSP TO STORE THE CURRENT TASK CONTEXT*/
	OS_Get_PSP(RTOS_kernel.current_task->current_psp_task);

	//using this Current_PSP (Pointer) to store (R4 to R11)
	RTOS_kernel.current_task->current_psp_task-- ;
	__asm volatile("mov %0,r4 " : "=r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task-- ;
	__asm volatile("mov %0,r5 " : "=r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task-- ;
	__asm volatile("mov %0,r6 " : "=r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task-- ;
	__asm volatile("mov %0,r7 " : "=r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task-- ;
	__asm volatile("mov %0,r8 " : "=r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task-- ;
	__asm volatile("mov %0,r9 " : "=r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task-- ;
	__asm volatile("mov %0,r10 " : "=r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task-- ;
	__asm volatile("mov %0,r11 " : "=r" (*(RTOS_kernel.current_task->current_psp_task))  );



	if (RTOS_kernel.next_task != NULL){
		RTOS_kernel.current_task = RTOS_kernel.next_task ;
	RTOS_kernel.next_task = NULL ;
	}

	/*RESTORE THE CONTEXT OF THE NEXT TASK THAT SHOULD BE RUN*/
	__asm volatile("mov r11,%0 " : : "r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task++ ;
	__asm volatile("mov r10,%0 " : : "r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task++ ;
	__asm volatile("mov r9,%0 " : : "r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task++ ;
	__asm volatile("mov r8,%0 " : : "r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task++ ;
	__asm volatile("mov r7,%0 " : : "r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task++ ;
	__asm volatile("mov r6,%0 " : : "r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task++ ;
	__asm volatile("mov r5,%0 " : : "r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task++ ;
	__asm volatile("mov r4,%0 " : : "r" (*(RTOS_kernel.current_task->current_psp_task))  );
	RTOS_kernel.current_task->current_psp_task++ ;

	//update PSP
	OS_Set_PSP(RTOS_kernel.current_task->current_psp_task);
	/*Branch for link register to switch to PSP to run next task*/
	__asm volatile("BX LR");

}


void RTOS_StartOS (void)
{
	RTOS_kernel.status = RTOS_RUNNING;  /* Change OS state */

	RTOS_kernel.current_task = &idle_task;

	RTOS_kernel.next_task = &idle_task;

	activate_task(&idle_task);  /* Activate task */

	Start_Ticker();  /* Start ticker */

	OS_Set_PSP(RTOS_kernel.current_task->current_psp_task);

	Switch_MSP_to_PSP();  /* Switch to PSP */

	Switch_Privileged_to_unPrivileged();  /* Switch to unprivileged */

	RTOS_kernel.current_task->pf_task_entry();


}



/*update task waiting counter
 * task will be suspended
 * with each timer interrupt
 * the counter increases until reach the wanted counts
 * then make the task waiting for ready queue*/


void Update_TaskWaitingTime (void)
{
	for (uint8_t i = 0; i < RTOS_kernel.number_of_active_tasks; i++)
	{
		if (RTOS_kernel.rtos_task[i]->task_status == SUSPEND)
		{
			if (RTOS_kernel.rtos_task[i]->task_wait.status == ENABLE_BLOCK) //It is blocking until meet the time line
			{
				if (RTOS_kernel.rtos_task[i]->task_wait.tick_count == 0)
				{
					RTOS_kernel.rtos_task[i]->task_wait.status = DISABLE_BLOCK;
					RTOS_kernel.rtos_task[i]->task_status = WAITING;
					RTOS_set_svc(SVC_TaskWaitingTime);
				}
				RTOS_kernel.rtos_task[i]->task_wait.tick_count--;
			}
		}
	}
}


void RTOS_TaskWaitingTime (uint32_t ticksNumber, tcp * Tref)
{
	Tref->task_wait.status = ENABLE_BLOCK;  //Enable blocking

	Tref->task_wait.tick_count = ticksNumber;  //Pass The Ticks numbers

	Tref->task_status = SUSPEND;  //update Task state

	RTOS_set_svc(SVC_terminateTask);  //Terminate task

}



RTOS_error_t RTOS_AcquireMutex (tcp* Tref, mutex* Mref)
{
	if (Mref->CurrentUser == NULL)
	{
		Mref->CurrentUser = Tref;

	} else if (Mref->NextUser == NULL)
	{
		Mref->NextUser = Tref;

		Tref->task_status = SUSPEND;

		RTOS_set_svc(SVC_terminateTask);

	} else
	{
		return MAX_MUTEX_USERS;
	}
	return NO_ERROR;

}

void RTOS_ReleaseMutex (mutex* Mref)
{
	if (Mref->CurrentUser != NULL)
	{
		Mref->CurrentUser = Mref->NextUser;
		Mref->NextUser = NULL;

		Mref->CurrentUser->task_status = WAITING;

		RTOS_set_svc(SVC_Activatetask);
	}
}
