/*
 * RTOS_Queue.h
 *
 *  Created on: oct 4, 2024
 *  Author: Abdelrahman Ibrahim
 */

#ifndef RTOS_QUEUE_H_
#define RTOS_QUEUE_H_

#include <stdint.h>
#include"RTOS_Scheduler.h"

#define QUEUE_ENTRY          tcp*


typedef enum myEnym{
    QUEUE_NO_ERROR,
    QUEUE_FULL,
    QUEUE_EMPTY,
    QUEUE_NULL
}QUEUE_ERROR_TYPE;


typedef struct myStruct{
    uint32_t size;
    uint32_t lenght;
    QUEUE_ENTRY * front;
    QUEUE_ENTRY * rear;
    QUEUE_ENTRY * base;
}queue_node_t;




QUEUE_ERROR_TYPE RTOS_queue_init(queue_node_t* fifo_node , QUEUE_ENTRY * buffer , uint32_t lenght );
QUEUE_ERROR_TYPE RTOS_queue_enqueue(queue_node_t * fifo_node , QUEUE_ENTRY  data);
QUEUE_ERROR_TYPE RTOS_queue_dequeue(queue_node_t * fifo_node , QUEUE_ENTRY*  return_data);
QUEUE_ERROR_TYPE RTOS_queue_IS_FULL(queue_node_t * fifo_node);
QUEUE_ERROR_TYPE RTOS_queue_IS_EMPTY(queue_node_t * fifo_node);
QUEUE_ERROR_TYPE RTOS_queue_print_debug(queue_node_t * fifo_node);


#endif /* MYRTOS_MYRTOS_INC_RTOS_QUEUE_H_ */
