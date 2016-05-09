#include <stm32f4xx.h>

void TIM1_UP_TIM10_IRQHandler();

unsigned int onoffleds;

int main()
{
	//SysClock - HSI 16 MHz
	//AHB = APB1 = APB2
	//GPIOG clock enabling, USART2 clock enabling
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->AHB1ENR |= RCC_APB1ENR_USART2EN;
	//TIM10 clock enabling
	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	// set port to output
	GPIOG -> MODER |= GPIO_MODER_MODER13_0 + GPIO_MODER_MODER14_0;
	// set USART2 TX/RX as pins' function
	GPIOA -> MODER |= GPIO_MODER_MODER2_1 + GPIO_MODER_MODER3_1; 
	GPIOA -> AFR[0] |= 0x7 << 4*2 | 0x7 << 4*3;
	// enable TIM1_UP_TIM10_IRQ
	NVIC -> ISER[0] |= 1 << (TIM1_UP_TIM10_IRQn);
  //NVIC -> ISER[0] |= 1 << (USART2_IRQn);
	//
	//stop timers while in breakpoint in debug
  DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM10_STOP;
	//USART2 -> SR ;
	USART2 -> CR1 |= USART_CR1_UE;
	USART2 -> CR2 |= 0x00;
	USART2 -> BRR = (104 << 4) | 3; // prescaler set to divide by 104,1875
	//send IDLE FRAME, enables transmission
	USART2 -> CR1 |= USART_CR1_TE;


	// TIM10 config
	TIM10 -> ARR = 1000; // 1000  milliseconds
	TIM10 -> PSC = 16000 - 1; // PCLK:16 MHz / 16 000 = 1 kHz
	TIM10 -> DIER |= TIM_DIER_UIE; //
	TIM10 -> CR1 |= TIM_CR1_ARPE	+ TIM_CR1_CEN; // control register
	TIM10 -> EGR = 1; // generate update event
	//
	for(;;){
		asm("NOP"); 
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
		USART2 -> DR = 'X';
	}
	TIM10 -> SR = 0x00;
}	
