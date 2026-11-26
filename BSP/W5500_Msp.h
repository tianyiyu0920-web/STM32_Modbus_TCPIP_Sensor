#ifndef     _W5500_MSP_H
#define     _W5500_MSP_H
#include    "main.h"


#define     CS_PORT     GPIOA
#define     CS_PIN      GPIO_PIN_8
#define     CS(x)       (CS_PORT->BSRR = (CS_PIN<<(16*!(x))))


#define     RST_PORT    GPIOA
#define     RST_PIN     GPIO_PIN_3
#define     RST(x)      (RST_PORT->BSRR = (RST_PIN<<(16*!(x))))


#define     INT_PORT    GPIOA
#define     INT_PIN     GPIO_PIN_4
#define     INT_IRQN    EXTI4_IRQn


#define     BUFF_SIZE   50
#define     PORT        502

typedef struct
{
    uint8_t     rx_buf[BUFF_SIZE];
    uint8_t     tx_buf[512];
    uint16_t    data_size;
}Socket_Control_TypeDef, *pSocket_Control_Typedef;


void SPI_Init(void);
void W5500_Init(uint8_t *p_ip, uint8_t *p_sn, uint8_t *p_gw, uint8_t *p_dns);
void test();
#endif
