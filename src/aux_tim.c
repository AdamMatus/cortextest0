#include <stdlib.h>
#include <stm32f4xx.h>
#include "aux_tim.h"
#include "led.h"
#include "usart.h"

static const uint32_t aux_tim_init_period = 5000; //in miliseconds
static unsigned int onoffleds;

volatile uint32_t LOG_LED_FLAG;


void tim10_config(){
	//TIM10 clock enabling
	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	// enable TIM1_UP_TIM10_IRQ
	NVIC -> ISER[0] |= 1 << (TIM1_UP_TIM10_IRQn);
 	//stop timers while in breakpoint in debug
  DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM10_STOP;
	// TIM10 config
	TIM10 -> ARR = aux_tim_init_period;
	TIM10 -> PSC = 16000 - 1; // PCLK:16 MHz / 16 000 = 1 kHz
	TIM10 -> DIER |= TIM_DIER_UIE; //
	TIM10 -> CR1 |= TIM_CR1_ARPE	+ TIM_CR1_CEN; // control register
	TIM10 -> EGR = 1; // generate update event
}

void TIM1_UP_TIM10_IRQHandler()
{
	if(TIM10 -> SR & 0x01){
		if(onoffleds%2){
			change_led_state(GREEN, ON);
		}
		else{
			change_led_state(GREEN, OFF);
		}

		toogle_leds(RED_LED_TOOGLE_MASK);
		onoffleds++;

		LOG_LED_FLAG |= 0x01;
	}
	TIM10 -> SR = 0x00;
}

void update_aux_tim_period(uint32_t period){
	TIM10 -> ARR = period;
}

void serial_command_aux_tim_handler(char* r_str, int index){
	r_str = r_str + index;
	uint32_t new_period = (uint32_t)strtol(r_str, NULL, 10); 
	if(!new_period)
		put_log_mesg("Wrong time format.");
	else
		update_aux_tim_period(new_period);
}
