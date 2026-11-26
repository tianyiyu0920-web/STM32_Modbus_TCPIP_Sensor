#include    "delay_tim2.h"


TIM_HandleTypeDef       htim2;
uint32_t g_fac_us = 0;


void Delay_Init(uint32_t tim2_clk)
{
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
    htim2.Init.Period = 65535;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.RepetitionCounter = 0;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    HAL_TIM_Base_Init(&htim2);
    HAL_TIM_Base_Start(&htim2);
    g_fac_us = tim2_clk;
}


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
}


void delay_us(uint32_t nus)
{
    uint32_t Tick = 0;
    uint32_t tstart = TIM2->CNT;
    uint32_t tnow = 0;
    uint32_t reload = TIM2->ARR;
    uint32_t cnt = nus*g_fac_us;
    while(Tick < cnt)
    {
        tnow = TIM2->CNT;
        if(tnow < tstart)
        {
            Tick+= (tstart - tnow);
            tstart = tnow;
            
        }
        else if(tnow > tstart)
        {
            Tick+= (tstart + (reload - tnow + 1));
            tstart = tnow;
        }
    }
    
}


void delay_ms(uint32_t nms)
{
    delay_us(1000*nms);
}