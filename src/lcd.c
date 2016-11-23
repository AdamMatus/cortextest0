#include <stm32f4xx.h>
#include <stdlib.h>
#include "lcd.h"
#include "usart.h"
#include "stm32f429I-disc-board-spi.h"

static inline void lcd_spi_chip_select();
static inline void lcd_spi_chip_deselect();

static inline void lcd_spi_data_select();
static inline void lcd_spi_command_select();

static void lcd_write_data(uint8_t data);
static void lcd_write_reg(uint8_t reg);

extern volatile uint32_t systemTicks;

void lcd_delay(uint32_t del){
	put_log_mesg("start delay");
	uint32_t start_tick = systemTicks;
	while(start_tick+del >  systemTicks);
	put_log_mesg("end of polling delay");
}

void lcd_init(){
	//**CS BIT INIT**
	//clock on
	LCD_CS_GPIO_CLOCK_EN_REG |= LCD_CS_GPIO_CLOCK_EN_MSK;
	//set direction of CS pin as an output
	LCD_CS_GPIO -> MODER |= 0x01 << 2*LCD_CS_PIN_NUM;
	//drive CS_PIN to high
	lcd_spi_chip_deselect();

	//configure SPI to LCD mode
	discf429_spi_lcd_mode_set();

//  /* Configure LCD */
//  lcd_write_reg(0xCA);
//  lcd_write_data(0xC3);
//  lcd_write_data(0x08);
//  lcd_write_data(0x50);
//  lcd_write_reg(LCD_POWERB);
//  lcd_write_data(0x00);
//  lcd_write_data(0xC1);
//  lcd_write_data(0x30);
//  lcd_write_reg(LCD_POWER_SEQ);
//  lcd_write_data(0x64);
//  lcd_write_data(0x03);
//  lcd_write_data(0x12);
//  lcd_write_data(0x81);
//  lcd_write_reg(LCD_DTCA);
//  lcd_write_data(0x85);
//  lcd_write_data(0x00);
//  lcd_write_data(0x78);
//  lcd_write_reg(LCD_POWERA);
//  lcd_write_data(0x39);
//  lcd_write_data(0x2C);
//  lcd_write_data(0x00);
//  lcd_write_data(0x34);
//  lcd_write_data(0x02);
//  lcd_write_reg(LCD_PRC);
//  lcd_write_data(0x20);
//  lcd_write_reg(LCD_DTCB);
//  lcd_write_data(0x00);
//  lcd_write_data(0x00);
//  lcd_write_reg(LCD_FRMCTR1);
//  lcd_write_data(0x00);
//  lcd_write_data(0x1B);
//  lcd_write_reg(LCD_DFC);
//  lcd_write_data(0x0A);
//  lcd_write_data(0xA2);
//  lcd_write_reg(LCD_POWER1);
//  lcd_write_data(0x10);
//  lcd_write_reg(LCD_POWER2);
//  lcd_write_data(0x10);
//  lcd_write_reg(LCD_VCOM1);
//  lcd_write_data(0x45);
//  lcd_write_data(0x15);
//  lcd_write_reg(LCD_VCOM2);
//  lcd_write_data(0x90);
//  lcd_write_reg(LCD_MAC);
//  lcd_write_data(0xC8);
//  lcd_write_reg(LCD_3GAMMA_EN);
//  lcd_write_data(0x00);
//  lcd_write_reg(LCD_RGB_INTERFACE);
//  lcd_write_data(0xC2);
//  lcd_write_reg(LCD_DFC);
//  lcd_write_data(0x0A);
//  lcd_write_data(0xA7);
//  lcd_write_data(0x27);
//  lcd_write_data(0x04);
//  
//  /* Colomn address set */
//  lcd_write_reg(LCD_COLUMN_ADDR);
//  lcd_write_data(0x00);
//  lcd_write_data(0x00);
//  lcd_write_data(0x00);
//  lcd_write_data(0xEF);
//  /* Page address set */
//  lcd_write_reg(LCD_PAGE_ADDR);
//  lcd_write_data(0x00);
//  lcd_write_data(0x00);
//  lcd_write_data(0x01);
//  lcd_write_data(0x3F);
//  lcd_write_reg(LCD_INTERFACE);
//  lcd_write_data(0x01);
//  lcd_write_data(0x00);
//  lcd_write_data(0x06);
//  
//  lcd_write_reg(LCD_GRAM);
//  lcd_delay(200);
//  
//  lcd_write_reg(LCD_GAMMA);
//  lcd_write_data(0x01);
//  
//  lcd_write_reg(LCD_PGAMMA);
//  lcd_write_data(0x0F);
//  lcd_write_data(0x29);
//  lcd_write_data(0x24);
//  lcd_write_data(0x0C);
//  lcd_write_data(0x0E);
//  lcd_write_data(0x09);
//  lcd_write_data(0x4E);
//  lcd_write_data(0x78);
//  lcd_write_data(0x3C);
//  lcd_write_data(0x09);
//  lcd_write_data(0x13);
//  lcd_write_data(0x05);
//  lcd_write_data(0x17);
//  lcd_write_data(0x11);
//  lcd_write_data(0x00);
//  lcd_write_reg(LCD_NGAMMA);
//  lcd_write_data(0x00);
//  lcd_write_data(0x16);
//  lcd_write_data(0x1B);
//  lcd_write_data(0x04);
//  lcd_write_data(0x11);
//  lcd_write_data(0x07);
//  lcd_write_data(0x31);
//  lcd_write_data(0x33);
//  lcd_write_data(0x42);
//  lcd_write_data(0x05);
//  lcd_write_data(0x0C);
//  lcd_write_data(0x0A);
//  lcd_write_data(0x28);
//  lcd_write_data(0x2F);
//  lcd_write_data(0x0F);
  
////  lcd_write_reg(LCD_SLEEP_OUT);
//  lcd_delay(200);
//  lcd_write_reg(LCD_DISPLAY_ON);
//  /* GRAM start writing */
//  lcd_write_reg(LCD_GRAM);

	//for(uint32_t i=1024; i>0; i--)
	//	lcd_write_data(0x0f);

	discf429_spi_gyro_mode_set();
}

static void lcd_spi_write(uint8_t data){
	lcd_spi_chip_select();
	DISCF429_SPI->CR1 |= SPI_CR1_BIDIOE + SPI_CR1_SPE;

	while(!(DISCF429_SPI -> SR & SPI_SR_TXE));
	DISCF429_SPI -> DR = data;
	while(!(DISCF429_SPI -> SR & SPI_SR_BSY));

	DISCF429_SPI->CR1 &= ~(SPI_CR1_SPE);
	lcd_spi_chip_deselect();
}

inline static void lcd_write_data(uint8_t data){
	lcd_spi_data_select();
	lcd_spi_write(data);
} 

inline static void lcd_write_reg(uint8_t reg){
	lcd_spi_command_select();
	lcd_spi_write(reg);
}

static void lcd_spi_read(uint8_t reg, uint8_t* data, size_t data_size){
	lcd_write_reg(reg);
	
//	lcd_spi_data_select();
//	lcd_spi_chip_select();
//
//	DISCF429_SPI->CR1 &= ~(SPI_CR1_BIDIOE);
//	DISCF429_SPI->CR1 |= SPI_CR1_SPE;
//
//	
//	while(data_size--){
//		while(!(DISCF429_SPI->SR & SPI_SR_RXNE));	
//		*data = DISCF429_SPI->DR;
//		data++;
//	}
//
//	DISCF429_SPI->CR1 &= ~(SPI_CR1_SPE);
//	lcd_spi_chip_deselect();
}

static inline void lcd_spi_data_select(){
	LCD_DC_GPIO -> BSRR |= LCD_DC_BSRR_HIGH_MSK;	
}

static void inline lcd_spi_command_select(){
	LCD_DC_GPIO -> BSRR |= LCD_DC_BSRR_LOW_MSK;
}

static inline void lcd_spi_chip_select(){
	LCD_CS_GPIO -> BSRR |= LCD_CS_BSRR_LOW_MSK;
}

static inline void lcd_spi_chip_deselect(){
	LCD_CS_GPIO -> BSRR |= LCD_CS_BSRR_HIGH_MSK;
}

/*******serial handlers**********/

static const size_t  MAX_NUM_OF_PARAMS = 8;
void serial_command_lcd_handler(char* msg, int index){

	//**
	//TODO
	discf429_spi_lcd_mode_set();
	//**
	
	msg=msg+index;

	uint8_t	reg_address	= (uint8_t) strtol(msg, &msg, 16);
	if(reg_address == 0){
		put_log_mesg("Wrong address format");
		discf429_spi_gyro_mode_set();
		return;
	}

	uint8_t num_of_params = (uint8_t) strtol(msg, &msg, 16);
	if(num_of_params == 0){
		put_log_mesg("Wrong num of params psie");
		discf429_spi_gyro_mode_set();
		return;
	}
	
	uint8_t data_arr[MAX_NUM_OF_PARAMS];
	uint8_t* param_p = data_arr;
	lcd_spi_read(reg_address, data_arr, num_of_params);
	
	while(num_of_params--){
		char print_mesg[17+23] = "Reg value is: 0x";//17 necessery to write 16-bit int
		itoa(*param_p, print_mesg+strlen(print_mesg),16);
		param_p++;
		put_log_mesg(print_mesg);
	}

	//**
	//LCD-SPI mode off
	discf429_spi_gyro_mode_set();
	//**

	return;
}
