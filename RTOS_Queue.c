/*
 * RTOS_Queue.c
 *
 *  Created on: May 14, 2024
 *  Author: Abdelrahman Ibrahim
 */


//#include <stdio.h>


#include"RTOS_Queue.h"



QUEUE_ERROR_TYPE RTOS_queue_init(queue_node_t* fifo_node , QUEUE_ENTRY * buffer , uint32_t lenght ){
    QUEUE_ERROR_TYPE error_check = QUEUE_NO_ERROR;
    if(! (fifo_node->base && lenght) ) error_check = QUEUE_NULL;
    fifo_node->size = 0;
    fifo_node->lenght = lenght;
    fifo_node->base = buffer;
    fifo_node->rear = buffer;
    fifo_node->front = buffer;
    return error_check;
}

QUEUE_ERROR_TYPE RTOS_queue_enqueue(queue_node_t * fifo_node , QUEUE_ENTRY  data){
    QUEUE_ERROR_TYPE error_check = QUEUE_NO_ERROR;
    if(RTOS_queue_IS_FULL(fifo_node) == QUEUE_FULL) error_check = QUEUE_FULL;
    else{
        *(fifo_node->rear) = data;
        fifo_node->size++;
        if(fifo_node->rear == (fifo_node->base + fifo_node->lenght * sizeof(QUEUE_ENTRY)) - 4 )
            fifo_node->rear = fifo_node->base;
        else
            fifo_node->rear++;
    }
    return error_check;
}

QUEUE_ERROR_TYPE RTOS_queue_dequeue(queue_node_t * fifo_node , QUEUE_ENTRY*  return_data){
    QUEUE_ERROR_TYPE error_check = QUEUE_NO_ERROR;
    if(RTOS_queue_IS_EMPTY(fifo_node) == QUEUE_EMPTY){
        error_check = QUEUE_EMPTY;
    }
    else{
        *(return_data) = *(fifo_node->front);
        fifo_node->size--;
        /*check circular*/
        if(fifo_node->front == (fifo_node->base + fifo_node->lenght * sizeof(QUEUE_ENTRY)) - 4 )
            fifo_node->front = fifo_node->base;
        else
            fifo_node->front++;
    }
    return error_check;
}


QUEUE_ERROR_TYPE RTOS_queue_IS_FULL(queue_node_t * fifo_node){
    QUEUE_ERROR_TYPE error_check = QUEUE_NO_ERROR;
    if(fifo_node->size >= fifo_node->lenght) error_check = QUEUE_FULL;
    return error_check;
}

QUEUE_ERROR_TYPE RTOS_queue_IS_EMPTY(queue_node_t * fifo_node){
    QUEUE_ERROR_TYPE error_check = QUEUE_NO_ERROR;
    if(!fifo_node->size) error_check = QUEUE_EMPTY;
    return error_check;
}



