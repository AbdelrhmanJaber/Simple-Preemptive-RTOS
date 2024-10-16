#include "rtos_debug.h"

static uint32_t USART_BRR(uint32_t baud_rate) {
    uint32_t apb1_clock = 8000000;
    return (apb1_clock + (baud_rate / 2)) / baud_rate;
}

void UART2_Init(void) {
    RCC_APB2ENR |= (1 << 2);
    RCC_APB1ENR |= (1 << 17);

    GPIOA_CRL &= ~(0xF << 8);
    GPIOA_CRL |= (0xB << 8);

    GPIOA_CRL &= ~(0xF << 12);
    GPIOA_CRL |= (0x4 << 12);

    USART2_BRR = USART_BRR(UART2_BAUD_RATE);

    USART2_CR1 |= (1 << 13);
    USART2_CR1 |= (1 << 3);
    USART2_CR1 |= (1 << 2);
}

void UART2_SendByte(uint8_t byte) {
    while (!(USART2_SR & (1 << 7)));
    USART2_DR = byte;
}

void UART2_SendString(const char* str) {
    while (*str) {
        UART2_SendByte((uint8_t)(*str++));
    }
}

uint8_t UART2_ReceiveByte(void) {
    while (!(USART2_SR & (1 << 5)));
    return (uint8_t)(USART2_DR);
}


void rtos_debug_print_message(char *format, ...) {
    char message[100] = {0};
    va_list args;
    va_start(args, format);
    vsprintf(message, format, args);
    UART2_SendString(message);
    va_end(args);
}
