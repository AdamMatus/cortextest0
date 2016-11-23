//SPI5 is used on board to drive LCD controller chip, GYRO L3G20 motion sensor
//and touch sensor integrated with LCD TFT
//
//L3G20 gyro sensor is connected with 4-wire interface (SDI/SDO lines)
//and lcd interface is bidiractional 3-wire so SPI5 is used only to init config
//of LCD and rest of work is done by RGB controller in STM32F4ZI chip

#ifndef STM32F4_DISCO_BOARD_SPI
#define STM32F4_DISCO_BOARD_SPI

#include <stm32f4xx.h>

#define DISCF429_SPI SPI5 
#define DISCF429_SPI_CLOCK_EN_REG RCC->APB2ENR
#define DISCF429_SPI_CLOCK_EN_MASK RCC_APB2ENR_SPI5EN
#define DISCF429_SPI_IRQn SPI5_IRQn
#define DISCF429_SPI_SCK_PIN (uint32_t) 7
#define DISCF429_SPI_MISO_PIN (uint32_t) 8
#define DISCF429_SPI_MOSI_PIN (uint32_t) 9
#define DISCF429_SPI_SCK_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define DISCF429_SPI_MISO_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define DISCF429_SPI_MOSI_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define DISCF429_SPI_SCK_GPIO_CLOCK_EN_MASK RCC_AHB1ENR_GPIOFEN
#define DISCF429_SPI_MISO_GPIO_CLOCK_EN_MASK RCC_AHB1ENR_GPIOFEN
#define DISCF429_SPI_MOSI_GPIO_CLOCK_EN_MASK RCC_AHB1ENR_GPIOFEN
#define DISCF429_SPI_SCK_GPIO GPIOF
#define DISCF429_SPI_MISO_GPIO GPIOF
#define DISCF429_SPI_MOSI_GPIO GPIOF
#define DISCF429_SPI_SCK_AF (uint32_t) 5
#define DISCF429_SPI_MISO_AF (uint32_t) 5
#define DISCF429_SPI_MOSI_AF  (uint32_t) 5 

extern uint16_t spi_lcd_mode_on;

void discf429_spi_config();
void spi5_irqhandler();
void discf429_spi_gyro_mode_set();
void discf429_spi_lcd_mode_set();

#endif
