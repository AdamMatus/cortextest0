#include "usart.h"
#include "gyro.h"
#include "led.h"
#include "lcd.h"
#include "aux_tim.h"

const uint16_t LOG_TIME_SIZE_FORMAT = 9; 
unsigned int rs_index;

static char received_string[128];

void config_usart(){
	//GPIOD clock enabling, USART2 clock enabling
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

	// set USART2 TX/RX as pins' function
	GPIOD -> MODER |= GPIO_MODER_MODER5_1 + GPIO_MODER_MODER6_1; 
	GPIOD -> PUPDR |= GPIO_PUPDR_PUPDR5_0;
	GPIOD -> OTYPER |= GPIO_OTYPER_OT_6; 
	GPIOD -> AFR[0] |= 0x7 << 4*5 | 0x7 << 4*6;

	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	USART2 -> CR1 |= USART_CR1_UE;
	USART2 -> CR2 |= 0x00;
	USART2 -> BRR = (104 << 4) | 3; // prescaler set to divide by 104,1875
	
	NVIC -> ISER[USART2_IRQn/32] |= 1 << (USART2_IRQn%32);

	USART2 ->CR1 |=   USART_CR1_RXNEIE +  USART_CR1_RE; //USART_CR1_TCIE 
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
	
	for(uint32_t i = LOG_TIME_SIZE_FORMAT - index; i > 0; --i){
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
	
	char* str_end = "\r\n"; // \r\n\0
	do{
		while( !(USART2->SR & USART_SR_TXE) );
		USART2 -> DR = *str_end;
	} while(*str_end++);

	while( !(USART2->SR & USART_SR_TC));
	USART2 -> CR1 &= ~USART_CR1_TE;

	return;
}

void USART2_IRQHandler()
{

	uint32_t stat_reg = USART2->SR ; 
	// tx handling
	/*if(stat_reg &  USART_SR_TC){
		USART2 -> DR = '\r';
		USART2 -> CR1 &= ~USART_CR1_TE;
	}*/


	// rx handling
	if(stat_reg &  USART_SR_RXNE){
		received_string[rs_index] = USART2->DR;
		if(received_string[rs_index++] == 0 ){
			//received_string[rs_index] = 0;
			rs_index = 0;
			new_mesg_flag = 1;
		}
	}
}


char* get_serial_mesg(){
 return received_string;	
}

// Parsing commands from serial inputs
// every commmand_node in link-list
// represent search node with string pattern
//

struct command_list serial_command_list;
static struct command_list_node serial_cln_echo;
static struct command_list_node serial_cln_led; 
static struct command_list_node serial_cln_gyro;
static struct command_list_node serial_cln_aux_tim;
static struct command_list_node serial_cln_lcd;

void serial_command_init(){
	serial_cln_echo.chfp = serial_command_echo_handler;
	serial_cln_echo.command_pattern = "ECHO";
	serial_cln_echo.next = NULL;
	add_command_node(&serial_command_list, &serial_cln_echo);

	serial_cln_led.chfp = serial_command_led_handler;
	serial_cln_led.command_pattern = "LED";
	serial_cln_led.next = NULL;
	add_command_node(&serial_command_list, &serial_cln_led);
	
	serial_cln_gyro.chfp = serial_command_gyro_handler;
	serial_cln_gyro.command_pattern = "GYRO";
	serial_cln_gyro.next = NULL;
	add_command_node(&serial_command_list, &serial_cln_gyro);

	serial_cln_aux_tim.chfp = serial_command_aux_tim_handler;
	serial_cln_aux_tim.command_pattern = "AUX TIM";
	serial_cln_aux_tim.next = NULL;
	add_command_node(&serial_command_list, &serial_cln_aux_tim);

	serial_cln_lcd.chfp = serial_command_lcd_handler;
	serial_cln_lcd.command_pattern = "LCD";
	serial_cln_lcd.next = NULL;
	add_command_node(&serial_command_list, &serial_cln_lcd);
}

void parse_serial_command(struct command_list* serial_command_list, char* r_str_word){

	struct command_list_node* cln = serial_command_list->last_cln;
 	if(!cln){
		put_log_mesg("parse_serial_command: there is no command_list_node");
		return;
	}

	int i = 0;
	char *c = cln->command_pattern;
	while(c[i] != 0) {
		if(c[i] == r_str_word[i]){
			i++;
			continue;
		}
		else if(cln->next){
			cln = cln->next;
			c = &(cln->command_pattern[i]);
			continue;
		}
		else{
			put_log_mesg("parse_serial_command: no matches for: ");
			put_log_mesg(r_str_word);
			return;
		}
	}

	cln->chfp(r_str_word, ++i);
	return;
}

void add_command_node(struct command_list* serial_command_list, struct command_list_node* cln){
	if(serial_command_list->last_cln == NULL){
		serial_command_list->last_cln = cln;
		cln->next = NULL;
	}
	else{
		cln->next = serial_command_list->last_cln;
		serial_command_list->last_cln = cln;
	}
}

// ECHO handler

void serial_command_echo_handler(char* r_str,int index){

	put_log_mesg(r_str+index);

	return;
}
