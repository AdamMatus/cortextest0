COMPILER=arm-none-eabi-gcc
OBJS=main.o startup_stm32f429xx.o system_stm32f4xx.o usart.o led.o gyro.o aux_tim.o stm32f429I-disc-board-spi.o lcd.o

DEVICE=STM32F429xx

LINKERSCRIPT=../STM32Cube_FW_F4_V1.11.0/Projects/STM32F429I-Discovery/Templates/TrueSTUDIO/STM32F429I_DISCO/STM32F429ZI_FLASH.ld
USER_INCLUDE=./inc
CMSIS_DEVICE_INCLUDE=../STM32Cube_FW_F4_V1.11.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CMSIS_CORE_INCLUDE=../STM32Cube_FW_F4_V1.11.0/Drivers/CMSIS/Include
LANG_VER=-std=gnu99

default: test0 

test0: main.hex

main.hex : main.elf
	arm-none-eabi-objcopy -Oihex main.elf main.hex

main.elf : $(OBJS) 
		$(COMPILER) -mcpu=cortex-m4 -mlittle-endian -mthumb -D $(DEVICE)  -T $(LINKERSCRIPT)  -Wl,--gc-sections $(OBJS) -g3 -o main.elf

main.o : src/main.c
	$(COMPILER) $(LANG_VER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -I $(USER_INCLUDE) -D $(DEVICE) -Os -g3 -c src/main.c -o main.o 

startup_stm32f429xx.o : src/startup_stm32f429xx.s
	$(COMPILER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -I $(USER_INCLUDE) -D $(DEVICE) -Os -g3 -c src/startup_stm32f429xx.s -o startup_stm32f429xx.o

system_stm32f4xx.o : src/system_stm32f4xx.c
	$(COMPILER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -I $(USER_INCLUDE) -D $(DEVICE) -Os -g3 -c src/system_stm32f4xx.c -o system_stm32f4xx.o 

usart.o : src/usart.c inc/usart.h
	$(COMPILER) $(LANG_VER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -I $(USER_INCLUDE) -D $(DEVICE) -Os -g3 -c src/usart.c -o usart.o 

led.o : src/led.c inc/led.h
	$(COMPILER) $(LANG_VER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -I $(USER_INCLUDE) -D $(DEVICE) -Os -g3 -c src/led.c -o led.o 

gyro.o : src/gyro.c inc/gyro.h inc/L3GD20.h
	$(COMPILER) $(LANG_VER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -I $(USER_INCLUDE) -D $(DEVICE) -Os -g3 -c src/gyro.c -o gyro.o 

aux_tim.o : src/aux_tim.c inc/aux_tim.h 
	$(COMPILER) $(LANG_VER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -I $(USER_INCLUDE) -D $(DEVICE) -Os -g3 -c src/aux_tim.c -o aux_tim.o 

stm32f429I-disc-board-spi.o : src/stm32f429I-disc-board-spi.c inc/stm32f429I-disc-board-spi.h 
	$(COMPILER) $(LANG_VER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -I $(USER_INCLUDE) -D $(DEVICE) -Os -g3 -c src/stm32f429I-disc-board-spi.c -o stm32f429I-disc-board-spi.o


lcd.o : src/lcd.c inc/lcd.h
	$(COMPILER) $(LANG_VER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -I $(USER_INCLUDE) -D $(DEVICE) -Os -g3 -c src/lcd.c -o lcd.o

clean :
	rm $(OBJS)




