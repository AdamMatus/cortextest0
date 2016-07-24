#ifndef L3GD20_H_
#define L3GD20_H_

#define GYRO_WHO_AM_I  									(	(uint8_t) 0x0F )
#define GYRO_CTRL_REG1  								(	(uint8_t) 0x20 )
#define GYRO_CTRL_REG2  								(	(uint8_t) 0x21 )
#define GYRO_CTRL_REG3  								(	(uint8_t) 0x22 )
#define GYRO_CTRL_REG4  								(	(uint8_t) 0x23 )
#define GYRO_CTRL_REG5  								(	(uint8_t) 0x24 )
#define GYRO_REFERENCE  								(	(uint8_t) 0x25 )
#define GYRO_OUT_TEMP  									(	(uint8_t) 0x26 )
#define GYRO_STATUS_REG  								(	(uint8_t) 0x27 )
#define GYRO_OUT_X_L  									(	(uint8_t) 0x28 )
#define GYRO_OUT_X_H  									(	(uint8_t) 0x29 )
#define GYRO_OUT_Y_L  									(	(uint8_t) 0x2a )
#define GYRO_OUT_Y_H  									(	(uint8_t) 0x2b )
#define GYRO_OUT_Z_L  									(	(uint8_t) 0x2c )
#define GYRO_OUT_Z_H  									(	(uint8_t) 0x2d )
#define GYRO_FIFO_CTRL_REG  						(	(uint8_t) 0x2e )
#define GYRO_FIFO_SRC_REG  							(	(uint8_t) 0x2f )
#define GYRO_INT1_CFG  									(	(uint8_t) 0x30 )
#define GYRO_INT1_SRC  									(	(uint8_t) 0x31 )
#define GYRO_INT1_THS_XH  							(	(uint8_t) 0x32 )
#define GYRO_INT1_THS_XL  							(	(uint8_t) 0x33 )
#define GYRO_INT1_THS_YH  							(	(uint8_t) 0x34 )
#define GYRO_INT1_THS_YL  							(	(uint8_t) 0x35 )
#define GYRO_INT1_THS_ZH  							(	(uint8_t) 0x36 )
#define GYRO_INT1_THS_ZL  							(	(uint8_t) 0x37 )
#define GYRO_INT1_DURATION  						(	(uint8_t) 0x38 )

#endif
