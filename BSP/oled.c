#include "oled.h"
#include "delay_tim2.h"
#include "oledfont.h"


static uint8_t cmd=0x00;
static uint8_t data=0x40;
static uint8_t init_init_cmds[25] = {
    0xAE, // 1. 关闭显示 (Sleep Mode)
    
    0xD5, // 2. 设置时钟分频因子/振荡频率
    0x80, //    [默认值] 分频1, 频率最高
    
    0xA8, // 3. 设置多路复用率 (Multiplex Ratio)
    0x3F, //    1/64 驱动 (对应 64 行全显示)
    
    0xD3, // 4. 设置显示偏移 (Display Offset)
    0x00, //    无偏移
    
    0x40, // 5. 设置显示开始行 (Display Start Line)
          //    从 RAM 的第 0 行开始读取显示
          
    0x8D, // 6. 核心：电荷泵设置 (Charge Pump Setting) ??最关键的一步
    0x14, //    0x14 代表开启电荷泵 (若不开启，屏幕无驱动电压，绝对不亮)
    
    0x20, // 7. 设置内存寻址模式 (Memory Addressing Mode)
    0x02, //    水平寻址模式 (方便后续整屏刷写)
    
    0xA1, // 8. 段重映射 (Segment Remap)
          //    0xA0左右反置, 0xA1正常 (配合 0xC8 实现顺时针 180 度翻转)
          
    0xC8, // 9. COM 输出扫描方向 (COM Output Scan Direction)
          //    0xC0上下反置, 0xC8正常
          
    0xDA, // 10. COM 硬件引脚配置 (COM Pins Hardware Configuration)
    0x12, //     针对 128x64 矩阵的交错引脚配置
    0x81, // 11. 对比度设置 (Contrast Control)
    0xCF, //     亮度值 (0x00~0xFF)，0xCF 是很亮但相对安全的亮度
    0xD9, // 12. 设置预充电周期 (Pre-charge Period)
    0xF1, //     提升面板驱动稳定性
    0xDB, // 13. VCOMH 电压倍率设置 (VCOMH Deselect Level)
    0x30, //     ~0.83 x VCC
    0xA4, // 14. 全局显示开启 (输出 RAM 中的内容)
    0xA6, // 15. 设置正常显示 (0xA6 正常显示, 0xA7 反白显示)
    0xAF, // 16. 开启显示 (Display ON) ??唤醒芯片

};


static inline void I2C_Delay()
{
    delay_us(4);
}


I2C_StateTypeDef I2C_WaitAck(uint16_t timeout)
{
    uint16_t timestart = HAL_GetTick();
    uint16_t timenow = 0;
    while(I2C_SDA_RD())
    {
        timenow = HAL_GetTick();
        if((timenow - timestart)>timeout)
        {
            return I2C_TIMEOUT;
        }
    }
    return I2C_OK;
}


static inline void I2C_ReleaseBus()
{
    I2C_SDA_WR(1);
    I2C_SCL_WR(1);
}


void I2C_Start()
{
    I2C_SDA_WR(1);
    I2C_SCL_WR(1);
    I2C_Delay(); 
    
    I2C_SDA_WR(0);
    I2C_Delay();
    I2C_SCL_WR(0);
    I2C_Delay();
}


I2C_StateTypeDef I2C_CallSlave(uint8_t add, uint8_t cmd)
{
    I2C_Start();
    uint8_t temp = ((add<<1)|cmd);
    for(uint8_t i=0; i<8;i++)
    {
        if(!(temp&0x80))
        {
            I2C_SDA_WR(0);
            I2C_Delay();
        }
        else
        {
            I2C_SDA_WR(1);
            I2C_Delay();
        }
        I2C_SCL_WR(1);
        I2C_Delay();
        I2C_SCL_WR(0);
        I2C_Delay();
        temp = (temp<<1);
    }
    I2C_SDA_WR(1);
    I2C_Delay();
    I2C_SCL_WR(1);
    I2C_Delay();
    if(I2C_WaitAck(500) == I2C_OK)
    {
        I2C_SCL_WR(0);
        I2C_Delay();
        return I2C_OK;
    }
    else
    {
        I2C_SCL_WR(0);
        I2C_Delay();
        return I2C_TIMEOUT;
    }
}


I2C_StateTypeDef I2C_SendByte(uint8_t data)
{
        for(uint8_t i=0; i<8;i++)
    {
        if(!(data&0x80))
        {
            I2C_SDA_WR(0);
            I2C_Delay();
        }
        else
        {
            I2C_SDA_WR(1);
            I2C_Delay();
        }
        I2C_SCL_WR(1);
        I2C_Delay();
        I2C_SCL_WR(0);
        I2C_Delay();
        data = (data<<1);
    }
    I2C_SDA_WR(1);
    I2C_Delay();
    I2C_SCL_WR(1);
    I2C_Delay();
    if(I2C_WaitAck(500) == I2C_OK)
    {
        I2C_SCL_WR(0);
        I2C_Delay();
        return I2C_OK;
    }
    else
    {
        I2C_SCL_WR(0);
        I2C_Delay();
        return I2C_TIMEOUT;
    }
}


uint8_t I2C_ReadByte(uint8_t ack)
{
    I2C_SDA_WR(1);
    I2C_Delay();
    uint8_t val = 0;
    for(uint8_t i=0;i<8;i++)
    {
        I2C_SCL_WR(1);
        I2C_Delay();
        val <<= 1;
        if(I2C_SDA_RD()) val |= 0x01;
        I2C_SCL_WR(0);
        I2C_Delay();
    }
/* ack frame */
    if(ack) I2C_SDA_WR(0);
    else I2C_SDA_WR(1);
    I2C_Delay();
    I2C_SCL_WR(1);
    I2C_Delay();
    I2C_SCL_WR(0);
    I2C_Delay();
    I2C_SDA_WR(1);
    I2C_Delay();
    return val;
}


void I2C_Stop()
{
    I2C_SDA_WR(0);
    I2C_Delay();
    I2C_SCL_WR(1);
    I2C_Delay();
    I2C_SDA_WR(1);
    I2C_Delay();
}


void OLED_Clear(void)
{
    I2C_ReleaseBus();
    for(uint8_t i=0;i<8;i++)
    {
        I2C_CallSlave(0x3C,0);
        I2C_SendByte(0x00);
        I2C_SendByte(0xB0|i);
        I2C_SendByte(0x02);
        I2C_SendByte(0x10);
        I2C_Stop();
        I2C_CallSlave(0x3C,0);
        I2C_SendByte(0x40);
        for(uint8_t j=0;j<128;j++)
        {
            I2C_SendByte(0);
        }
        I2C_Stop();
    }
}


void OLED_Init(void)
{
    /* !!!!!!For Port Change, Please ensure the CLK will be correct enabled!!!!!! */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_Init;
    GPIO_Init.Pin=I2C_SDA_PIN;
    GPIO_Init.Mode=GPIO_MODE_OUTPUT_OD;
    GPIO_Init.Speed=GPIO_SPEED_FREQ_MEDIUM;
    GPIO_Init.Pull=GPIO_PULLUP;
    GPIO_Init.Alternate = 0;
    
    HAL_GPIO_Init(I2C_SDA_PORT, &GPIO_Init);
    
    GPIO_Init.Pin=I2C_SCL_PIN;
    HAL_GPIO_Init(I2C_SCL_PORT, &GPIO_Init);
    
    I2C_ReleaseBus();
    
    I2C_CallSlave(0x3C, 0);
    I2C_SendByte(0x00);
    for(uint8_t i=0;i<25;i++)
    {
        I2C_SendByte(init_init_cmds[i]);
    }
    I2C_Stop();
    
    OLED_Clear();
}


void OLED_Set_Brightness(uint8_t brightness)
{
    I2C_CallSlave(0x3C, 0);
    I2C_SendByte(0x00);
    I2C_SendByte(0x81);
    I2C_SendByte(brightness);
    I2C_Stop();
}


void OLED_Refresh(void)
{
    I2C_ReleaseBus();
    
    for(uint8_t i=0;i<8;i++)
    {
        uint8_t page=(0xB0+i);
        I2C_CallSlave(0x3C,0);
        I2C_SendByte(0x00);
        I2C_SendByte(page);
        I2C_SendByte(0x02);
        I2C_SendByte(0x10);
        I2C_Stop();
        I2C_CallSlave(0x3C,0);
        I2C_SendByte(0x40);
        for(uint8_t j=0;j<128;j++)
        {
            I2C_SendByte(oled_gram[i][j]);
        }
        I2C_Stop();
    }
}


void OLED_WritePoint(uint8_t x, uint8_t y)
{
    uint8_t page;
    uint8_t col=x;
    uint8_t data;
    page=y/8;
    data=1<<(y%8);
    oled_gram[page][col]=data;
    OLED_Refresh();
}


void OLED_WriteImage(uint8_t *pixelbyte)
{
    I2C_ReleaseBus();
    for(uint8_t i=0;i<8;i++)
    {
        uint8_t page=(0xB0+i);
        I2C_CallSlave(0x3C,0);
        I2C_SendByte(0x00);
        I2C_SendByte(page);
        I2C_SendByte(0x02);
        I2C_SendByte(0x10);
        I2C_Stop();
        I2C_CallSlave(0x3C,0);
        I2C_SendByte(0x40);
        for(uint8_t j=0;j<128;j++)
        {
            I2C_SendByte(*pixelbyte);
            pixelbyte++;
        }
        I2C_Stop();
    }
}


void OLED_WriteGif(uint8_t *pixelbyte)
{
    uint8_t lengh= *pixelbyte;
    pixelbyte++;
    for(int i=0; i<lengh;i++)
    {
        OLED_WriteImage(pixelbyte);
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
        HAL_Delay(20);
        pixelbyte+=1024;
    }
}




void OLED_WriteChar(uint8_t* x, uint8_t* y, char c)
{
    I2C_ReleaseBus();
    uint8_t char_array_nr= c- ' ';
    uint8_t page= (*y)*2;
    uint8_t col= ((*x)*8)+2;
    for(uint8_t i=0;i<2;i++)
    {
        I2C_CallSlave(0x3C, 0);
        I2C_SendByte(0x00);
        I2C_SendByte(0xB0|(page+i));
        I2C_SendByte(0x00|(col&0x0F));
        I2C_SendByte(0x10|((col&0xF0)>>4));
        I2C_Stop();
        I2C_CallSlave(0x3C, 0);
        I2C_SendByte(0x40);
        for(uint8_t j=0;j<8;j++)
        {
            I2C_SendByte(oled_asc2_1608[char_array_nr][j+8*i]);
        }
        I2C_Stop();
    }
    (*x)+=1;
    if((*x)>15)
    {
        (*y)+=1;
        *x = 0;
        if((*y)>3)
        {
            *y = 0;
        }
    }
}


void OLED_WriteString(uint8_t *x, uint8_t *y, char* str)
{
    uint8_t i=0;
    while(str[i]!='\0')
    {
        if((*x)>15)
        {
            (*x)=0;
            (*y)++;
        }
        OLED_WriteChar(x, y, str[i]);
        i++;
    }
}


void OLED_WriteNumber_DEC(uint8_t *x, uint8_t *y, int32_t number)
{
    if(number!=0)
    {
        if(number< 0)
        {
            OLED_WriteChar(x, y, '-');
            number = -number;
        }
        char number_string[10]={0};
        for(uint8_t i=0;i<10;i++)
        {
            number_string[i]=((number%10)+48);
            number=number/10;
        }
        uint8_t j=9;
        while(number_string[j]==48)
        {
            j--;
        }
        for(int i=j; i>=0;i--)
        {
            OLED_WriteChar(x, y, number_string[i]);
        }
        
    }
    else
    {
        OLED_WriteChar(x, y, '0');
    }
}


void OLED_WriteNumber_HEX(uint8_t *x, uint8_t *y, uint32_t number)
{
    if(number!= 0)
    {
        OLED_WriteString(x, y, "0x");
        char number_string[8]={0};
        for(uint8_t i=0; i<8; i++)
        {
            uint8_t asc2_number;
            if(number%16<10)
            {
                asc2_number= ((number%16)+'0');
            }
            else
            {
                asc2_number= ((number%16)-10+'A');
            }
            number_string[i]= asc2_number;
            number= number/16;
        }
        uint8_t j=7;
        while(number_string[j]=='0')
        {
            j--;
        }
        for(int i=j; i>=0; i--)
        {
            OLED_WriteChar(x, y, number_string[i]);
        }
    }
    else
    {
        OLED_WriteChar(x, y, '0');
    }
}
    

void OLED_WriteFloat(uint8_t *x, uint8_t *y, float number)
{
    if(number<0)
    {
        OLED_WriteChar(x, y, '-');
        number = -number;
    }
    int32_t Integer=(int32_t)number;
    uint32_t Decimal=(uint32_t)((number- Integer)*100.0+0.5);
    if (Decimal >= 100) 
    {
        Integer += 1;
        Decimal -= 100;
    }
    OLED_WriteNumber_DEC(x, y, Integer);
    OLED_WriteChar(x, y, '.');
    uint8_t j= (Decimal/10)+'0';
    OLED_WriteChar(x, y, j);
    uint8_t k= (Decimal%10)+'0';
    OLED_WriteChar(x, y, k);
}