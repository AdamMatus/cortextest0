#include <stm32f4xx.h>
#include <stdio.h>
#include <core_cm4.h>

const uint16_t LOG_TIME_SIZE_FORMAT = 7;

void TIM1_UP_TIM10_IRQHandler();
void put_log_mesg(char* mesg);
volatile uint32_t systemTicks;

unsigned int onoffleds;
volatile uint32_t LOG_LED_FLAG;
char recived_char;



int main()
{
	//SysClock - HSI 16 MHz
	//SysTickConfig
	SysTick->LOAD = 16000000/(8*1000)-1;	
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk + SysTick_CTRL_ENABLE_Msk;

	//AHB = APB1 = APB2
	//GPIOG clock enabling, USART2 clock enabling
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	//
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	//TIM10 clock enabling
	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	// set port to output
	GPIOG -> MODER |= GPIO_MODER_MODER13_0 + GPIO_MODER_MODER14_0;
	// set USART2 TX/RX as pins' function
	GPIOD -> MODER |= GPIO_MODER_MODER5_1 + GPIO_MODER_MODER6_1; 
	GPIOD -> PUPDR |= GPIO_PUPDR_PUPDR5_0;
	GPIOD -> OTYPER |= GPIO_OTYPER_OT_6; 
	GPIOD -> AFR[0] |= 0x7 << 4*5 | 0x7 << 4*6;
	// enable TIM1_UP_TIM10_IRQ
	NVIC -> ISER[0] |= 1 << (TIM1_UP_TIM10_IRQn);
	NVIC -> ISER[1] |= 1 << (USART2_IRQn - 8*sizeof(NVIC->ISER[0]));
	//
	//stop timers while in breakpoint in debug
  DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM10_STOP;
	//USART2 -> SR ;
	USART2 -> CR1 |= USART_CR1_UE;
	USART2 -> CR2 |= 0x00;
	USART2 -> BRR = (104 << 4) | 3; // prescaler set to divide by 104,1875

	// TIM10 config
	TIM10 -> ARR = 1000; // 1000  milliseconds
	TIM10 -> PSC = 16000 - 1; // PCLK:16 MHz / 16 000 = 1 kHz
	TIM10 -> DIER |= TIM_DIER_UIE; //
	TIM10 -> CR1 |= TIM_CR1_ARPE	+ TIM_CR1_CEN; // control register
	TIM10 -> EGR = 1; // generate update event
	//
	
	USART2 ->CR1 |=  USART_CR1_TCIE + USART_CR1_RXNEIE +  USART_CR1_RE;


	while(1){
		if(LOG_LED_FLAG & 0x01){
			put_log_mesg("LEDS just TOOGLED! xD\n");
			LOG_LED_FLAG &= ~0x01;
		}
	}

	
	return 0;
}

void TIM1_UP_TIM10_IRQHandler()
{
	if(TIM10 -> SR & 0x01){
		if(onoffleds%2)
			GPIOG -> BSRR |= GPIO_BSRR_BS_13 + GPIO_BSRR_BS_14;
		else
			GPIOG -> BSRR |= GPIO_BSRR_BR_13 + GPIO_BSRR_BR_14;

		onoffleds++;

		LOG_LED_FLAG |= 0x01;
	}
	TIM10 -> SR = 0x00;
}	

void USART2_IRQHandler()
{
	uint32_t stat_reg = USART2->SR ; 
	if(stat_reg &  USART_SR_TC){
		USART2 -> DR = '\r';
		USART2 -> CR1 &= ~USART_CR1_TE;
	}	
	
	if(stat_reg &  USART_SR_RXNE){
		recived_char = USART2->DR;
	}
}	

void SysTick_Handler()
{
	systemTicks++;
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
	
	for(uint32_t i = 0; i < LOG_TIME_SIZE_FORMAT - 6; i++){ // 6 - six signes generated yet
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
