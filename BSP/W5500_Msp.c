#include        "W5500_Msp.h"
#include        "socket.h"
#include        "delay_tim2.h"


SPI_HandleTypeDef       hspi1;
GPIO_InitTypeDef        GPIO_W5500_Struct;


void SPI_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    
    
    HAL_SPI_Init(&hspi1);
}


void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_W5500_Struct.Mode = GPIO_MODE_AF_PP;
    GPIO_W5500_Struct.Alternate = GPIO_AF5_SPI1;
    GPIO_W5500_Struct.Pin = GPIO_PIN_5 |GPIO_PIN_7;
    GPIO_W5500_Struct.Pull = GPIO_NOPULL;
    GPIO_W5500_Struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_W5500_Struct);
    
    GPIO_W5500_Struct.Pull = GPIO_PULLUP;
    GPIO_W5500_Struct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &GPIO_W5500_Struct);
    
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
    GPIO_W5500_Struct.Pin = CS_PIN;
    GPIO_W5500_Struct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_W5500_Struct.Pull = GPIO_NOPULL;
    GPIO_W5500_Struct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(CS_PORT, &GPIO_W5500_Struct);
}


void socket_run(uint8_t sn, pSocket_Control_Typedef sock_ctl, uint16_t port)
{
    uint8_t sr = getSn_SR(1);
        switch(sr)
        {
            case SOCK_CLOSED:
                #ifdef      TEST
                        test();
                #endif
                socket(1, Sn_MR_TCP, port, 0);
                break;
            case SOCK_LISTEN:
                #ifdef      TEST
                        test();
                #endif
                delay_ms(50);
                break;
            case SOCK_ESTABLISHED:
                #ifdef      TEST
                        test();
                #endif
                if(getSn_RX_RSR(1)>0)
                {
                    
                }
                delay_ms(50);
                break;
            case SOCK_INIT:
                #ifdef      TEST
                        test();
                #endif
                listen(1);
                break;
            case SOCK_CLOSE_WAIT:
                #ifdef      TEST
                        test();
                #endif
                disconnect(1);
                close(1);
                break;
            default:
                break;
        }
}