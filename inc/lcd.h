#ifndef _DISCF429_LCD_H_
#define _DISCF429_LCD_H_

#include "ili9341.h"

#define LCD_CS_GPIO GPIOC
#define LCD_CS_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define LCD_CS_GPIO_CLOCK_EN_MSK RCC_AHB1ENR_GPIOCEN
#define LCD_CS_PIN_NUM ( (uint32_t) 2 )
#define LCD_CS_BSRR_HIGH_MSK GPIO_BSRR_BS_2
#define LCD_CS_BSRR_LOW_MSK GPIO_BSRR_BR_2

#define LCD_DC_GPIO GPIOD
#define LCD_DC_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define LCD_DC_GPIO_CLOCK_EN_MSK RCC_AHB1ENR_GPIODEN
#define LCD_DC_PIN_NUM ( (uint32_t) 13 )
#define LCD_DC_BSRR_HIGH_MSK GPIO_BSRR_BS_13
#define LCD_DC_BSRR_LOW_MSK GPIO_BSRR_BR_13

void lcd_init();
void serial_command_lcd_handler(char*,int);

#endif

