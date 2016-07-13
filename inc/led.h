#ifndef LED_H
#define LED_H

#include <stm32f429xx.h>

#define GREEN_LED_ON_MASK GPIO_BSRR_BS_13
#define GREEN_LED_OFF_MASK GPIO_BSRR_BR_13
#define GREEN_LED_TOOGLE_MASK GPIO_ODR_ODR_13

#define RED_LED_ON_MASK GPIO_BSRR_BS_14
#define RED_LED_OFF_MASK GPIO_BSRR_BR_14
#define RED_LED_TOOGLE_MASK GPIO_ODR_ODR_14

enum led_id {GREEN, RED};
enum led_action {OFF, ON, TOOGLE};

void config_leds();
//slow version
void change_led_state(enum led_id ledid, enum led_action ledact);

//fast versions
inline void set_leds(uint32_t mask){
	GPIOG -> BSRR |= mask;
}

inline void reset_leds(uint32_t mask){
	GPIOG -> BSRR |= mask;
}

inline void toogle_leds(uint32_t mask){
	GPIOG -> ODR ^= mask;
}

void serial_command_led_handler(char*, int);

#endif 
