#ifndef _DEFINE_USART_H_
#define _DEFINE_USART_H_

#include <stm32f4xx.h>

const uint16_t LOG_TIME_SIZE_FORMAT; 

extern  volatile uint32_t systemTicks;

void USART2_IRQHandler();
void config_usart();
void put_log_mesg(char* mesg);

#endif
