#include    "Modbus_proto.h"
#include    "delay_tim2.h"
#include    "CRC16.h"
#include    "flash.h"


#ifdef  TCPIP
    #include    "W5500_Msp.h"
    #include    "socket.h"
#endif

    uint16_t count = 0;
const Firmware_Info_TypeDef       Firmware_Info_Default_Flash_Struct __attribute__((section(".ARM.__at_"STR(FIRM_WARE_ADD))))=
{
    .version = VERSION,
#ifdef  TCPIP
    .ip = {192,168,1,222},
    .sn = {255,255,255,0},
    .gw = {192,168,1,1},
    .dns = {8,8,8,8},
    .tcpip_port = TCPIP_PORT,
#endif
    .sensor_add = SENSOR_DEFAULT_ADD,
    .oled_brightness = 255,
    .Flash_CRC = 0x4834,
};

Firmware_Info_TypeDef Firmware_Info_Load_Struct = {0};
Firmware_Info_TypeDef Firmware_Info_Default_Ram_Struct = Firmware_Info_Default_Flash_Struct;

#ifdef  TCPIP
#define MBAP_HANDLE()   \
    do{                 \
        memcpy(TX_MBAP_PTR(modbus_ctl), RX_MBAP_PTR(modbus_ctl), 4);    \
        pack_16_msb(&(TX_MBAP_PTR(modbus_ctl)[4]), (uint16_t)modbus_ctl->send_len); \
    }while(0)
#else
#define MBAP_HANDLE()   \
    do{}while(0);
#endif


#ifdef  TCPIP
    #define ERROR_HANDLING_MBAP()    \
        do{     \
            if(modbus_ctl->recv_len>RX_BUFF_SIZE)   \
            {   \
                modbus_ctl->send_len = 0;   \
                return ERROR_FRAME;\
            }   \
        }while(0)
#else
    #define ERROR_HANDLING_MBAP()    \
        do{}while(0)
#endif
    
#define ERROR_HANDLING_02(x)   \
    do{     \
        if(start_add>(x) || (start_add + register_num>(x))) \
        {   \
            p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];   \
            p_tx[1] = p_rx[1] + 0x80;   \
            p_tx[2] = 0x02; \
            modbus_ctl->send_len = 3;   \
            MBAP_HANDLE();  \
            return ERROR_CODE_02;   \
        }   \
       }while(0)


#define ERROR_HANDLING_03(x)   \
    do{     \
            if(register_num==0 || register_num>(x))\
            {   \
                p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];   \
                p_tx[1] = p_rx[1] + 0x80;   \
                p_tx[2] = 0x03; \
                modbus_ctl->send_len = 3;   \
                MBAP_HANDLE();  \
                return ERROR_CODE_03;   \
            }       \
       }while(0)


#define ERROR_HANDLING_04()     \
    do{ \
            p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];  \
            p_tx[1] = p_rx[1] + 0x80;   \
            p_tx[2] = 0x04; \
            modbus_ctl->send_len = 3;   \
            MBAP_HANDLE();  \
            return ERROR_CODE_04;   \
    }while(0)
Modbus_Ctl_TypeDef  Modbus_Ctl;
Modbus_Register_Struct_TypeDef      Modbus_Register_Struct =
       {
           .AI = {200, 800, 560},
           .AO = {[TEMP_SET] = 200, [HUMI_SET] = 650,},
           .DI = {0,1,0,1,0,1,0,1},
           .DO = {1,0,1,0,1,0,1,0},
       };

typedef int8_t(*Funk_OP)(Modbus_Ctl_TypeDef *modbus_ctl);


int8_t Funk_01(Modbus_Ctl_TypeDef *modbus_ctl);
int8_t Funk_02(Modbus_Ctl_TypeDef *modbus_ctl);
int8_t Funk_03(Modbus_Ctl_TypeDef *modbus_ctl);
int8_t Funk_04(Modbus_Ctl_TypeDef *modbus_ctl);
int8_t Funk_05(Modbus_Ctl_TypeDef *modbus_ctl);
int8_t Funk_06(Modbus_Ctl_TypeDef *modbus_ctl);
int8_t Funk_15(Modbus_Ctl_TypeDef *modbus_ctl);
int8_t Funk_16(Modbus_Ctl_TypeDef *modbus_ctl);


Funk_OP Funk_OP_List[17] = 
{
    [0x01] = Funk_01,
    [0x02] = Funk_02,
    [0x03] = Funk_03,
    [0x04] = Funk_04,
    [0x05] = Funk_05,
    [0x06] = Funk_06,
    [0x0F] = Funk_15,
    [0x10] = Funk_16,
};


static inline uint16_t unpack_16_msb(uint8_t *buff)
{
    return (buff[0]<<8)|buff[1];
}


static inline void pack_16_msb(uint8_t *buff, uint16_t data)
{
    buff[0] = (uint8_t)(data>>8);
    buff[1] = (uint8_t)data;
}


static inline uint8_t pack_bit(uint16_t *buff, uint8_t num)
{
    uint8_t result = 0;
    for(uint8_t i=0;i<num;i++)
    {
        result <<= 1;
        result |= buff[num - i - 1];
    }
    return result;
}


int8_t Modbus_Flash_Erase()
{
    uint8_t count = 0;
    while(Flash_Erase((uint32_t)FIRM_WARE_ADD) == HAL_ERROR)
    {
        count ++;
        if(count >= 3)
        {
            return -1;
        }
    }
    return 0;
}


int8_t Modbus_Flash_Write(uint32_t add, uint8_t *data, uint32_t size)
{
    uint8_t count = 0;
    while(Flash_Write(add, data, size) == HAL_ERROR)
    {
        count ++;
        if(count >= 3)
        {
            return -1;
        }
    }
    return 0;
}


Firmware_Info_TypeDef* Find_Firmware_Info(Firmware_Info_TypeDef *p_find, uint16_t *count)
{
    uint16_t record_count = 0;
    while(*(uint32_t*)(p_find + 1) != 0xFFFFFFFF)
    {
        p_find ++;
        record_count ++;
        if(record_count > MAX_CONFIG_RECORDS)
        {
            *count = record_count;
            return NULL;
        }
    }
    *count = record_count;
    return p_find;
}


void Modbus_AO_Register_Load(void)
{
    Firmware_Info_TypeDef *p_read = &Firmware_Info_Load_Struct;
    Modbus_Register_Struct_TypeDef *p_write = &Modbus_Register_Struct;
#ifdef  TCPIP
    p_write->AO[AO_IP_ADD_LO] = (((p_read->ip[1])<<8) | (p_read->ip[0]));
    p_write->AO[AO_IP_ADD_HI] = (((p_read->ip[3])<<8) | (p_read->ip[2]));
    p_write->AO[AO_SUBNET_MASK_LO] = (((p_read->sn[1])<<8) | (p_read->sn[0]));
    p_write->AO[AO_SUBNET_MASK_HI] = (((p_read->sn[3])<<8) | (p_read->sn[2]));
    p_write->AO[AO_GATEWAY_LO] = (((p_read->gw[1])<<8) | (p_read->gw[0]));
    p_write->AO[AO_GATEWAY_HI] = (((p_read->gw[3])<<8) | (p_read->gw[2]));
    p_write->AO[AO_DNS_LO] = (((p_read->dns[1])<<8) | (p_read->dns[0]));
    p_write->AO[AO_DNS_HI] = (((p_read->dns[3])<<8) | (p_read->dns[2]));
    p_write->AO[AO_TCP_PORT] = p_read->tcpip_port;
#endif
    p_write->AO[AO_SENSOR_ADD] = p_read->sensor_add;
    p_write->AO[AO_TEM_OFFSET] = p_read->tem_offset;
    p_write->AO[AO_HUMI_OFFSET] = p_read->humi_offset;
    p_write->AO[AO_CO2_ALTITUDE_COMP] = p_read->CO2_altitude_comp;
    p_write->AO[AO_CO2_ABC_ENABLE] = p_read->CO2_abc_enable;
    p_write->AO[AO_CO2_FRC_REF_VAL] = p_read->CO2_frc_ref_val;
    p_write->AO[AO_TEM_ALARM_HI] = p_read->tem_alarm_hi;
    p_write->AO[AO_TEM_ALARM_LO] = p_read->tem_alarm_lo;
    p_write->AO[AO_TEM_ALARM_HYSTERESIS] = p_read->tem_alarm_hysteresis;
    p_write->AO[AO_HUMI_ALARM_HI] = p_read->humi_alarm_hi;
    p_write->AO[AO_HUMI_ALARM_LO] = p_read->humi_alarm_lo;
    p_write->AO[AO_HUMI_ALARM_HYSTERESIS] = p_read->humi_alarm_hysteresis;
    p_write->AO[AO_CO2_ALARM_HI] = p_read->CO2_alarm_hi;
    p_write->AO[AO_CO2_ALARM_LO] = p_read->CO2_alarm_lo;
    p_write->AO[AO_CO2_ALARM_HYSTERESIS] = p_read->CO2_alarm_hysteresis;
    p_write->AO[AO_OLED_BRIGHTNESS] = p_read->oled_brightness;
}


void Modbus_Firmware_Reload(void)
{
    Firmware_Info_TypeDef *p_write = &Firmware_Info_Load_Struct;
    Modbus_Register_Struct_TypeDef *p_read = &Modbus_Register_Struct;
    p_write->version = VERSION;
    p_write->sensor_add = p_read->AO[AO_SENSOR_ADD];
#ifdef  TCPIP
    p_write->ip[0] = (uint8_t)((p_read->AO[AO_IP_ADD_LO])&0xFF);
    p_write->ip[1] = (uint8_t)(((p_read->AO[AO_IP_ADD_LO])&0xFF00)>>8);
    p_write->ip[2] = (uint8_t)((p_read->AO[AO_IP_ADD_HI])&0xFF);
    p_write->ip[3] = (uint8_t)(((p_read->AO[AO_IP_ADD_HI])&0xFF00)>>8);
    p_write->sn[0] = (uint8_t)((p_read->AO[AO_SUBNET_MASK_LO])&0xFF);
    p_write->sn[1] = (uint8_t)(((p_read->AO[AO_SUBNET_MASK_LO])&0xFF00)>>8);
    p_write->sn[2] = (uint8_t)((p_read->AO[AO_SUBNET_MASK_HI])&0xFF);
    p_write->sn[3] = (uint8_t)(((p_read->AO[AO_SUBNET_MASK_HI])&0xFF00)>>8);
    p_write->gw[0] = (uint8_t)((p_read->AO[AO_GATEWAY_LO])&0xFF);
    p_write->gw[1] = (uint8_t)(((p_read->AO[AO_GATEWAY_LO])&0xFF00)>>8);
    p_write->gw[2] = (uint8_t)((p_read->AO[AO_GATEWAY_HI])&0xFF);
    p_write->gw[3] = (uint8_t)(((p_read->AO[AO_GATEWAY_HI])&0xFF00)>>8);
    p_write->dns[0] = (uint8_t)((p_read->AO[AO_DNS_LO])&0xFF);
    p_write->dns[1] = (uint8_t)(((p_read->AO[AO_DNS_LO])&0xFF00)>>8);
    p_write->dns[2] = (uint8_t)((p_read->AO[AO_DNS_HI])&0xFF);
    p_write->dns[3] = (uint8_t)(((p_read->AO[AO_DNS_HI])&0xFF00)>>8);
    p_write->tcpip_port = p_read->AO[AO_TCP_PORT];
#endif
    p_write->tem_offset = p_read->AO[AO_TEM_OFFSET];
    p_write->tem_alarm_hi = p_read->AO[AO_TEM_ALARM_HI];
    p_write->tem_alarm_lo = p_read->AO[AO_TEM_ALARM_LO];
    p_write->tem_alarm_hysteresis = p_read->AO[AO_TEM_ALARM_HYSTERESIS];
    p_write->humi_offset = p_read->AO[AO_HUMI_OFFSET];
    p_write->humi_alarm_hi = p_read->AO[AO_HUMI_ALARM_HI];
    p_write->humi_alarm_lo = p_read->AO[AO_HUMI_ALARM_LO];
    p_write->humi_alarm_hysteresis = p_read->AO[AO_HUMI_ALARM_HYSTERESIS];
    p_write->CO2_altitude_comp = p_read->AO[AO_CO2_ALTITUDE_COMP];
    p_write->CO2_abc_enable = p_read->AO[AO_CO2_ABC_ENABLE];
    p_write->CO2_frc_ref_val = p_read->AO[AO_CO2_FRC_REF_VAL];
    p_write->CO2_alarm_hi = p_read->AO[AO_CO2_ALARM_HI];
    p_write->CO2_alarm_lo = p_read->AO[AO_CO2_ALARM_LO];
    p_write->CO2_alarm_hysteresis = p_read->AO[AO_CO2_ALARM_HYSTERESIS];
    p_write->oled_brightness = p_read->AO[AO_OLED_BRIGHTNESS];
    p_write->reserved = 0;
#ifdef  TCPIP
    uint16_t FLASH_CRC = CRC16((uint16_t*)&Firmware_Info_Load_Struct, 27);
    p_write->Flash_CRC = FLASH_CRC;
#else
    uint16_t FLASH_CRC = CRC16((uint16_t*)&Firmware_Info_Load_Struct, 18);
    p_write->Flash_CRC = FLASH_CRC;
#endif
}


int8_t Modbus_Init(void)
{
//    uint16_t count = 0;
    Firmware_Info_TypeDef *p_find = Find_Firmware_Info((Firmware_Info_TypeDef*)FIRM_WARE_ADD, &count);
    if(p_find == NULL)
    {
        Modbus_Register_Struct.AI[AI_SYS_STATUS] |= (ERR_FLASH_CFG_OOB | STS_CFG_FALLBACK_DEFAULT);
        __disable_irq();
        if(Modbus_Flash_Erase() == -1)
        {
            Modbus_Register_Struct.AI[AI_SYS_STATUS] |= ERR_FLASH_PROGRAM_FAIL;
            memcpy((void*)&Firmware_Info_Load_Struct, (void*)&Firmware_Info_Default_Ram_Struct, sizeof(Firmware_Info_TypeDef));
            __enable_irq();
            return -1;
        }
        if(Modbus_Flash_Write((uint32_t)FIRM_WARE_ADD, (uint8_t*)&Firmware_Info_Default_Ram_Struct, sizeof(Firmware_Info_TypeDef)) == -1)
        {
            Modbus_Register_Struct.AI[AI_SYS_STATUS] |= ERR_FLASH_PROGRAM_FAIL;
            memcpy((void*)&Firmware_Info_Load_Struct, (void*)&Firmware_Info_Default_Ram_Struct, sizeof(Firmware_Info_TypeDef));
            __enable_irq();
            return -1;
        }
        __enable_irq();
        p_find = (Firmware_Info_TypeDef*)FIRM_WARE_ADD;
    }
    if(CRC16((uint16_t*)p_find, (sizeof(Firmware_Info_TypeDef)/2)) != 0)
    {
        Modbus_Register_Struct.AI[AI_SYS_STATUS] |= (ERR_FLASH_CRC_MISMATCH | ERR_FLASH_PROGRAM_FAIL);
        memcpy((void*)&Firmware_Info_Load_Struct, (void*)&Firmware_Info_Default_Ram_Struct, sizeof(Firmware_Info_TypeDef));
    }
    else
    {
        memcpy((void*)&Firmware_Info_Load_Struct, (void*)p_find, sizeof(Firmware_Info_TypeDef));
    }
    Modbus_AO_Register_Load();
#ifdef  TCPIP
    W5500_Init(Firmware_Info_Load_Struct.ip, Firmware_Info_Load_Struct.sn, Firmware_Info_Load_Struct.gw, Firmware_Info_Load_Struct.dns);
#endif
    return 0;
}


void Modbus_Process(Modbus_Ctl_TypeDef *modbus_ctl)
{
    uint8_t *p_rx = RX_PDU_PTR(modbus_ctl);
    uint8_t *p_tx = TX_PDU_PTR(modbus_ctl);    
    uint8_t fc = p_rx[1];
    if(fc > 16 || Funk_OP_List[fc] == NULL)
    {
        p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
        p_tx[1] = fc + 0x80;
        p_tx[2] = 0x01;
        modbus_ctl->send_len = 3;
        MBAP_HANDLE();
        return;
    }
    Funk_OP_List[fc](modbus_ctl);
}


int8_t Modbus_Recv(uint8_t sock, Modbus_Ctl_TypeDef *modbus_ctl, uint16_t timeout)
{
//    uint32_t Tick = HAL_GetTick();
#ifdef  TCPIP
    uint16_t recv_len = 0;
    while(recv_len<6)
    {
        if(getSn_RX_RSR(sock)>0)
        {
            recv_len += recv(sock, &(RX_MBAP_PTR(modbus_ctl)[recv_len]), 6-recv_len);
        }
//        if((HAL_GetTick()-Tick)>timeout)return TIMEOUT;
    }
    
    recv_len = 0;
    uint16_t len_pdu_payload = unpack_16_msb(&(RX_MBAP_PTR(modbus_ctl)[4]));
    if(len_pdu_payload > RX_BUFF_SIZE)
    {
        uint16_t remain = getSn_RX_RSR(sock);
        while(remain>0)
        {
            remain -= recv(sock, RX_PDU_PTR(modbus_ctl), RX_BUFF_SIZE);
        }
        modbus_ctl->recv_len = len_pdu_payload;
        return DATAERROR;
    }
    
    while(recv_len<len_pdu_payload)
    {
        if(getSn_RX_RSR(sock)>0)
        {
            recv_len += recv(sock, &(RX_PDU_PTR(modbus_ctl)[recv_len]), len_pdu_payload-recv_len);
        }
//        if((HAL_GetTick()-Tick)>timeout)return TIMEOUT;
    }
    modbus_ctl->recv_len = len_pdu_payload;
#endif
    return 0;
}


int8_t Modbus_Send(uint8_t sock, Modbus_Ctl_TypeDef *modbus_ctl)
{
#ifdef  TCPIP
    if(modbus_ctl->send_len == 0)
    {
        return 0;
    }
    else
    {
        while(1)
        {
            if(getSn_TX_FSR(sock) >= modbus_ctl->send_len)
            return send(sock, TX_MBAP_PTR(modbus_ctl), (modbus_ctl->send_len) + 6);
            delay_us(500);
        }
    }
#endif
}


/* Read Multiple DO */
int8_t Funk_01(Modbus_Ctl_TypeDef *modbus_ctl)
{
    ERROR_HANDLING_MBAP();
    uint8_t *p_rx = RX_PDU_PTR(modbus_ctl);
    uint8_t *p_tx = TX_PDU_PTR(modbus_ctl);
#ifdef  TCPIP
    if(modbus_ctl->recv_len != 6)
    {
        modbus_ctl->send_len = 0;
        return ERROR_FRAME;
    }
#endif
    uint16_t start_add = unpack_16_msb(&p_rx[2]);
    uint16_t register_num = unpack_16_msb(&p_rx[4]);
    uint8_t byte_num = (register_num + 7) / 8;
    ERROR_HANDLING_02(DO_SIZE);
    ERROR_HANDLING_03(DO_SIZE);
    p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
    p_tx[1] = p_rx[1];
    p_tx[2] = byte_num;
    for(uint8_t i=0;i<(register_num/8);i++)
    {
        p_tx[3 + i] = pack_bit(&(Modbus_Register_Struct.DO[start_add + 8*i]), 8);
    }
    if(register_num%8)p_tx[byte_num + 2] = pack_bit(&(Modbus_Register_Struct.DO[start_add + 8*(byte_num -1)]), register_num%8);
    modbus_ctl->send_len = byte_num + 3;
    MBAP_HANDLE();
    return 0;
}


/* Read Multiple DI */
int8_t Funk_02(Modbus_Ctl_TypeDef *modbus_ctl)
{
    ERROR_HANDLING_MBAP();
    uint8_t *p_rx = RX_PDU_PTR(modbus_ctl);
    uint8_t *p_tx = TX_PDU_PTR(modbus_ctl);
#ifdef  TCPIP
    if(modbus_ctl->recv_len != 6)
    {
        modbus_ctl->send_len = 0;
        return ERROR_FRAME;
    }
#endif
    uint16_t start_add = unpack_16_msb(&p_rx[2]);
    uint16_t register_num = unpack_16_msb(&p_rx[4]);
    uint8_t byte_num = (register_num + 7)/8;
    ERROR_HANDLING_02(DI_SIZE);
    ERROR_HANDLING_03(DI_SIZE);
    p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
    p_tx[1] = p_rx[1];
    p_tx[2] = byte_num;
    for(uint8_t i=0;i<(register_num/8);i++)
    {
        p_tx[3 + i] = pack_bit(&(Modbus_Register_Struct.DI[start_add + 8*i]), 8);
    }
    if(register_num%8)p_tx[byte_num + 2] = pack_bit(&(Modbus_Register_Struct.DI[start_add + 8*(byte_num - 1)]), register_num%8);
    modbus_ctl->send_len = byte_num + 3;
    MBAP_HANDLE();
    return 0;
}


/* Read Multiple AO */
int8_t Funk_03(Modbus_Ctl_TypeDef *modbus_ctl)
{
    ERROR_HANDLING_MBAP();
    uint8_t *p_rx = RX_PDU_PTR(modbus_ctl);
    uint8_t *p_tx = TX_PDU_PTR(modbus_ctl);
#ifdef  TCPIP
    if(modbus_ctl->recv_len != 6)
    {
        modbus_ctl->send_len = 0;
        return ERROR_FRAME;
    }
#endif
    uint16_t start_add = unpack_16_msb(&p_rx[2]);
    uint16_t register_num = unpack_16_msb(&p_rx[4]);
    uint8_t byte_num = register_num * 2;
    ERROR_HANDLING_02(AO_SIZE);
    /* modbus 03功能码最大允许读取125个AO寄存器 */
    uint16_t max_read_limit = (AO_SIZE < 125)?AO_SIZE:125;
    ERROR_HANDLING_03(max_read_limit);
    p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
    p_tx[1] = p_rx[1];
    p_tx[2] = byte_num;
    for(uint16_t i=0;i<register_num;i++)
    {
        pack_16_msb(&p_tx[3 + i*2], Modbus_Register_Struct.AO[start_add + i]);
    }
    modbus_ctl->send_len = byte_num + 3;
    MBAP_HANDLE();
    return 0;
}


/* Read Multiple AI */
int8_t Funk_04(Modbus_Ctl_TypeDef *modbus_ctl)
{
    ERROR_HANDLING_MBAP();
    uint8_t *p_rx = RX_PDU_PTR(modbus_ctl);
    uint8_t *p_tx = TX_PDU_PTR(modbus_ctl);
#ifdef  TCPIP
    if(modbus_ctl->recv_len != 6)
    {
        modbus_ctl->send_len = 0;
        return ERROR_FRAME;
    }
#endif
    uint16_t start_add = unpack_16_msb(&p_rx[2]);
    uint16_t register_num = unpack_16_msb(&p_rx[4]);
    uint8_t byte_num = register_num * 2;
    ERROR_HANDLING_02(AI_SIZE);
    /* modbus 03功能码最大允许读取125个AO寄存器 */
    uint16_t max_read_limit = (AO_SIZE < 125)?AO_SIZE:125;
    ERROR_HANDLING_03(max_read_limit);
    p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
    p_tx[1] = p_rx[1];
    p_tx[2] = byte_num;
    for(uint16_t i=0;i<register_num;i++)
    {
        pack_16_msb(&p_tx[3 + i*2], Modbus_Register_Struct.AI[start_add + i]);
    }
    modbus_ctl->send_len = byte_num + 3;
    MBAP_HANDLE();
    return 0;
}


/* Write Single DO */
int8_t Funk_05(Modbus_Ctl_TypeDef *modbus_ctl)
{
    ERROR_HANDLING_MBAP();
    uint8_t *p_rx = RX_PDU_PTR(modbus_ctl);
    uint8_t *p_tx = TX_PDU_PTR(modbus_ctl);
#ifdef  TCPIP
    if(modbus_ctl->recv_len != 6)
    {
        modbus_ctl->send_len = 0;
        return ERROR_FRAME;
    }
#endif
    uint16_t start_add = unpack_16_msb(&p_rx[2]);
    uint16_t val = unpack_16_msb(&p_rx[4]);
    uint16_t register_num = 1;
    ERROR_HANDLING_02(DO_SIZE);
    if(val != 0xFF00 && val != 0x0000)
    {
        p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
        p_tx[1] = 0x85;
        p_tx[2] = 0x03;
        modbus_ctl->send_len = 3;
        MBAP_HANDLE();
        return ERROR_CODE_03;
    }
    /* 可以替换成专用函数处理 */
    if(val)Modbus_Register_Struct.DO[start_add] = 1;
    else Modbus_Register_Struct.DO[start_add] = 0;
    /*------------------------------------------*/
    p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
    memcpy(&p_tx[1], &p_rx[1], 5);
    modbus_ctl->send_len = 6;
    MBAP_HANDLE();
    return 0;
}


/* Write Single AO */
int8_t Funk_06(Modbus_Ctl_TypeDef *modbus_ctl)
{
    ERROR_HANDLING_MBAP();
    uint8_t *p_rx = RX_PDU_PTR(modbus_ctl);
    uint8_t *p_tx = TX_PDU_PTR(modbus_ctl);
#ifdef  TCPIP
    if(modbus_ctl->recv_len != 6)
    {
        modbus_ctl->send_len = 0;
        return ERROR_FRAME;
    }
#endif
    uint16_t start_add = unpack_16_msb(&p_rx[2]);
    uint16_t val = unpack_16_msb(&p_rx[4]);
    uint16_t register_num = 1;
    ERROR_HANDLING_02(AO_SIZE);
    /* 可以替换成专用函数处理 */
    Modbus_Register_Struct.AO[start_add] = val;
    /* -------------------------- */
    
    
    if(Modbus_Register_Struct.AO[AO_SYS_COMMAND] == CMD_SAVE_AND_REBOOT && \
       Modbus_Register_Struct.AI[AI_SYS_STATUS] & ERR_FLASH_PROGRAM_FAIL)
    {
        ERROR_HANDLING_04();
    }
    if(Modbus_Register_Struct.AO[AO_SYS_COMMAND] == CMD_FACTORY_RESET && \
       Modbus_Register_Struct.AI[AI_SYS_STATUS] & ERR_FLASH_PROGRAM_FAIL)
    {
        ERROR_HANDLING_04();
    }
    
    
    p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
    memcpy(&p_tx[1], &p_rx[1], 5);
    modbus_ctl->send_len = 6;
    MBAP_HANDLE();
    return 0;
}


/* Write Multiple DO */
int8_t Funk_15(Modbus_Ctl_TypeDef *modbus_ctl)
{
    ERROR_HANDLING_MBAP();
    uint8_t *p_rx = RX_PDU_PTR(modbus_ctl);
    uint8_t *p_tx = TX_PDU_PTR(modbus_ctl);
    uint16_t start_add = unpack_16_msb(&p_rx[2]);
    uint16_t register_num = unpack_16_msb(&p_rx[4]);
    uint8_t byte_num = p_rx[6];
#ifdef  TCPIP
    if(modbus_ctl->recv_len != (7 + byte_num) || byte_num != (register_num + 7)/8)
    {
        modbus_ctl->send_len = 0;
        return ERROR_FRAME;
    }
#endif
    ERROR_HANDLING_02(DO_SIZE);
    ERROR_HANDLING_03(DO_SIZE);
    /* 可以替换成专用函数处理 */
    for(uint16_t i=0;i<(register_num/8);i++)
    {
        uint8_t val = p_rx[7 + i];
        for(uint8_t j=0;j<8;j++)
        {
            if(val&0x01)Modbus_Register_Struct.DO[start_add + 8*i + j] = 1;
            else Modbus_Register_Struct.DO[start_add + 8*i + j] = 0;
            val >>= 1;
        }
    }
    if(register_num % 8)
    {
        for(uint8_t i=0;i<(register_num % 8);i++)
        {
            uint8_t val = p_rx[7 + (register_num/8)];
            for(uint8_t j=0;j<(register_num % 8);j++)
            {
                if(val & 0x01)Modbus_Register_Struct.DO[start_add + (register_num/8)*8 + i] = 1;
                else Modbus_Register_Struct.DO[start_add + (register_num/8)*8 + i] = 0;
                val >>= 1;
            }
        }
    }
    /* -------------------------- */
    p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
    memcpy(&p_tx[1], &p_rx[1], 5);
    modbus_ctl->send_len = 6;
    MBAP_HANDLE();
    return 0;
}


/* Write Multiple AO */
int8_t Funk_16(Modbus_Ctl_TypeDef *modbus_ctl)
{
    ERROR_HANDLING_MBAP();
    uint8_t *p_rx = RX_PDU_PTR(modbus_ctl);
    uint8_t *p_tx = TX_PDU_PTR(modbus_ctl);
    uint16_t start_add = unpack_16_msb(&p_rx[2]);
    uint16_t register_num = unpack_16_msb(&p_rx[4]);
    uint8_t byte_num = p_rx[6];
#ifdef  TCPIP
    if(modbus_ctl->recv_len != (7 + byte_num) || byte_num != 2 * register_num)
    {
        modbus_ctl->send_len = 0;
        return ERROR_FRAME;
    }
#endif
    ERROR_HANDLING_02(AO_SIZE);
    uint16_t max_read_limit = (AO_SIZE < 123)?AO_SIZE:123;
    ERROR_HANDLING_03(max_read_limit);
    /* 可以替换成专用函数处理 */
    for(uint16_t i=0;i<register_num;i++)
    {
        Modbus_Register_Struct.AO[start_add + i] = unpack_16_msb(&p_rx[7 + i*2]);
    }
    
    /* -------------------------- */
    if(Modbus_Register_Struct.AO[AO_SYS_COMMAND] == CMD_SAVE_AND_REBOOT && \
       Modbus_Register_Struct.AI[AI_SYS_STATUS] & ERR_FLASH_PROGRAM_FAIL)
    {
        ERROR_HANDLING_04();
    }
    if(Modbus_Register_Struct.AO[AO_SYS_COMMAND] == CMD_FACTORY_RESET && \
       Modbus_Register_Struct.AI[AI_SYS_STATUS] & ERR_FLASH_PROGRAM_FAIL)
    {
        ERROR_HANDLING_04();
    }
    
    
    p_tx[0] = Modbus_Register_Struct.AO[AO_SENSOR_ADD];
    memcpy(&p_tx[1], &p_rx[1], 5);
    modbus_ctl->send_len = 6;
    MBAP_HANDLE();
    return 0;
}