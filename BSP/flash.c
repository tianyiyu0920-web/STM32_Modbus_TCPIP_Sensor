#include    "flash.h"


HAL_StatusTypeDef   Flash_Erase(uint32_t add)
{
    uint32_t sector_head = 0;
    if(0x08000000<=add && add<=0x08003FFF)
    {
        sector_head = FLASH_SECTOR_0;
    }
    else if(0x08004000<=add && add<=0x08007FFF)
    {
        sector_head = FLASH_SECTOR_1;
    }
    else if(0x08008000<=add && add<=0x0800BFFF)
    {
        sector_head = FLASH_SECTOR_2;
    }
    else if(0x0800C000<=add && add<=0x0800FFFF)
    {
        sector_head = FLASH_SECTOR_3;
    }
    else if(0x08010000<=add && add<=0x0801FFFF)
    {
        sector_head = FLASH_SECTOR_4;
    }
    else if(0x08020000<=add && add<=0x0803FFFF)
    {
        sector_head = FLASH_SECTOR_5;
    }
    else if(0x08040000<=add && add<=0x0805FFFF)
    {
        sector_head = FLASH_SECTOR_6;
    }
    else if(0x08060000<=add && add<=0x0807FFFF)
    {
        sector_head = FLASH_SECTOR_7;
    }
    else
    {
        return HAL_ERROR;
    }
    uint32_t PageError;
    FLASH_EraseInitTypeDef  Flash_Erase_Struct = {0};
    Flash_Erase_Struct.NbSectors = 1;
    Flash_Erase_Struct.Sector = sector_head;
    Flash_Erase_Struct.TypeErase = FLASH_TYPEERASE_SECTORS;
    Flash_Erase_Struct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    if(HAL_FLASHEx_Erase(&Flash_Erase_Struct, &PageError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }
    HAL_FLASH_Lock();
    return HAL_OK;
}


HAL_StatusTypeDef   Flash_Write(uint32_t add, uint8_t *data, uint32_t size)
{
    uint32_t offset = 0;
    uint32_t word = 0;
    if(add%4)return HAL_ERROR;
    
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    for(uint32_t i=0;i<(size/4);i++)
    {
        memcpy(&word, data + offset, 4);
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, add, word) != HAL_OK)
            {
                HAL_FLASH_Lock();
                return HAL_ERROR;
            }
            offset += 4;
            add += 4;
    }
    if(size%4)
    {
        word = 0;
        uint8_t *p_rest = data + offset;
        for(uint8_t i=0;i<(size%4);i++)
        {
            word |= ((uint32_t)*p_rest << (8*i));
            p_rest++;
        }
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, add, word) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
    }
    HAL_FLASH_Lock();
    return HAL_OK;
}