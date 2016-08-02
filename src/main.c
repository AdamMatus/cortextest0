#include <stm32f4xx.h>
#include <stdio.h>
#include <core_cm4.h>
#include <stdlib.h>
#include <string.h>

#include "usart.h"
#include "led.h"
#include "gyro.h"
#include "L3GD20.h"

void TIM1_UP_TIM10_IRQHandler();
void put_log_mesg(char* mesg);
volatile uint32_t systemTicks;

unsigned int onoffleds;
volatile uint32_t LOG_LED_FLAG;

extern struct command_list serial_command_list;

void systick_config();
void tim10_config();

int main()
{
	//AHB = APB1 = APB2

	systick_config();

	tim10_config();

	config_leds();

	config_usart();

	gyro_init();

	serial_command_init();

	//******************************************************
	put_log_mesg("main loop starting in here"); 

	char r_str_word[SERIAL_PARSER_COMMAND_MAX_LENGTH];	
	while(1){
		if(LOG_LED_FLAG & 0x01){
			put_log_mesg("LEDS just TOOGLED! xD");
			LOG_LED_FLAG &= ~0x01;
			
			gyro_read_xyz(); //includes printing to serial port
		}

		if(new_mesg_flag){
			strncpy(r_str_word, get_serial_mesg(), SERIAL_PARSER_COMMAND_MAX_LENGTH);
			parse_serial_command(&serial_command_list, r_str_word);
			new_mesg_flag = 0;
		}
	}

	return 0;
}

#define TIM10_TIME_INTERVAL 3000

void tim10_config(){
	//TIM10 clock enabling
	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	// enable TIM1_UP_TIM10_IRQ
	NVIC -> ISER[0] |= 1 << (TIM1_UP_TIM10_IRQn);
 	//stop timers while in breakpoint in debug
  DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM10_STOP;
	// TIM10 config
	TIM10 -> ARR = TIM10_TIME_INTERVAL; // 1000  milliseconds
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
	
void systick_config(){
	//SysClock - HSI 16 MHz
	//SysTickConfig
	SysTick->LOAD = 16000000/(8*1000)-1;	
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk + SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler()
{
	systemTicks++;
}
