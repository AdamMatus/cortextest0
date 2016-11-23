#include "stm32f429I-disc-board-spi.h"

uint16_t spi_lcd_mode_on = 0;

void discf429_spi_config(){
	//**SPI INIT**
	//SPI clock ON
	DISCF429_SPI_CLOCK_EN_REG |= DISCF429_SPI_CLOCK_EN_MASK;
	//SPI CR1 conf -  8-bit frame & PCLK/16  
	DISCF429_SPI -> CR1 &= ~( SPI_CR1_SSM + SPI_CR1_LSBFIRST); 
	DISCF429_SPI -> CR1 |= SPI_CR1_BR_0 + SPI_CR1_BR_1 + SPI_CR1_CPOL + SPI_CR1_CPHA | SPI_CR1_SSM + SPI_CR1_SSI;
	//SPI CR2 conf - SS output enable xD
	DISCF429_SPI -> CR2 &= ~(0x0000);
	//DISCF429_SPI -> CR2 |= SPI_CR2_SSOE; // + SPI_CR2_RXNEIE;
	//SPI interrupt enabling
	//NVIC->ISER[DISCF429_SPI_IRQn/32] |= 0x01 << (DISCF429_SPI_IRQn%32);
	//SPI PIN CONFIG
	DISCF429_SPI_SCK_GPIO_CLOCK_EN_REG |= DISCF429_SPI_SCK_GPIO_CLOCK_EN_MASK;
	DISCF429_SPI_MISO_GPIO_CLOCK_EN_REG |= DISCF429_SPI_MISO_GPIO_CLOCK_EN_MASK;
	DISCF429_SPI_MOSI_GPIO_CLOCK_EN_REG |= DISCF429_SPI_MOSI_GPIO_CLOCK_EN_MASK;
		//on MODER reg 2 bits for each pin
	DISCF429_SPI_SCK_GPIO -> MODER |= 0x02 << (DISCF429_SPI_SCK_PIN*2); 
	DISCF429_SPI_SCK_GPIO -> AFR[(DISCF429_SPI_SCK_PIN*4)/32] |= DISCF429_SPI_SCK_AF << ((DISCF429_SPI_SCK_PIN*4)%32);

	DISCF429_SPI_MISO_GPIO -> MODER |= 0x02 << (DISCF429_SPI_MISO_PIN*2);
	DISCF429_SPI_MISO_GPIO -> AFR[(DISCF429_SPI_MISO_PIN*4)/32] |= DISCF429_SPI_MISO_AF << ((DISCF429_SPI_MISO_PIN*4)%32);
	DISCF429_SPI_MISO_GPIO -> PUPDR |= 0x02 << (DISCF429_SPI_MISO_PIN*2);

	DISCF429_SPI_MOSI_GPIO -> MODER |= 0x02 << (DISCF429_SPI_MOSI_PIN*2); 
	DISCF429_SPI_MOSI_GPIO -> AFR[(DISCF429_SPI_MOSI_PIN*4)/32] |= DISCF429_SPI_MOSI_AF << ((DISCF429_SPI_MOSI_PIN*4)%32);
	DISCF429_SPI_MOSI_GPIO -> PUPDR |= 0x02 << (DISCF429_SPI_MOSI_PIN*2);

	//turning on SPI
	DISCF429_SPI -> CR1 |= SPI_CR1_MSTR + SPI_CR1_SPE;

	spi_lcd_mode_on = 0;
}

//on the discovery f429i board there is not possible to work simultaneously with
//gyroscope and lcd by spi
void discf429_spi_lcd_mode_set(){
	DISCF429_SPI -> CR1 &= ~(SPI_CR1_SPE);  //disabling SPI clock

	DISCF429_SPI -> CR1 |= SPI_CR1_BIDIMODE + SPI_CR1_BIDIOE; // BI dir mode on and output en

	//taking care of switching SPE bit is provided in read/write funcs of bidi
	//mode fun
	
	spi_lcd_mode_on = 0;
}

void discf429_spi_gyro_mode_set(){
	DISCF429_SPI -> CR1 &= ~(SPI_CR1_SPE); //disabling SPI clock

	DISCF429_SPI -> CR1 &= ~(SPI_CR1_BIDIMODE + SPI_CR1_BIDIOE); //disabling 

	DISCF429_SPI -> CR1 |= SPI_CR1_SPE; //turn on SPI clock
	spi_lcd_mode_on = 1;
}
