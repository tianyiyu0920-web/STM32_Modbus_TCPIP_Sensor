#ifndef     _SHT31_H
#define     _SHT31_H
#include    "main.h"
#include    "oled.h"


#define     CRC_SHT31


#define     SHT31_ADD     0x44


#define SHT31_Reset_Soft()      SHT31_WR_Command(0x30A2)
#define SHT31_Reset_General     SHT31_WR_Command(0x0006)
#define SHT31_Heater_on()       SHT31_WR_Command(0x306D)
#define SHT31_Heater_off()      SHT31_WR_Command(0x3066)
#define SHT31_CMD_Read_SR()     SHT31_WR_Command(0xF32D)
#define SHT31_Clear_SR()        SHT31_WR_Command(0x3041)
#define SHT31_Mesurement_Single_CSE_High()      SHT31_WR_Command(0x2C06)
#define SHT31_Mesurement_Single_CSE_Medium()    SHT31_WR_Command(0x2C0D)
#define SHT31_Mesurement_Single_CSE_Low()       SHT31_WR_Command(0x2C10)
#define SHT31_Mesurement_Single_DSE_High()      SHT31_WR_Command(0x2400)
#define SHT31_Mesurement_Single_DSE_Medium()    SHT31_WR_Command(0x240B)
#define SHT31_Mesurement_Single_DSE_Low()       SHT31_WR_Command(0x2416)
#define SHT31_Mesurement_Priod_MPS_05_High()    SHT31_WR_Command(0x2032)
#define SHT31_Mesurement_Priod_MPS_05_Medium()  SHT31_WR_Command(0x2024)
#define SHT31_Mesurement_Priod_MPS_05_Low()     SHT31_WR_Command(0x202F)
#define SHT31_Mesurement_Priod_MPS_1_High()     SHT31_WR_Command(0x2130)
#define SHT31_Mesurement_Priod_MPS_1_Medium()   SHT31_WR_Command(0x2126)
#define SHT31_Mesurement_Priod_MPS_1_Low()      SHT31_WR_Command(0x212D)
#define SHT31_Mesurement_Priod_MPS_2_High()     SHT31_WR_Command(0x2236)
#define SHT31_Mesurement_Priod_MPS_2_Medium()   SHT31_WR_Command(0x2220)
#define SHT31_Mesurement_Priod_MPS_2_Low()      SHT31_WR_Command(0x222B)
#define SHT31_Mesurement_Priod_MPS_4_High()     SHT31_WR_Command(0x2334)
#define SHT31_Mesurement_Priod_MPS_4_Medium()   SHT31_WR_Command(0x2322)
#define SHT31_Mesurement_Priod_MPS_4_Low()      SHT31_WR_Command(0x2329)
#define SHT31_Mesurement_Priod_MPS_10_High()    SHT31_WR_Command(0x2737)
#define SHT31_Mesurement_Priod_MPS_10_Medium()  SHT31_WR_Command(0x2721)
#define SHT31_Mesurement_Priod_MPS_10_Low()     SHT31_WR_Command(0x272A)
#define SHT31_ART()     SHT31_WR_Command(0x2B32)
#define SHT31_Break()   SHT31_WR_Command(0x3093)
#define SHT31_Fetch()   SHT31_WR_Command(0xE000)


extern uint8_t SHT31_Rx_Buff[10];
extern float TRH[2];


int8_t SHT31_GetValue_TRH(uint8_t *buff, float *out_buff);
I2C_StateTypeDef SHT31_WR_Command(uint16_t command);
void SHT31_Init();

#endif
