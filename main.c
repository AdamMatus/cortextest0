#include <stm32f4xx.h>

void TIM1_UP_TIM10_IRQHandler();

unsigned int onoffleds;
char csttring_to_send[] = "pies to jest";
volatile uint32_t USART2_TX_FLAG;
char recived_char;

int main()
{
	USART2_TX_FLAG = 0;
	//SysClock - HSI 16 MHz
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
	
	USART2 ->CR1 |=  USART_CR1_TCIE +  USART_CR1_RE;
	while(1){

		if(USART2_TX_FLAG)
		{
			char* sindex = csttring_to_send;
			
			USART2 -> CR1 |= USART_CR1_TE ;

			while(*sindex){
				while( !(USART2->SR & USART_SR_TXE) );
				USART2 -> DR = *sindex;
				sindex++;
			}
			USART2_TX_FLAG = 0;
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

		USART2_TX_FLAG = 1;

		}
	TIM10 -> SR = 0x00;
}	

void USART2_IRQHandler()
{
	uint32_t stat_reg = USART2->SR ; 
	if(stat_reg &  USART_SR_TC){
		USART2 -> DR = ' ';
		USART2 -> CR1 &= ~USART_CR1_TE;
	}	
	
	if(stat_reg &  USART_SR_RXNE){
		recived_char = USART2->DR;
	}
}	
