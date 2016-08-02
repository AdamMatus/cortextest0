#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gyro.h"
#include "L3GD20.h"

volatile static uint16_t received_word;
//volatile static uint16_t gyro_rx_flag;
//
static struct command_list gyro_serial_list;
static struct command_list_node serial_cln_gyro_read;
static struct command_list_node serial_cln_gyro_write;

inline void enable_spi_transmission();
inline void disable_spi_transmission();

void gyro_init(){
	gyro_spi_config();
	gyro_serial_command_init();
	gyro_bypass_mode_init();
}
void gyro_spi_config(){

	//**SPI INIT**
	//SPI clock ON
	GYRO_SPI_CLOCK_EN_REG |= GYRO_SPI_CLOCK_EN_MASK;
	//SPI CR1 conf -  8-bit frame & PCLK/16  
	GYRO_SPI -> CR1 &= ~( SPI_CR1_SSM + SPI_CR1_LSBFIRST); 
	GYRO_SPI -> CR1 |= SPI_CR1_DFF + SPI_CR1_BR_0 + SPI_CR1_BR_1 + SPI_CR1_CPOL + SPI_CR1_CPHA;
	//SPI CR2 conf - RX interrupt enable & SS output enable xD
	GYRO_SPI -> CR2 &= ~(0x0000);
	GYRO_SPI -> CR2 |= SPI_CR2_SSOE; // + SPI_CR2_RXNEIE;
	//SPI interrupt enabling
	//NVIC->ISER[GYRO_SPI_IRQn/32] |= 0x01 << (GYRO_SPI_IRQn%32);
	//SPI PIN CONFIG
	GYRO_SPI_SCK_GPIO_CLOCK_EN_REG |= GYRO_SPI_SCK_GPIO_CLOCK_EN_MASK;
	GYRO_SPI_MISO_GPIO_CLOCK_EN_REG |= GYRO_SPI_MISO_GPIO_CLOCK_EN_MASK;
	GYRO_SPI_MOSI_GPIO_CLOCK_EN_REG |= GYRO_SPI_MOSI_GPIO_CLOCK_EN_MASK;
		//on MODER reg 2 bits for each pin
	GYRO_SPI_SCK_GPIO -> MODER |= 0x02 << (GYRO_SPI_SCK_PIN*2); 
	GYRO_SPI_SCK_GPIO -> AFR[(GYRO_SPI_SCK_PIN*4)/32] |= GYRO_SPI_SCK_AF << ((GYRO_SPI_SCK_PIN*4)%32);

	GYRO_SPI_MISO_GPIO -> MODER |= 0x02 << (GYRO_SPI_MISO_PIN*2);
	GYRO_SPI_MISO_GPIO -> AFR[(GYRO_SPI_MISO_PIN*4)/32] |= GYRO_SPI_MISO_AF << ((GYRO_SPI_MISO_PIN*4)%32);
	GYRO_SPI_MISO_GPIO -> PUPDR |= 0x02 << (GYRO_SPI_MISO_PIN*2);

	GYRO_SPI_MOSI_GPIO -> MODER |= 0x02 << (GYRO_SPI_MOSI_PIN*2); 
	GYRO_SPI_MOSI_GPIO -> AFR[(GYRO_SPI_MOSI_PIN*4)/32] |= GYRO_SPI_MOSI_AF << ((GYRO_SPI_MOSI_PIN*4)%32);
	GYRO_SPI_MOSI_GPIO -> PUPDR |= 0x02 << (GYRO_SPI_MOSI_PIN*2);

	//**CS BIT INIT**
	//clock on
	GYRO_CS_GPIO_CLOCK_EN_REG |= GYRO_CS_RCC_EN_MASK;
	//set direction of CS pin as an output
	GYRO_CS_GPIO -> MODER |= GYRO_CS_MODER_MASK;
	//drive CS_PIN to high
	GYRO_CS_GPIO -> BSRR |= GYRO_CS_HIGH;

	//turning on SPI
	GYRO_SPI -> CR1 |= SPI_CR1_MSTR + SPI_CR1_SPE;
}

void gyro_serial_command_init(){
	serial_cln_gyro_read.chfp = serial_command_gyro_read_handler;
	serial_cln_gyro_read.command_pattern = "READ";
	serial_cln_gyro_read.next = NULL;
	add_command_node(&gyro_serial_list, &serial_cln_gyro_read);

	serial_cln_gyro_write.chfp = serial_command_gyro_write_handler;
	serial_cln_gyro_write.command_pattern = "WRITE";
	serial_cln_gyro_write.next = NULL;
	add_command_node(&gyro_serial_list, &serial_cln_gyro_write);
}

void gyro_bypass_mode_init(){
	
	gyro_single_write(GYRO_CTRL_REG1,0x0F);
}

// strlen of (GYRO X:) = 7, magic number
void gyro_read_xyz(){
	uint16_t data[4];
	char str[7+8*2+1+1] = "GYRO X:"; // 8*2 - max size of dec num, 1 for \n 1 for \0

//	gyro_multiple_read(GYRO_STATUS_REG,data, 4);

	data[0] = gyro_single_read(GYRO_STATUS_REG);
	data[1] = 0x0F & gyro_single_read(GYRO_OUT_X_L);
	data[1] |= gyro_single_read(GYRO_OUT_X_H) << 8;
	data[2] = 0x0F & gyro_single_read(GYRO_OUT_Y_L);
	data[2] |= gyro_single_read(GYRO_OUT_Y_H) << 8;
	data[3] = 0x0F & gyro_single_read(GYRO_OUT_Z_L);
	data[3] |= gyro_single_read(GYRO_OUT_Z_H) << 8;

	for(int i=1; i <= 3; i++){
		itoa( (int16_t)data[i], str+7, 10);	//on str+len is '\0', 7 is magic
		size_t len = strlen(str); 
		str[len] = 0;
		put_log_mesg(str);
		str[5]++; // X is on 5th position in "GYRO X:"
	}
}

uint16_t gyro_single_read(uint8_t address){ //polling function
	enable_spi_transmission();
	GYRO_SPI -> DR = 0x80FF + (address << 8);
	while(!(GYRO_SPI->SR & SPI_SR_RXNE));
	received_word = GYRO_SPI -> DR;
	disable_spi_transmission();
	return received_word;
}

void gyro_multiple_read(uint8_t address,uint16_t* data,size_t n){ //polling fuction
	enable_spi_transmission();
	GYRO_SPI -> DR = 0xC0FF + (address << 8);
	for(int i=0; i < n; i++){
		while(!(GYRO_SPI->SR & SPI_SR_TXE));
		GYRO_SPI -> DR = 0xFFFF;
		while(!(GYRO_SPI->SR & SPI_SR_RXNE));
		data[i] = GYRO_SPI->DR;
	}
	disable_spi_transmission();
	return;
}

void gyro_single_write(uint8_t address,uint8_t data){
	enable_spi_transmission();
	GYRO_SPI -> DR = 0x0000 + (address << 8) + data;
	while(!(GYRO_SPI -> SR & SPI_SR_RXNE));
	GYRO_SPI -> DR;
	disable_spi_transmission();
	return;
}

/*void SPI5_IRQHandler(){
	uint16_t stat_reg = GYRO_SPI->SR;

	if(stat_reg & SPI_SR_RXNE){
		received_word = GYRO_SPI->DR;
		gyro_rx_flag = 1;
	}
		
}
*/

inline void enable_spi_transmission(){
	GYRO_CS_GPIO -> BSRR |= GYRO_CS_LOW;	
}
inline void disable_spi_transmission(){
	GYRO_CS_GPIO -> BSRR |= GYRO_CS_HIGH;
}

void serial_command_gyro_handler(char* r_str,int index){
	parse_serial_command(&gyro_serial_list, r_str+index);
	return;
}

void serial_command_gyro_read_handler(char* r_str, int index){
	char* str_p = &r_str[index];
	uint8_t reg_address = (uint8_t)strtol(str_p, NULL, 16);
	if(reg_address == 0)
		put_log_mesg("Wrong gyro reg address.");
	else{
		uint16_t read_data = gyro_single_read(reg_address);
		char mesg[17+23] = "Reg value is: 0x";//17 necessery to write 16-bit int
		itoa(read_data,mesg+strlen(mesg),16);
		put_log_mesg(mesg);
	}
	return;
}

void serial_command_gyro_write_handler(char* r_str, int index){
	char* str_p = &r_str[index];
	uint8_t reg_address = (uint8_t)strtol(str_p, &str_p, 16);
	if(reg_address == 0){
		put_log_mesg("Wrong gyro reg address.");
		return;
	}

	uint8_t reg_data = (uint8_t)strtol(str_p, NULL, 16);
	if(reg_data == 0){
		put_log_mesg("Wrong gyro reg data. Or 0x00 value xD.");
		return;
	}

	else{
		gyro_single_write(reg_address, reg_data);
		put_log_mesg("Data has been sent to device.");
	}

	return;	
}
