#include "led.h"

void config_leds(){
	//turn on GPIOG clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
	//set dir to out
	GPIOG -> MODER |= GPIO_MODER_MODER13_0 + GPIO_MODER_MODER14_0;
}

// low-speed (program size) verison of led managing
void change_led_state(enum led_id ledid, enum led_action ledact){

	uint32_t reset_mask = 0;
	uint32_t set_mask = 0;
	uint32_t toogle_mask = 0;

	switch(ledid){
		case GREEN:
			reset_mask = GPIO_BSRR_BR_13;
			set_mask   = GPIO_BSRR_BS_13;
			toogle_mask= GPIO_ODR_ODR_13;
			break;

		case RED:
			reset_mask = GPIO_BSRR_BR_14;
			set_mask   = GPIO_BSRR_BS_14;
			toogle_mask= GPIO_ODR_ODR_14;
			break;
	}

	switch(ledact){
		case OFF:
			GPIOG -> BSRR |= reset_mask;
			break;
		case ON:
			GPIOG -> BSRR |= set_mask;
			break;
		case TOOGLE:
			GPIOG -> ODR ^= toogle_mask;
			break;
	}
}


