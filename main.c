#include <stm32f4xx.h>

void TIM1_UP_TIM10_IRQHandler();

unsigned int onoffleds;

int main()
{
	//GPIOG clock enabling
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
	//TIM10 clock enabling
	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	// set port to output
	GPIOG -> MODER |= GPIO_MODER_MODER13_0 + GPIO_MODER_MODER14_0;
	//
	NVIC -> ISER[0] |= 1 << (TIM1_UP_TIM10_IRQn);
	//
  DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM10_STOP;
	//
	TIM10 -> ARR = 1000; // 500 milliseconds
	TIM10 -> PSC = 0x0FFFFFF; // 16 MHz / 16 000
	TIM10 -> DIER |= TIM_DIER_UIE;
	TIM10 -> CR1 |= TIM_CR1_ARPE	+ TIM_CR1_CEN; 
	TIM10 -> EGR = 1;
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
	}
	TIM10 -> SR = 0x00;
}	
