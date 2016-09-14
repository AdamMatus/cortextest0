#include <stdlib.h>
#include <string.h>
#include "gyro.h"
#include "L3GD20.h"
#include "stm32f429I-disc-board-spi.h"

volatile static uint16_t received_word;
//volatile static uint16_t gyro_rx_flag;
//
static struct command_list gyro_serial_list;
static struct command_list_node serial_cln_gyro_read;
static struct command_list_node serial_cln_gyro_write;

static inline void enable_spi_transmission();
static inline void disable_spi_transmission();

static inline void gyro_serial_command_init();

static struct gyro_c_buff xbuffer, ybuffer, zbuffer;
static struct gyro_c_buff* xyz_p[3] = {&xbuffer, &ybuffer, &zbuffer};

static inline void read_xyz();

void gyro_init(){

	//**CS BIT INIT**
	//clock on
	GYRO_CS_GPIO_CLOCK_EN_REG |= GYRO_CS_RCC_EN_MASK;
	//set direction of CS pin as an output
	GYRO_CS_GPIO -> MODER |= GYRO_CS_MODER_MASK;
	//drive CS_PIN to high
	GYRO_CS_GPIO -> BSRR |= GYRO_CS_HIGH;

	//**GYRO INT1 and INT2 line config**
	//clock on
	GYRO_INT1_GPIO_CLOCK_EN_REG |= GYRO_INT1_RCC_EN_MASK;
	GYRO_INT2_GPIO_CLOCK_EN_REG |= GYRO_INT2_RCC_EN_MASK;
	//set direction of CS pin as intput 
	GYRO_INT1_GPIO -> MODER |= 0x00 << GYRO_INT1_PIN;
	GYRO_INT2_GPIO -> MODER |= 0x00 << GYRO_INT2_PIN;
	//pull down int pins
	GYRO_INT1_GPIO -> PUPDR |= 0x02 << GYRO_INT1_PIN;
	GYRO_INT2_GPIO -> PUPDR |= 0x02 << GYRO_INT2_PIN;	
	//drive CS_PIN to high
	GYRO_CS_GPIO -> BSRR |= GYRO_CS_HIGH;

	//SYSCFG clock on 
	RCC -> APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	//SYSCFG EXTI1 and EXTI2 line enabling as INT1 and INT2
	SYSCFG -> EXTICR[(GYRO_INT1_EXTI_NUM*4)/32] |= GYRO_INT1_SYSCFG_EXTICR_MSK; 
	SYSCFG -> EXTICR[(GYRO_INT2_EXTI_NUM*4)/32] |= GYRO_INT2_SYSCFG_EXTICR_MSK;
	// EXT1 and EXT2 conf
	EXTI -> IMR  |= 1 << GYRO_INT1_EXTI_NUM | 1 << GYRO_INT2_EXTI_NUM;
	EXTI -> RTSR |= 1 << GYRO_INT1_EXTI_NUM | 1 << GYRO_INT2_EXTI_NUM;
	gyro_serial_command_init();

	/*** MODE INIT ***/
	gyro_bypass_mode_init();
	/***/

	for(uint32_t i =0; i < 3; i++){
		init_c_buff(xyz_p[i]);
	}

	//interrupt conf
	NVIC->ISER[GYRO_INT1_IRQn/32] |= 0x01 << (GYRO_INT1_IRQn%32);
	NVIC->ISER[GYRO_INT2_IRQn/32] |= 0x01 << (GYRO_INT2_IRQn%32);

	//init read, necessery to make INT2/DTRDY go low
	read_xyz();
}


static uint32_t gyro_flags = 0;
#define GYRO_FLAGS_OVER_THS 0x02

void EXTI1_IRQHandler(){ //TODO
	uint16_t src_reg = gyro_single_read(GYRO_INT1_SRC); // its too long TODO
	EXTI -> PR &= 1 << GYRO_INT1_EXTI_NUM;
	if(src_reg & 0x2A)
		gyro_flags |= GYRO_FLAGS_OVER_THS;
//	else if(src_reg & 0x15)
//	gyro_flags &= ~(GYRO_FLAGS_OVER_THS);
}

#define GYRO_FLAGS_DTRDY 0x01

void EXTI2_IRQHandler(){
	gyro_flags |= GYRO_FLAGS_DTRDY;
	EXTI -> PR |= (1 << GYRO_INT2_EXTI_NUM);
}

//TODO after approx. 2 minutes there is no interrupt generatet on EXTI2

void gyro_poll_fun(){
	if(gyro_flags & GYRO_FLAGS_DTRDY){

		read_xyz();

		gyro_flags &= ~(GYRO_FLAGS_DTRDY);
	}

	if(gyro_flags & GYRO_FLAGS_OVER_THS){
		put_log_mesg("GYRO: Over threshold.");
		gyro_flags &= ~(GYRO_FLAGS_OVER_THS);
	}
}
static inline void read_xyz(){
	uint16_t gyro_data = 0;
	gyro_data  = 0xFF & gyro_single_read(GYRO_OUT_X_L);
	gyro_data |= gyro_single_read(GYRO_OUT_X_H) << 8;
	write_c_buff(&xbuffer, gyro_data);	

	gyro_data = 0xFF & gyro_single_read(GYRO_OUT_Y_L);
	gyro_data |= gyro_single_read(GYRO_OUT_Y_H) << 8;
	write_c_buff(&ybuffer, gyro_data);	

	gyro_data = 0xFF & gyro_single_read(GYRO_OUT_Z_L);
	gyro_data |= gyro_single_read(GYRO_OUT_Z_H) << 8;
	write_c_buff(&zbuffer, gyro_data);	
}

// strlen of (GYRO X:) = 7, magic number
void gyro_print_xyz(){
	char str[7+8*2+1+1] = "GYRO X:"; // 8*2 - max size of dec num, 1 for \n 1 for \0
	
//	gyro_multiple_read(GYRO_STATUS_REG,test_arr, 4);
	for(uint32_t i=0; i < 3; i++){
		itoa( (int16_t)read_last_c_buff(xyz_p[i]), str+7, 10);	//on str+len is '\0', 7 is magic
		size_t len = strlen(str); 
		str[len] = 0;
		put_log_mesg(str);
		str[5]++; // X is on 5th position in "GYRO X:"
	}
}

static inline void gyro_serial_command_init(){
	serial_cln_gyro_read.chfp = serial_command_gyro_read_handler;
	serial_cln_gyro_read.command_pattern = "READ";
	serial_cln_gyro_read.next = NULL;
	add_command_node(&gyro_serial_list, &serial_cln_gyro_read);

	serial_cln_gyro_write.chfp = serial_command_gyro_write_handler;
	serial_cln_gyro_write.command_pattern = "WRITE";
	serial_cln_gyro_write.next = NULL;
	add_command_node(&gyro_serial_list, &serial_cln_gyro_write);
}

// BYPASS mode + DATARDY INT2 + INT1 Thresshold
void gyro_bypass_mode_init(){
	gyro_single_write(GYRO_CTRL_REG1, 0x07); // power down
	gyro_single_write(GYRO_CTRL_REG2, 0x20); // HP filter 7,2hz in normal mode
	gyro_single_write(GYRO_CTRL_REG5, 0x1F); // enable HP filter

	gyro_single_write(GYRO_INT1_THS_XL, 0xFF);  //threshold
	gyro_single_write(GYRO_INT1_THS_YL, 0xFF);
	gyro_single_write(GYRO_INT1_THS_ZL, 0xFF);

	gyro_single_write(GYRO_INT1_CFG, 0x2A); // OR interupt conf, XYZ higher
	gyro_single_write(GYRO_CTRL_REG3, 0x08); // I1 enable, I2 DATARDY //TODO
	gyro_single_write(GYRO_INT1_DURATION, 0x0F); // int1 after 0xF * 1/ODR

	gyro_single_write(GYRO_CTRL_REG1, 0x0F); // enable all axis in gyroscope, ODR 95Hz
}

//TODO
//FIFO mode with watermark level triggering
void gyro_FIFO_mode_init(){
	gyro_single_write(GYRO_CTRL_REG1, 0x07); //power down
	gyro_single_write(GYRO_CTRL_REG2, 0x20); // HP filter 7,2hz in normal mode
	gyro_single_write(GYRO_CTRL_REG5, 0x5F); // enable HP filter	
	gyro_single_write(GYRO_FIFO_CTRL_REG, 0x3F); //FIFO mode
	gyro_single_write(GYRO_CTRL_REG1, 0x0F); // enable all axis in gyroscope, ODR 95Hz
}


uint16_t gyro_single_read(uint8_t address){ //polling function
	enable_spi_transmission();
	DISCF429_SPI -> DR = 0x80 + address ;
	while(!(DISCF429_SPI->SR & SPI_SR_TXE));
	DISCF429_SPI -> DR = 0xFF;
	while(!(DISCF429_SPI->SR & SPI_SR_RXNE));
	DISCF429_SPI -> DR;
	while(!(DISCF429_SPI->SR & SPI_SR_RXNE));
	received_word = DISCF429_SPI -> DR;
	disable_spi_transmission();
	return received_word;
}

//TODO 
void gyro_multiple_read(uint8_t address,uint16_t* data,size_t n){ 
	enable_spi_transmission();
	DISCF429_SPI -> DR = 0xC0FF + (address << 8);
	for(int i=0; i < n; i++){
		while(!(DISCF429_SPI->SR & SPI_SR_TXE));
		DISCF429_SPI -> DR = 0xFFFF;
		while(!(DISCF429_SPI->SR & SPI_SR_RXNE));
		data[i] = DISCF429_SPI->DR;
	}
	disable_spi_transmission();
	return;
}

void gyro_single_write(uint8_t address,uint8_t data){
	enable_spi_transmission();
	DISCF429_SPI -> DR = 0x00 + address;
	while(!(DISCF429_SPI -> SR & SPI_SR_TXE));
	DISCF429_SPI -> DR = data;
	for(uint32_t i = 2; i !=0; i--){
		while(!(DISCF429_SPI -> SR & SPI_SR_RXNE));
		DISCF429_SPI -> DR;
	}
	disable_spi_transmission();
	return;
}

/*void SPI5_IRQHandler(){
	uint16_t stat_reg = DISCF429_SPI->SR;

	if(stat_reg & SPI_SR_RXNE){
		received_word = DISCF429_SPI->DR;
		gyro_rx_flag = 1;
	}
		
}
*/

inline void enable_spi_transmission(){
	GYRO_CS_GPIO -> BSRR |= GYRO_CS_LOW;	
}
inline void disable_spi_transmission(){
	GYRO_CS_GPIO -> BSRR |= GYRO_CS_HIGH;
}

// CIRCULAR BUFF

void init_c_buff(struct gyro_c_buff* buffp){
	buffp->read_i = 0;	
	buffp->write_i = 0;	
}

void write_c_buff(struct gyro_c_buff* buffp, uint16_t data){
	buffp->buff[buffp->write_i] = data;
	buffp->write_i = (buffp->write_i+1) & (GYRO_C_BUFF_SIZE - 1);
}

uint16_t read_last_c_buff(struct gyro_c_buff* buffp){
	return buffp->buff[(buffp->write_i-1) & (GYRO_C_BUFF_SIZE-1)];
}

uint16_t read_c_buff(struct gyro_c_buff* buffp){

	return 0;
}	

// SERIAL HANDLERS
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
void serial_command_gyro_handler(char* r_str,int index){
	parse_serial_command(&gyro_serial_list, r_str+index);
	return;
}

void serial_command_gyro_read_handler(char* r_str, int index){
	char* str_p = &r_str[index];
	uint8_t reg_address = (uint8_t)strtol(str_p, NULL, 16);
	if(reg_address == 0)
		put_log_mesg("Wrong gyro reg address.");
	else{
		uint16_t read_data = gyro_single_read(reg_address);
		char mesg[17+23] = "Reg value is: 0x";//17 necessery to write 16-bit int
		itoa(read_data,mesg+strlen(mesg),16);
		put_log_mesg(mesg);
	}
	return;
}

void serial_command_gyro_write_handler(char* r_str, int index){
	char* str_p = &r_str[index];
	uint8_t reg_address = (uint8_t)strtol(str_p, &str_p, 16);
	if(reg_address == 0){
		put_log_mesg("Wrong gyro reg address.");
		return;
	}

	uint8_t reg_data = (uint8_t)strtol(str_p, NULL, 16);
	if(reg_data == 0){
		put_log_mesg("Wrong gyro reg data. Or 0x00 value xD.");
		return;
	}

	else{
		gyro_single_write(reg_address, reg_data);
		put_log_mesg("Data has been sent to device.");
	}

	return;	
}
// END OF SERIAL HANDLERS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
