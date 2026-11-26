#include        "W5500_Msp.h"
#include        "wizchip_conf.h"
#include        "FreeRTOS.h"
#include        "task.h"
#include        "delay_tim2.h"
#include        "oled.h"


void cs_select(void);
void cs_deselect(void);
uint8_t spi_read_byte(void);
void spi_write_byte(uint8_t data);
void spi_read_burst(uint8_t *pbuf, uint16_t len);
void spi_write_burst(uint8_t *pbuf, uint16_t len);
void cris_enter(void);
void cris_exit(void);


void W5500_Init(uint8_t ip[4], uint8_t sn[4], uint8_t gw[4], uint8_t dns[4])
{
    SPI_Init();
    
    GPIO_InitTypeDef    gpio_struct = {0};
    
    /* !!!!!!For Port Change, Please ensure the CLK will be correct enabled!!!!!! */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* enable INT_IRQN */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    gpio_struct.Mode = GPIO_MODE_IT_FALLING;
    gpio_struct.Pin = INT_PIN;
    gpio_struct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(INT_PORT, &gpio_struct);
    
    HAL_NVIC_SetPriority(INT_IRQN, 2, 0);
    HAL_NVIC_EnableIRQ(INT_IRQN);
    
    /* Init RST PIN */
    HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_SET);
    gpio_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_struct.Pin = RST_PIN;
    gpio_struct.Pull = GPIO_NOPULL;
    gpio_struct.Speed = GPIO_SPEED_MEDIUM;
    HAL_GPIO_Init(RST_PORT, &gpio_struct);
    
    
    /* Reset W5500 */
    RST(0);
    delay_ms(2);
    RST(1);
    
    
    /* Register all ioLibrary_Driver common control interfaces */
    reg_wizchip_cs_cbfunc(cs_select, cs_deselect);
    reg_wizchip_spi_cbfunc(spi_read_byte, spi_write_byte);
    reg_wizchip_spiburst_cbfunc(spi_read_burst, spi_write_burst);
    reg_wizchip_cris_cbfunc(cris_enter, cris_exit);
    
    
    /* Socket Buffer Init */
    uint8_t memsize[2][8] = 
    {
        {2,2,2,2,2,2,2,2,},
        {2,2,2,2,2,2,2,2,},
    };
    if(ctlwizchip(CW_INIT_WIZCHIP, (void*)memsize) == -1)
    {
        while(1);
    }
    
    
    /* Configure network information */
    wiz_NetInfo WIZNETINFO = 
    {
        .mac = {0x02,0x01,0x01,0x01,0x01,0x01},
        .dhcp = NETINFO_STATIC,
        .ip = {ip[0],ip[1],ip[2],ip[3]},
        .sn = {sn[0],sn[1],sn[2],sn[3]},
        .gw = {gw[0],gw[1],gw[2],gw[3]},
        .dns = {dns[0],dns[1],dns[2],dns[3]},
    };
    ctlnetwork(CN_SET_NETINFO, (void*)&WIZNETINFO);
}
    

extern  SPI_HandleTypeDef       hspi1;

void cs_select(void)
{
    CS(0);
}


void cs_deselect(void)
{
    CS(1);
}


uint8_t spi_read_byte(void)
{
    uint8_t dummy_data = 0xFF;
    uint8_t w5500_rx = 0;
    HAL_SPI_TransmitReceive(&hspi1, &dummy_data, &w5500_rx, 1, 1000);
    return w5500_rx;
}


void spi_write_byte(uint8_t data)
{
    uint8_t w5500_tx = data;
    HAL_SPI_Transmit(&hspi1, &w5500_tx, 1, 1000);
}


void spi_read_burst(uint8_t *pbuf, uint16_t len)
{
    HAL_SPI_Receive(&hspi1, pbuf, len, 1000);
}


void spi_write_burst(uint8_t *pbuf, uint16_t len)
{
    HAL_SPI_Transmit(&hspi1, pbuf, len, 1000);
}


void cris_enter(void)
{
    taskENTER_CRITICAL();
}


void cris_exit(void)
{
    taskEXIT_CRITICAL();
}

void test()
{
    char str_buf[20];
    uint8_t sta = getSn_SR(1);
    uint8_t x = 0;
    uint8_t y = 1;
    sprintf(str_buf, "SR: %02X", sta);
    OLED_WriteString(&x, &y, str_buf);
}