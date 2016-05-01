#include <stm32f4xx.h>

int main()
{
	volatile unsigned int counter = 0;
	// AHB1 clock bus enable
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
	// set port to output
	GPIOG -> MODER |= GPIO_MODER_MODER13_0 + GPIO_MODER_MODER14_0;
	//turn on LEDS
	for(;;){
		GPIOG -> BSRR |= GPIO_BSRR_BS_13 + GPIO_BSRR_BS_14;
		for(counter = 0; counter < 0x0FFFFFF; ++counter) { }
		GPIOG -> BSRR |= GPIO_BSRR_BR_13 + GPIO_BSRR_BR_14;
		for(counter = 0; counter < 0x0FFFFFF; ++counter) { }
	}
	
	return 0;
}
