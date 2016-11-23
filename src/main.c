#include <stm32f4xx.h>
#include <stdio.h>
#include <core_cm4.h>
#include <stdlib.h>
#include <string.h>

#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "gyro.h"
#include "L3GD20.h"
#include "stm32f429I-disc-board-spi.h"

void TIM1_UP_TIM10_IRQHandler();
void put_log_mesg(char* mesg);
volatile uint32_t systemTicks;

extern uint32_t LOG_LED_FLAG;
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

	discf429_spi_config();

	gyro_init();

	lcd_init();

	serial_command_init();

	//******************************************************
	put_log_mesg("main loop starting in here"); 

	char r_str_word[SERIAL_PARSER_COMMAND_MAX_LENGTH];	
	while(1){
		if(LOG_LED_FLAG & 0x01){
			put_log_mesg("LEDS just TOOGLED! xD");
			LOG_LED_FLAG &= ~0x01;
			gyro_print_xyz();
		}

		if(new_mesg_flag){
			strncpy(r_str_word, get_serial_mesg(), SERIAL_PARSER_COMMAND_MAX_LENGTH);
			parse_serial_command(&serial_command_list, r_str_word);
			new_mesg_flag = 0;
		}
	gyro_poll_fun();
	}

	return 0;
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
