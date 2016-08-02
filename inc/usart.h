#ifndef _DEFINE_USART_H_
#define _DEFINE_USART_H_

#include <string.h>
#include <stm32f4xx.h>

const uint16_t LOG_TIME_SIZE_FORMAT; 
#define SERIAL_PARSER_COMMAND_MAX_LENGTH 32

uint16_t new_mesg_flag;

extern  volatile uint32_t systemTicks;

typedef void (*command_handler_fp)(char*, int);

struct command_list_node;

struct command_list_node{
	struct command_list_node* next;
	char* command_pattern;
	command_handler_fp chfp;
};

struct command_list{
	struct command_list_node* last_cln;
};

void USART2_IRQHandler();
void config_usart();
void put_log_mesg(char* mesg);

void serial_command_init();
void add_command_node(struct command_list*, struct command_list_node* cln);
void parse_serial_command(struct command_list*, char* str);

void serial_command_echo_handler(char*, int);

char* get_serial_mesg();

#endif
