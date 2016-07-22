#ifndef GYRO_H_
#define GYRO_H_

#include <stm32f4xx.h>

#define GYRO_SPI SPI5 
#define GYRO_SPI_CLOCK_EN_REG RCC->APB2ENR
#define GYRO_SPI_CLOCK_EN_MASK RCC_APB2ENR_SPI5EN
#define GYRO_SPI_IRQn SPI5_IRQn

#define GYRO_CS_GPIO GPIOC
#define GYRO_CS_PIN (uint32_t) 1
#define GYRO_CS_MODER_MASK GPIO_MODER_MODER1_0
#define GYRO_CS_GPIO_CLOCK_EN_REG  RCC->AHB1ENR
#define GYRO_CS_RCC_EN_MASK RCC_AHB1ENR_GPIOCEN
#define GYRO_CS_HIGH GPIO_BSRR_BS_1
#define GYRO_CS_LOW GPIO_BSRR_BR_1

void config_gyro();
uint16_t gyro_read_id();
void SPI5_IRQHandler();

#endif
