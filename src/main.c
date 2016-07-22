#include <stm32f4xx.h>
#include <stdio.h>
#include <core_cm4.h>

#include "usart.h"
#include "led.h"
#include "gyro.h"

void TIM1_UP_TIM10_IRQHandler();
void put_log_mesg(char* mesg);
volatile uint32_t systemTicks;

unsigned int onoffleds;
volatile uint32_t LOG_LED_FLAG;

int main()
{
	//SysClock - HSI 16 MHz
	//SysTickConfig
	SysTick->LOAD = 16000000/(8*1000)-1;	
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk + SysTick_CTRL_ENABLE_Msk;

	//AHB = APB1 = APB2
	//GPIOG clock enabling, USART2 clock enabling
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	//
	//TIM10 clock enabling
	RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;

	config_leds();

	// set USART2 TX/RX as pins' function
	GPIOD -> MODER |= GPIO_MODER_MODER5_1 + GPIO_MODER_MODER6_1; 
	GPIOD -> PUPDR |= GPIO_PUPDR_PUPDR5_0;
	GPIOD -> OTYPER |= GPIO_OTYPER_OT_6; 
	GPIOD -> AFR[0] |= 0x7 << 4*5 | 0x7 << 4*6;
	// enable TIM1_UP_TIM10_IRQ
	NVIC -> ISER[0] |= 1 << (TIM1_UP_TIM10_IRQn);
	NVIC -> ISER[USART2_IRQn/32] |= 1 << (USART2_IRQn%32);
	//
	//stop timers while in breakpoint in debug
  DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM10_STOP;
	// TIM10 config
	TIM10 -> ARR = 5000; // 1000  milliseconds
	TIM10 -> PSC = 16000 - 1; // PCLK:16 MHz / 16 000 = 1 kHz
	TIM10 -> DIER |= TIM_DIER_UIE; //
	TIM10 -> CR1 |= TIM_CR1_ARPE	+ TIM_CR1_CEN; // control register
	TIM10 -> EGR = 1; // generate update event
	//
	config_usart();

	config_gyro();

	//SPI GYRO connection test
	if(gyro_read_id() == 0xD4)
		put_log_mesg("SPI GYRO connectivity test succeed!\n");
	else
		put_log_mesg("SPI GYRO connectivity test failed! \n");

	put_log_mesg("main loop starting in here\n\r"); 

	struct command_list_node serial_cln_echo;
	serial_cln_echo.chfp = serial_command_echo_handler;
	serial_cln_echo.command_pattern = "ECHO";
	serial_cln_echo.next = NULL;
	add_command_node(&serial_cln_echo);

	struct command_list_node serial_cln_led; 
	serial_cln_led.chfp = serial_command_led_handler;
	serial_cln_led.command_pattern = "LED";
	serial_cln_led.next = NULL;
	add_command_node(&serial_cln_led);

	while(1){
		if(LOG_LED_FLAG & 0x01){
			put_log_mesg("LEDS just TOOGLED! xD\n");
			LOG_LED_FLAG &= ~0x01;
		}

		if(new_mesg_flag){
			parse_serial_command();
			new_mesg_flag = 0;
		}
	}

	return 0;
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
	

void SysTick_Handler()
{
	systemTicks++;
}
