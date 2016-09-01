#ifndef AUX_TIM_H_
#define AUX_TIM_H_

void tim10_config();
void update_aux_tim_period(uint32_t period);
void serial_command_aux_tim_handler(char*,int);

#endif
