#include "usart.h"

const uint16_t LOG_TIME_SIZE_FORMAT = 9; 
unsigned int rs_index;
char received_string[128];

void config_usart(){
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	USART2 -> CR1 |= USART_CR1_UE;
	USART2 -> CR2 |= 0x00;
	USART2 -> BRR = (104 << 4) | 3; // prescaler set to divide by 104,1875

	USART2 ->CR1 |=  USART_CR1_TCIE + USART_CR1_RXNEIE +  USART_CR1_RE;
}

void put_log_mesg(char* mesg){
	//put clock log
	char data_string[LOG_TIME_SIZE_FORMAT + 1]; // additional sign ' '
	uint32_t sys_tick_copy = systemTicks/1000; //seconds asumming that stick is 1ms

	uint32_t index = 0;
	data_string[index++] = ' '; // separator at the end of the time ( xD )

	uint32_t modulosixty;
	for(uint32_t i = 0; i<2; i++){
		modulosixty = sys_tick_copy%60;
		data_string[index++] = '0' + modulosixty%10;
  	modulosixty/=10;
  	data_string[index++] = '0' + modulosixty;
		sys_tick_copy/=60; //minutes after i=0 and hours after i=1
		data_string[index++] = ':';
	}
	
	for(uint32_t i = LOG_TIME_SIZE_FORMAT - index; i > 0; --i){
		data_string[index++] = '0' + sys_tick_copy%10;
		sys_tick_copy/=10;
	}	

 	USART2 -> CR1 |= USART_CR1_TE ;

	while(index--){ // print backfoward, fuck script kiddies im the hacker
		while( !(USART2->SR & USART_SR_TXE) );
		USART2 -> DR = data_string[index];
	}

	while(*mesg){
		while( !(USART2->SR & USART_SR_TXE) );
		USART2 -> DR = *mesg;
		mesg++;
	}
}

void USART2_IRQHandler()
{

	uint32_t stat_reg = USART2->SR ; 
	// tx handling
	if(stat_reg &  USART_SR_TC){
		USART2 -> DR = '\r';
		USART2 -> CR1 &= ~USART_CR1_TE;
	}


	// rx handling
	if(stat_reg &  USART_SR_RXNE){
		received_string[rs_index] = USART2->DR;
		if(received_string[rs_index++] == 0 ){
			//received_string[rs_index] = 0;
			rs_index = 0;
	}
	
}


}
