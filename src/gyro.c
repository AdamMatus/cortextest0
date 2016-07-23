#include "gyro.h"

volatile static uint16_t received_word;
volatile static uint16_t gyro_rx_flag;

inline void enable_spi_transmission();
inline void disable_spi_transmission();

void config_gyro(){

	//**SPI INIT**
	//SPI clock ON
	GYRO_SPI_CLOCK_EN_REG |= GYRO_SPI_CLOCK_EN_MASK;
	//SPI CR1 conf -  8-bit frame & PCLK/16  
	GYRO_SPI -> CR1 &= ~( SPI_CR1_SSM + SPI_CR1_LSBFIRST); 
	GYRO_SPI -> CR1 |= SPI_CR1_DFF + SPI_CR1_BR_0 + SPI_CR1_BR_1 + SPI_CR1_CPOL + SPI_CR1_CPHA;
	//SPI CR2 conf - RX interrupt enable & SS output enable xD
	GYRO_SPI -> CR2 &= ~(0x0000);
	GYRO_SPI -> CR2 |= SPI_CR2_SSOE + SPI_CR2_RXNEIE;
	//SPI interrupt enabling
	NVIC->ISER[GYRO_SPI_IRQn/32] |= 0x01 << (GYRO_SPI_IRQn%32);
	//SPI PIN CONFIG
	GYRO_SPI_SCK_GPIO_CLOCK_EN_REG |= GYRO_SPI_SCK_GPIO_CLOCK_EN_MASK;
	GYRO_SPI_MISO_GPIO_CLOCK_EN_REG |= GYRO_SPI_MISO_GPIO_CLOCK_EN_MASK;
	GYRO_SPI_MOSI_GPIO_CLOCK_EN_REG |= GYRO_SPI_MOSI_GPIO_CLOCK_EN_MASK;
		//on MODER reg 2 bits for each pin
	GYRO_SPI_SCK_GPIO -> MODER |= 0x02 << (GYRO_SPI_SCK_PIN*2); 
	GYRO_SPI_SCK_GPIO -> AFR[(GYRO_SPI_SCK_PIN*4)/32] |= GYRO_SPI_SCK_AF << ((GYRO_SPI_SCK_PIN*4)%32);
	GYRO_SPI_MISO_GPIO -> MODER |= 0x02 << (GYRO_SPI_MISO_PIN*2);
	GYRO_SPI_MISO_GPIO -> AFR[(GYRO_SPI_MISO_PIN*4)/32] |= GYRO_SPI_MISO_AF << ((GYRO_SPI_MISO_PIN*4)%32);
	GYRO_SPI_MOSI_GPIO -> MODER |= 0x02 << (GYRO_SPI_MOSI_PIN*2); 
	GYRO_SPI_MOSI_GPIO -> AFR[(GYRO_SPI_MOSI_PIN*4)/32] |= GYRO_SPI_MOSI_AF << ((GYRO_SPI_MOSI_PIN*4)%32);

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

uint16_t gyro_read_id(){
	enable_spi_transmission();
	GYRO_SPI -> DR = 0x8F00;
	while(!gyro_rx_flag);
	disable_spi_transmission();
	gyro_rx_flag = 0;
	return received_word;
}

void SPI5_IRQHandler(){
	uint16_t stat_reg = GYRO_SPI->SR;

	if(stat_reg & SPI_SR_RXNE){
		received_word = GYRO_SPI->DR;
		gyro_rx_flag = 1;
	}
		
}


inline void enable_spi_transmission(){
	GYRO_CS_GPIO -> BSRR |= GYRO_CS_LOW;	
}
inline void disable_spi_transmission(){
	GYRO_CS_GPIO -> BSRR |= GYRO_CS_HIGH;
}

//DO IT
void serial_command_gyro_read_handler(char* r_str,int index){
	int original_index = index;

	put_log_mesg(r_str+original_index);
	return;
}
