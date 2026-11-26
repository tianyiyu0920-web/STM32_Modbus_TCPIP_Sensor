#ifndef     _FLASH_H
#define     _FLASH_H
#include    "main.h"


HAL_StatusTypeDef   Flash_Erase(uint32_t add);
HAL_StatusTypeDef   Flash_Write(uint32_t add, uint8_t *data, uint32_t size);

#endif
