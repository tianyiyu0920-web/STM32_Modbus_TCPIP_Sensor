#ifndef _OLED_H
#define  _OLED_H
#include  "main.h"

#define ACK           0x01
#define NACK          0x00


#define I2C_SDA_PORT  GPIOB
#define I2C_SCL_PORT  GPIOB
#define I2C_SDA_PIN   GPIO_PIN_15
#define I2C_SCL_PIN   GPIO_PIN_14
#define I2C_SDA_WR(x)  (I2C_SDA_PORT->BSRR = ((uint32_t)I2C_SDA_PIN<<(16*!(x))))

#define I2C_SCL_WR(x)  (I2C_SCL_PORT->BSRR = ((uint32_t)I2C_SCL_PIN<<(16*!(x))))
//#define I2C_SDA_WR(x)   HAL_GPIO_WritePin(I2C_SDA_PORT, I2C_SDA_PIN, (x))
//#define I2C_SCL_WR(x)   HAL_GPIO_WritePin(I2C_SCL_PORT, I2C_SCL_PIN, (x))
#define I2C_SCL_RD()   HAL_GPIO_ReadPin(I2C_SCL_PORT, I2C_SCL_PIN)
#define I2C_SDA_RD()   HAL_GPIO_ReadPin(I2C_SDA_PORT, I2C_SDA_PIN)
//#define I2C_SCL_RD()   (I2C_SCL_PORT-> IDR& I2C_SCL_PIN)
//#define I2C_SDA_RD()   (I2C_SDA_PORT-> IDR& I2C_SDA_PIN)


typedef enum
{
    I2C_OK,
    I2C_ERROR,
    I2C_TIMEOUT,
}I2C_StateTypeDef;


//void I2C_Delay();
//uint8_t I2C_CallSlave (uint8_t add, uint8_t cmd);
//uint8_t I2C_SendByte (uint8_t data);
//void I2C_Stop(void);
I2C_StateTypeDef I2C_WaitAck(uint16_t timeout);
void I2C_Start();
I2C_StateTypeDef I2C_CallSlave(uint8_t add, uint8_t cmd);
I2C_StateTypeDef I2C_SendByte(uint8_t data);
uint8_t I2C_ReadByte(uint8_t ack);
void I2C_Stop();

void OLED_Init(void);
void OLED_Set_Brightness(uint8_t brightness);
void OLED_Refresh(void);
void OLED_WritePoint(uint8_t x, uint8_t y);
void OLED_WriteChar(uint8_t* x, uint8_t* y, char c);
void OLED_WriteString(uint8_t *x, uint8_t *y, char* str);
void OLED_WriteNumber_DEC(uint8_t *x, uint8_t *y, int32_t number);
void OLED_WriteFloat(uint8_t *x, uint8_t *y, float number);
void OLED_WriteImage(uint8_t *pixelbyte);
void OLED_WriteGif(uint8_t *pixelbyte);
void OLED_Clear(void);
void OLED_WriteNumber_HEX(uint8_t *x, uint8_t *y, uint32_t number);


#endif
