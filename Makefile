COMPILER=arm-none-eabi-gcc
OBJS=main.o startup_stm32f429xx.o system_stm32f4xx.o

DEVICE=STM32F429xx

LINKERSCRIPT=../STM32Cube_FW_F4_V1.11.0/Projects/STM32F429I-Discovery/Templates/TrueSTUDIO/STM32F429I_DISCO/STM32F429ZI_FLASH.ld
CMSIS_DEVICE_INCLUDE=../STM32Cube_FW_F4_V1.11.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include
CMSIS_CORE_INCLUDE=../STM32Cube_FW_F4_V1.11.0/Drivers/CMSIS/Include
LANG_VER=-std=gnu99

deafult: test0 

test0: main.hex

main.hex : main.elf
	arm-none-eabi-objcopy -Oihex main.elf main.hex

main.elf : $(OBJS) 
		$(COMPILER) -mcpu=cortex-m4 -mlittle-endian -mthumb -D $(DEVICE)  -T $(LINKERSCRIPT)  -Wl,--gc-sections $(OBJS) -g3 -o main.elf

main.o : main.c
	$(COMPILER) $(LANG_VER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -D $(DEVICE) -Os -g3 -c main.c -o main.o 

startup_stm32f429xx.o : startup_stm32f429xx.s
	$(COMPILER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -D $(DEVICE) -Os -g3 -c startup_stm32f429xx.s -o startup_stm32f429xx.o

system_stm32f4xx.o : system_stm32f4xx.c
	$(COMPILER) -Wall -mcpu=cortex-m4 -mlittle-endian -mthumb -I $(CMSIS_CORE_INCLUDE) -I $(CMSIS_DEVICE_INCLUDE) -D $(DEVICE) -Os -g3 -c system_stm32f4xx.c -o system_stm32f4xx.o 

clean :
	rm $(OBJS)




