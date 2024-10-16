#ifndef RTOS_DEBUG
#define RTOS_DEBUG

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define UART2_BAUD_RATE 9600
#define UART2_TX_PIN  (1 << 2)
#define UART2_RX_PIN  (1 << 3)

#define RCC_BASE       0x40021000
#define RCC_APB2ENR   (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_APB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x1C))
#define GPIOA_BASE     0x40010800
#define GPIOA_CRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x00))

#define USART2_BASE    0x40004400
#define USART2_SR      (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR      (*(volatile uint32_t *)(USART2_BASE + 0x04))
#define USART2_BRR     (*(volatile uint32_t *)(USART2_BASE + 0x08))
#define USART2_CR1     (*(volatile uint32_t *)(USART2_BASE + 0x0C))

void UART2_Init(void);
void UART2_SendByte(uint8_t byte);
void UART2_SendString(const char* str);
uint8_t UART2_ReceiveByte(void);
void rtos_debug_print_message(char *format, ...);

#endif
