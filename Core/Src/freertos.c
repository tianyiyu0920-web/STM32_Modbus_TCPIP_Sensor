/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "Modbus_proto.h"
#include "oled.h"
#include "socket.h"
#include "W5500_Msp.h"
#include "delay_tim2.h"
#include "SHT31.h"
#include "flash.h"
#include "oledgif.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define         demo

#define         TASK_MODBUS_ROUNDING_STACK_SIZE     256
#define         TASK_MODBUS_ROUNDING_PRI            7
TaskHandle_t    Task_Modbus_Rounding_Handler;
void Task_Modbus_Rounding(void *pvargument);

#define         TASK_OLED_STACK_SIZE     256
#define         TASK_OLED_PRI            14
TaskHandle_t    Task_OLED_Handler;
void Task_OLED(void *pvargument);

#define         TASK_FETCH_TRH_STACK_SIZE     128
#define         TASK_FETCH_TRH_PRI            8
TaskHandle_t    Task_FETCH_TRH_Handler;
void Task_Fetch_TRH(void *pvargument);

#define         TASK_MODBUS_INPUT_REFRESH_STACK_SIZE     128
#define         TASK_MODBUS_INPUT_REFRESH_PRI            8
TaskHandle_t    Task_MODBUS_INPUT_REFRESH_Handler;
void Task_Modbus_Input_Refresh(void *pvargument);

#define         TASK_MODBUS_COMMAND_EXEC_STACK_SIZE     128
#define         TASK_MODBUS_COMMAND_EXEC_PRI            13
TaskHandle_t    Task_MODBUS_COMMAND_EXEC_Handler;
void Task_Modbus_Command_Exec(void *pvargument);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
      Delay_Init(100);
#ifdef  demo
    GPIO_InitTypeDef    sgpio;
    sgpio.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    sgpio.Mode = GPIO_MODE_INPUT;
    sgpio.Speed = GPIO_SPEED_FREQ_LOW;
    sgpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &sgpio);
#endif
      OLED_Init();
      OLED_WriteGif((uint8_t *)oled_gif_bai);
      OLED_Clear();
      if(Modbus_Init() == -1)HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
      OLED_Set_Brightness((uint8_t)Modbus_Register_Struct.AO[AO_OLED_BRIGHTNESS]);
      SHT31_Init();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  xTaskCreate((TaskFunction_t) Task_Modbus_Rounding,
                (const char * const) "Task_Modbus_Rounding",
                (const configSTACK_DEPTH_TYPE) TASK_MODBUS_ROUNDING_STACK_SIZE,
                (void * const) NULL,
                (UBaseType_t) TASK_MODBUS_ROUNDING_PRI,
                (TaskHandle_t * const) Task_Modbus_Rounding_Handler );
  xTaskCreate((TaskFunction_t) Task_OLED,
                (const char * const) "Task_OLED",
                (const configSTACK_DEPTH_TYPE) TASK_OLED_STACK_SIZE,
                (void * const) NULL,
                (UBaseType_t) TASK_OLED_PRI,
                (TaskHandle_t * const) Task_OLED_Handler );
  xTaskCreate((TaskFunction_t) Task_Fetch_TRH,
                (const char * const) "Task_Fetch_TRH",
                (const configSTACK_DEPTH_TYPE) TASK_FETCH_TRH_STACK_SIZE,
                (void * const) NULL,
                (UBaseType_t) TASK_FETCH_TRH_PRI,
                (TaskHandle_t * const) Task_FETCH_TRH_Handler );
  xTaskCreate((TaskFunction_t) Task_Modbus_Input_Refresh,
                (const char * const) "Task_Modbus_Input_Refresh",
                (const configSTACK_DEPTH_TYPE) TASK_MODBUS_INPUT_REFRESH_STACK_SIZE,
                (void * const) NULL,
                (UBaseType_t) TASK_MODBUS_INPUT_REFRESH_PRI,
                (TaskHandle_t * const) Task_MODBUS_INPUT_REFRESH_Handler );
  xTaskCreate((TaskFunction_t) Task_Modbus_Command_Exec,
                (const char * const) "Task_Modbus_Command_Exec",
                (const configSTACK_DEPTH_TYPE) TASK_MODBUS_COMMAND_EXEC_STACK_SIZE,
                (void * const) NULL,
                (UBaseType_t) TASK_MODBUS_COMMAND_EXEC_PRI,
                (TaskHandle_t * const) Task_MODBUS_COMMAND_EXEC_Handler );
  vTaskDelete(NULL);
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void Task_Modbus_Rounding(void *pvargument)
{
    uint16_t recv_len = 0;
    uint16_t sned_len = 0;
    while(1)
    {
        uint8_t sr = getSn_SR(1);
            switch(sr)
            {
                case SOCK_CLOSED:
                    socket(1, Sn_MR_TCP, Modbus_Register_Struct.AO[AO_TCP_PORT], 0);
                    break;
                case SOCK_LISTEN:
                    vTaskDelay(50);
                    break;
                case SOCK_ESTABLISHED:
                    if((recv_len = getSn_RX_RSR(1))>0)
                    {
                        Modbus_Recv(1, &Modbus_Ctl, 2000);
                        Modbus_Process(&Modbus_Ctl);
                        Modbus_Send(1, &Modbus_Ctl);
                    }
                    vTaskDelay(5);
                    break;
                case SOCK_INIT:
                    listen(1);
                    break;
                case SOCK_CLOSE_WAIT:
                    disconnect(1);
                    close(1);
                    break;
                default:
                    break;
            }
    }
}


void Task_OLED(void *pvargument)
{
    uint8_t x = 0;
    uint8_t y = 0;
    char IP[17] = {0};
    sprintf(IP, "%d.%d.%d.%d", Firmware_Info_Load_Struct.ip[0], Firmware_Info_Load_Struct.ip[1],\
            Firmware_Info_Load_Struct.ip[2], Firmware_Info_Load_Struct.ip[3]);
    while(1)
    {
        x = 0;
        y = 0;
        OLED_WriteString(&x, &y, IP);
        x = 0;
        y = 1;
        uint8_t sr = getSn_SR(1);
        OLED_WriteString(&x, &y, "SR:");
        OLED_WriteNumber_DEC(&x, &y, sr);
        x = 0;
        y = 2;
        OLED_WriteString(&x, &y, "TEMP=");
        OLED_WriteFloat(&x, &y, TRH[0]);
        OLED_WriteChar(&x, &y, 'C');
        x = 0;
        y = 3;
        OLED_WriteString(&x, &y, "HUMI=");
        OLED_WriteFloat(&x, &y, TRH[1]);
        OLED_WriteChar(&x, &y, '%');
        vTaskDelay(1000);
    }
}


void Task_Fetch_TRH(void *pvargument)
{
    while(1)
    {
        if(SHT31_Mesurement_Single_DSE_High() == I2C_OK)
        {
            vTaskDelay(20);
            SHT31_GetValue_TRH(SHT31_Rx_Buff, TRH);
        }
        vTaskDelay(1000);
    }
}


void Task_Modbus_Input_Refresh(void *pvargument)
{
    uint16_t temp = 0;
    while(1)
    {
        Modbus_Register_Struct.AI[AI_TEM] = (uint16_t)(TRH[0]*10);
        Modbus_Register_Struct.AI[AI_HUMI] = (uint16_t)(TRH[1]*10);
        vTaskDelay(2000);
    }
}


void Task_Modbus_Command_Exec(void *pvargument)
{
    uint16_t status;
    uint16_t oled_brightness = Modbus_Register_Struct.AO[AO_OLED_BRIGHTNESS];
    while(1)
    {
        status = Modbus_Register_Struct.AO[AO_SYS_COMMAND];
        if(status)
        {
            switch(status)
            {
                case(CMD_SAVE_AND_REBOOT):
                /* AO寄存器写回  更新FLASH数据   复位更新 */
                    if(Modbus_Register_Struct.AI[AI_SYS_STATUS] & ERR_FLASH_PROGRAM_FAIL)break;
                    Modbus_Firmware_Reload();
                    uint16_t count;
                    Firmware_Info_TypeDef *p_find = Find_Firmware_Info((Firmware_Info_TypeDef*)FIRM_WARE_ADD, &count);
                    if(count == MAX_CONFIG_RECORDS)
                    {
                        taskENTER_CRITICAL();
                        if(Modbus_Flash_Erase() == -1)
                        {
                            Modbus_Register_Struct.AI[AI_SYS_STATUS] |= ERR_FLASH_PROGRAM_FAIL;
                        }
                        if(Modbus_Flash_Write((uint32_t)FIRM_WARE_ADD, (uint8_t*)&Firmware_Info_Load_Struct, sizeof(Firmware_Info_TypeDef)) == -1)
                        {
                            Modbus_Register_Struct.AI[AI_SYS_STATUS] |= ERR_FLASH_PROGRAM_FAIL;
                        };
                        taskEXIT_CRITICAL();
                    }
                    else if(count > MAX_CONFIG_RECORDS)
                    {
                        Modbus_Register_Struct.AI[AI_SYS_STATUS] |= (ERR_FLASH_CFG_OOB | STS_CFG_FALLBACK_DEFAULT);
                        taskENTER_CRITICAL();
                        if(Modbus_Flash_Erase() == -1)
                        {
                            Modbus_Register_Struct.AI[AI_SYS_STATUS] |= ERR_FLASH_PROGRAM_FAIL;
                        }
                        if(Modbus_Flash_Write((uint32_t)FIRM_WARE_ADD, (uint8_t*)&Firmware_Info_Default_Ram_Struct, sizeof(Firmware_Info_TypeDef)) == -1)
                        {
                            Modbus_Register_Struct.AI[AI_SYS_STATUS] |= ERR_FLASH_PROGRAM_FAIL;
                        }
                        taskEXIT_CRITICAL();
                    }
                    else
                    {
                        p_find ++;
                        taskENTER_CRITICAL();
                        if(Modbus_Flash_Write((uint32_t)p_find,(uint8_t*)&Firmware_Info_Load_Struct, sizeof(Firmware_Info_TypeDef)) == -1)
                        {
                            Modbus_Register_Struct.AI[AI_SYS_STATUS] |= ERR_FLASH_PROGRAM_FAIL;
                        };
                        taskEXIT_CRITICAL();
                    }
                    NVIC_SystemReset();
                    break;
                case(CMD_CLEAR_LATCHED_FAULTS):
                    Modbus_Register_Struct.AI[AI_SYS_STATUS] &= ~SYS_LATCHED_ERR_MASK;
                    break;
                default:
                    break;
            }
            Modbus_Register_Struct.AO[AO_SYS_COMMAND] = 0;
            status = 0;
        }
        if(Modbus_Register_Struct.AO[AO_OLED_BRIGHTNESS] != oled_brightness)
        {
            oled_brightness = Modbus_Register_Struct.AO[AO_OLED_BRIGHTNESS];
            OLED_Set_Brightness((uint8_t)oled_brightness);
        }
        vTaskDelay(1000);
    }
}
/* USER CODE END Application */

