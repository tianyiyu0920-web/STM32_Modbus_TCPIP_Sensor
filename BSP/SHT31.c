#include    "SHT31.h"
#include    "oled.h"
#include    "delay_tim2.h"


uint8_t SHT31_Rx_Buff[10] = {0};
float TRH[2] = {0};

#ifdef  CRC_SHT31
uint8_t CRC_Table[256] = {
[0x00] = 0x00,[0X01] = 0X31,[0X02] = 0X62,[0X03] = 0X53,
[0X04] = 0XC4,[0X05] = 0XF5,[0X06] = 0XA6,[0X07] = 0X97,
[0X08] = 0XB9,[0X09] = 0X88,[0X0A] = 0XDB,[0X0B] = 0XEA,
[0X0C] = 0X7D,[0X0D] = 0X4C,[0X0E] = 0X1F,[0X0F] = 0X2E,
[0X10] = 0X43,[0X11] = 0X72,[0X12] = 0X21,[0X13] = 0X10,
[0X14] = 0X87,[0X15] = 0XB6,[0X16] = 0XE5,[0X17] = 0XD4,
[0X18] = 0XFA,[0X19] = 0XCB,[0X1A] = 0X98,[0X1B] = 0XA9,
[0X1C] = 0X3E,[0X1D] = 0X0F,[0X1E] = 0X5C,[0X1F] = 0X6D,
[0X20] = 0X86,[0X21] = 0XB7,[0X22] = 0XE4,[0X23] = 0XD5,
[0X24] = 0X42,[0X25] = 0X73,[0X26] = 0X20,[0X27] = 0X11,
[0X28] = 0X3F,[0X29] = 0X0E,[0X2A] = 0X5D,[0X2B] = 0X6C,
[0X2C] = 0XFB,[0X2D] = 0XCA,[0X2E] = 0X99,[0X2F] = 0XA8,
[0X30] = 0XC5,[0X31] = 0XF4,[0X32] = 0XA7,[0X33] = 0X96,
[0X34] = 0X01,[0X35] = 0X30,[0X36] = 0X63,[0X37] = 0X52,
[0X38] = 0X7C,[0X39] = 0X4D,[0X3A] = 0X1E,[0X3B] = 0X2F,
[0X3C] = 0XB8,[0X3D] = 0X89,[0X3E] = 0XDA,[0X3F] = 0XEB,
[0X40] = 0X3D,[0X41] = 0X0C,[0X42] = 0X5F,[0X43] = 0X6E,
[0X44] = 0XF9,[0X45] = 0XC8,[0X46] = 0X9B,[0X47] = 0XAA,
[0X48] = 0X84,[0X49] = 0XB5,[0X4A] = 0XE6,[0X4B] = 0XD7,
[0X4C] = 0X40,[0X4D] = 0X71,[0X4E] = 0X22,[0X4F] = 0X13,
[0X50] = 0X7E,[0X51] = 0X4F,[0X52] = 0X1C,[0X53] = 0X2D,
[0X54] = 0XBA,[0X55] = 0X8B,[0X56] = 0XD8,[0X57] = 0XE9,
[0X58] = 0XC7,[0X59] = 0XF6,[0X5A] = 0XA5,[0X5B] = 0X94,
[0X5C] = 0X03,[0X5D] = 0X32,[0X5E] = 0X61,[0X5F] = 0X50,
[0X60] = 0XBB,[0X61] = 0X8A,[0X62] = 0XD9,[0X63] = 0XE8,
[0X64] = 0X7F,[0X65] = 0X4E,[0X66] = 0X1D,[0X67] = 0X2C,
[0X68] = 0X02,[0X69] = 0X33,[0X6A] = 0X60,[0X6B] = 0X51,
[0X6C] = 0XC6,[0X6D] = 0XF7,[0X6E] = 0XA4,[0X6F] = 0X95,
[0X70] = 0XF8,[0X71] = 0XC9,[0X72] = 0X9A,[0X73] = 0XAB,
[0X74] = 0X3C,[0X75] = 0X0D,[0X76] = 0X5E,[0X77] = 0X6F,
[0X78] = 0X41,[0X79] = 0X70,[0X7A] = 0X23,[0X7B] = 0X12,
[0X7C] = 0X85,[0X7D] = 0XB4,[0X7E] = 0XE7,[0X7F] = 0XD6,
[0X80] = 0X7A,[0X81] = 0X4B,[0X82] = 0X18,[0X83] = 0X29,
[0X84] = 0XBE,[0X85] = 0X8F,[0X86] = 0XDC,[0X87] = 0XED,
[0X88] = 0XC3,[0X89] = 0XF2,[0X8A] = 0XA1,[0X8B] = 0X90,
[0X8C] = 0X07,[0X8D] = 0X36,[0X8E] = 0X65,[0X8F] = 0X54,
[0X90] = 0X39,[0X91] = 0X08,[0X92] = 0X5B,[0X93] = 0X6A,
[0X94] = 0XFD,[0X95] = 0XCC,[0X96] = 0X9F,[0X97] = 0XAE,
[0X98] = 0X80,[0X99] = 0XB1,[0X9A] = 0XE2,[0X9B] = 0XD3,
[0X9C] = 0X44,[0X9D] = 0X75,[0X9E] = 0X26,[0X9F] = 0X17,
[0XA0] = 0XFC,[0XA1] = 0XCD,[0XA2] = 0X9E,[0XA3] = 0XAF,
[0XA4] = 0X38,[0XA5] = 0X09,[0XA6] = 0X5A,[0XA7] = 0X6B,
[0XA8] = 0X45,[0XA9] = 0X74,[0XAA] = 0X27,[0XAB] = 0X16,
[0XAC] = 0X81,[0XAD] = 0XB0,[0XAE] = 0XE3,[0XAF] = 0XD2,
[0XB0] = 0XBF,[0XB1] = 0X8E,[0XB2] = 0XDD,[0XB3] = 0XEC,
[0XB4] = 0X7B,[0XB5] = 0X4A,[0XB6] = 0X19,[0XB7] = 0X28,
[0XB8] = 0X06,[0XB9] = 0X37,[0XBA] = 0X64,[0XBB] = 0X55,
[0XBC] = 0XC2,[0XBD] = 0XF3,[0XBE] = 0XA0,[0XBF] = 0X91,
[0XC0] = 0X47,[0XC1] = 0X76,[0XC2] = 0X25,[0XC3] = 0X14,
[0XC4] = 0X83,[0XC5] = 0XB2,[0XC6] = 0XE1,[0XC7] = 0XD0,
[0XC8] = 0XFE,[0XC9] = 0XCF,[0XCA] = 0X9C,[0XCB] = 0XAD,
[0XCC] = 0X3A,[0XCD] = 0X0B,[0XCE] = 0X58,[0XCF] = 0X69,
[0XD0] = 0X04,[0XD1] = 0X35,[0XD2] = 0X66,[0XD3] = 0X57,
[0XD4] = 0XC0,[0XD5] = 0XF1,[0XD6] = 0XA2,[0XD7] = 0X93,
[0XD8] = 0XBD,[0XD9] = 0X8C,[0XDA] = 0XDF,[0XDB] = 0XEE,
[0XDC] = 0X79,[0XDD] = 0X48,[0XDE] = 0X1B,[0XDF] = 0X2A,
[0XE0] = 0XC1,[0XE1] = 0XF0,[0XE2] = 0XA3,[0XE3] = 0X92,
[0XE4] = 0X05,[0XE5] = 0X34,[0XE6] = 0X67,[0XE7] = 0X56,
[0XE8] = 0X78,[0XE9] = 0X49,[0XEA] = 0X1A,[0XEB] = 0X2B,
[0XEC] = 0XBC,[0XED] = 0X8D,[0XEE] = 0XDE,[0XEF] = 0XEF,
[0XF0] = 0X82,[0XF1] = 0XB3,[0XF2] = 0XE0,[0XF3] = 0XD1,
[0XF4] = 0X46,[0XF5] = 0X77,[0XF6] = 0X24,[0XF7] = 0X15,
[0XF8] = 0X3B,[0XF9] = 0X0A,[0XFA] = 0X59,[0XFB] = 0X68,
[0XFC] = 0XFF,[0XFD] = 0XCE,[0XFE] = 0X9D,[0XFF] = 0XAC,};
#endif


#ifdef CRC_SHT31
uint8_t _CRC_SHT31(uint8_t *buff, uint8_t size)
{
    uint8_t start_val = 0xFF;
    for(uint8_t i=0;i<size;i++)
    {
        start_val ^= buff[i];
        start_val = CRC_Table[start_val];
    }
    return start_val;
}
#endif


I2C_StateTypeDef SHT31_RD(uint8_t *rx_buff, uint8_t size)
{
    I2C_Start();
    if(I2C_CallSlave(SHT31_ADD, 1) != I2C_OK)
    {
        I2C_Stop();
        return I2C_TIMEOUT;
    }
    for(uint8_t i=0;i<(size - 1);i++)
    {
        rx_buff[i] = I2C_ReadByte(ACK);
    }
    rx_buff[size - 1] = I2C_ReadByte(NACK);
    I2C_Stop();
    return I2C_OK;
}


I2C_StateTypeDef SHT31_WR_Command(uint16_t command)
{
    uint8_t MSB = (uint8_t)((command & 0xFF00)>>8);
    uint8_t LSB = (uint8_t)(command & 0x00FF);
    I2C_Start();
    if(I2C_CallSlave(SHT31_ADD, 0) != I2C_OK)
    {
        I2C_Stop();
        return I2C_TIMEOUT;
    }
    if(I2C_SendByte(MSB) != I2C_OK)
    {
        I2C_Stop();
        return I2C_TIMEOUT;
    }
    if(I2C_SendByte(LSB) != I2C_OK)
    {
        I2C_Stop();
        return I2C_TIMEOUT;
    }
    I2C_Stop();
    return I2C_OK;
}


int8_t SHT31_Get_SR(uint8_t *buff)
{
    SHT31_RD(buff, 3);
#ifdef  CRC_SHT31
    if(_CRC_SHT31(&buff[0], 2) != buff[2])return -1;
#endif
    return 0;
}


void SHT31_Init()
{
    SHT31_Reset_Soft();
    delay_ms(2);
    SHT31_Clear_SR();
    SHT31_Heater_off();
}


int8_t SHT31_GetValue_TRH(uint8_t *buff, float *out_buff)
{
    if(SHT31_RD(buff, 6) != I2C_OK)return -1;
#ifdef CRC_SHT31
    if(_CRC_SHT31(&buff[0], 2) != buff[2])return -1;
    if(_CRC_SHT31(&buff[3], 2) != buff[5])return -1;
#endif
    uint16_t raw_t = ((uint16_t)buff[0]<<8) | buff[1];
    uint16_t raw_rh = ((uint16_t)buff[3]<<8) | buff[4];
    out_buff[0] = -45.0f + 175.0f*((float)raw_t/65535.0f);
    out_buff[1] = 100.0f*((float)raw_rh/65535.0f);
    return 0;
}
