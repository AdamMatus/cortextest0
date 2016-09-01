#ifndef GYRO_H_
#define GYRO_H_

#include <stddef.h>
#include <stm32f4xx.h>
#include <usart.h>

#include <L3GD20.h>

#define GYRO_SPI SPI5 
#define GYRO_SPI_CLOCK_EN_REG RCC->APB2ENR
#define GYRO_SPI_CLOCK_EN_MASK RCC_APB2ENR_SPI5EN
#define GYRO_SPI_IRQn SPI5_IRQn
#define GYRO_SPI_SCK_PIN (uint32_t) 7
#define GYRO_SPI_MISO_PIN (uint32_t) 8
#define GYRO_SPI_MOSI_PIN (uint32_t) 9
#define GYRO_SPI_SCK_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define GYRO_SPI_MISO_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define GYRO_SPI_MOSI_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define GYRO_SPI_SCK_GPIO_CLOCK_EN_MASK RCC_AHB1ENR_GPIOFEN
#define GYRO_SPI_MISO_GPIO_CLOCK_EN_MASK RCC_AHB1ENR_GPIOFEN
#define GYRO_SPI_MOSI_GPIO_CLOCK_EN_MASK RCC_AHB1ENR_GPIOFEN
#define GYRO_SPI_SCK_GPIO GPIOF
#define GYRO_SPI_MISO_GPIO GPIOF
#define GYRO_SPI_MOSI_GPIO GPIOF
#define GYRO_SPI_SCK_AF (uint32_t) 5
#define GYRO_SPI_MISO_AF (uint32_t) 5
#define GYRO_SPI_MOSI_AF  (uint32_t) 5

#define GYRO_CS_GPIO GPIOC
#define GYRO_CS_PIN (uint32_t) 1
#define GYRO_CS_MODER_MASK GPIO_MODER_MODER1_0
#define GYRO_CS_GPIO_CLOCK_EN_REG  RCC->AHB1ENR
#define GYRO_CS_RCC_EN_MASK RCC_AHB1ENR_GPIOCEN
#define GYRO_CS_HIGH GPIO_BSRR_BS_1
#define GYRO_CS_LOW GPIO_BSRR_BR_1

#define GYRO_INT1_GPIO GPIOA
#define GYRO_INT1_PIN (uint32_t) 1
#define GYRO_INT1_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define GYRO_INT1_RCC_EN_MASK RCC_AHB1ENR_GPIOAEN
#define GYRO_INT1_SYSCFG_EXTICR_MSK SYSCFG_EXTICR1_EXTI1_PA
#define GYRO_INT1_EXTI_NUM 1
#define GYRO_INT1_IRQn EXTI1_IRQn

#define GYRO_INT2_GPIO GPIOA
#define GYRO_INT2_PIN (uint32_t) 2
#define GYRO_INT2_GPIO_CLOCK_EN_REG RCC->AHB1ENR
#define GYRO_INT2_RCC_EN_MASK RCC_AHB1ENR_GPIOAEN
#define GYRO_INT2_SYSCFG_EXTICR_MSK SYSCFG_EXTICR1_EXTI2_PA
#define GYRO_INT2_EXTI_NUM 2 
#define GYRO_INT2_IRQn EXTI2_IRQn

#define GYRO_C_BUFF_SIZE ( (size_t) 32 )
struct gyro_c_buff {
	uint16_t buff[GYRO_C_BUFF_SIZE];
	uint16_t *read_p;
	uint16_t *write_p;
};
void init_c_buff(struct gyro_c_buff* buffp);
void write_c_buff(struct gyro_c_buff* buffp, uint16_t data);
uint16_t read_c_buff(struct gyro_c_buff* bufp);

void gyro_bypass_mode_init();
void gyro_init();
uint16_t gyro_single_read(uint8_t address);
void gyro_multiple_read(uint8_t, uint16_t*, size_t);
void gyro_print_xyz();
void gyro_single_write(uint8_t address, uint8_t data);
void spi5_irqhandler();
void serial_command_gyro_handler(char* r_str, int index);
void serial_command_gyro_read_handler(char* r_str,int index);
void serial_command_gyro_write_handler(char* r_str,int index);

void gyro_poll_fun();
#endif
