#ifndef     _MODBUS_PROTO
#define     _MODBUS_PROTO
#include    "main.h"


#define     TCPIP
#define     TCPIP_PORT  502


#define     VERSION     1
#define     SENSOR_DEFAULT_ADD  0xFF
#define     FIRM_WARE_ADD       0x08060000
#define     _STR(x)     #x
#define     STR(x)      _STR(x)
#define     MAX_CONFIG_RECORDS  1999


#define     DO_SIZE     8
#define     DI_SIZE     8
#define     AI_SIZE     64
#define     AI_TEM      0
#define     AI_HUMI     1
#define     AI_CO2      2
#define     AI_SYS_STATUS   0x3F
#define     AO_SIZE     64
#define     AO_IP_ADD_LO        0x00
#define     AO_IP_ADD_HI        0x01
#define     AO_SUBNET_MASK_LO   0x02
#define     AO_SUBNET_MASK_HI   0x03
#define     AO_GATEWAY_LO       0x04
#define     AO_GATEWAY_HI       0x05
#define     AO_DNS_LO           0x06
#define     AO_DNS_HI           0x07
#define     AO_TCP_PORT         0x08
#define     AO_SENSOR_ADD       0x09
#define     AO_TEM_OFFSET       0x10
#define     AO_HUMI_OFFSET      0x11
#define     AO_CO2_ALTITUDE_COMP    0x12
#define     AO_CO2_ABC_ENABLE       0x13
#define     AO_CO2_FRC_REF_VAL      0x14
#define     AO_TEM_ALARM_HI         0x20
#define     AO_TEM_ALARM_LO         0x21
#define     AO_TEM_ALARM_HYSTERESIS 0x22
#define     AO_HUMI_ALARM_HI        0x23
#define     AO_HUMI_ALARM_LO        0x24
#define     AO_HUMI_ALARM_HYSTERESIS    0x25
#define     AO_CO2_ALARM_HI         0x26
#define     AO_CO2_ALARM_LO         0x27
#define     AO_CO2_ALARM_HYSTERESIS 0x28
#define     AO_BUZZER_ENABLE        0x29
#define     AO_OLED_BRIGHTNESS      0x30
#define     AO_RELAY_MANUAL_CTRL    0x31
#define     AO_SYS_COMMAND          0x3F


#define     ERR_FLASH_CFG_CORRUPT    (1U << 0)
#define     ERR_FLASH_CFG_OOB        (1U << 1)
#define     ERR_FLASH_CRC_MISMATCH   (1U << 2)
#define     STS_CFG_FALLBACK_DEFAULT (1U << 3)
#define     ERR_FLASH_PROGRAM_FAIL   (1U << 4)
#define     ERR_SYS_IWDG_RESET       (1U << 5)
#define     ERR_SYS_BOR_RESET        (1U << 6)
#define     ERR_SHT31_COMM_TIMEOUT   (1U << 8)
#define     ERR_SHT31_CRC_FAIL       (1U << 9)
#define     ERR_SCD40_COMM_TIMEOUT   (1U << 10)
#define     ERR_SCD40_CRC_FAIL       (1U << 11)
#define     ERR_I2C_BUS_BUSY         (1U << 12)

#define     SYS_LATCHED_ERR_MASK     (ERR_FLASH_CFG_CORRUPT | \
                                     ERR_FLASH_CFG_OOB     | \
                                     STS_CFG_FALLBACK_DEFAULT | \
                                     ERR_SYS_IWDG_RESET)


#define     CMD_CLEAR_LATCHED_FAULTS        0x0001U
#define     CMD_SAVE_AND_REBOOT             0x55AAU
#define     CMD_FACTORY_RESET               0xAA55U

#define     TEMP_SET    8
/* humidity */
#define     HUMI_SET    9
#define     MBAP_HEAD       6
#define     RX_BUFF_SIZE    128
#define     TX_BUFF_SIZE    128


#define     ERROR_FRAME     -0x01
#define     ERROR_CODE_01   0x01
#define     ERROR_CODE_02   0x02
#define     ERROR_CODE_03   0x03
#define     ERROR_CODE_04   0x04
#define     ERROR_CODE_05   0x05
#define     ERROR_CODE_06   0x06
#define     ERROR_CODE_07   0x07
#define     ERROR_CODE_08   0x08

#define     TIMEOUT         0x01
#define     DATAERROR       0x02

#ifdef      TCPIP
    #define     RX_MBAP_PTR(ctl) ((ctl)->rx_buff)
    #define     TX_MBAP_PTR(ctl) ((ctl)->tx_buff)
    
    #define     RX_PDU_PTR(ctl) ((ctl)->rx_buff + MBAP_HEAD)
    #define     TX_PDU_PTR(ctl) ((ctl)->tx_buff + MBAP_HEAD)
#else
    #define     RX_PDU_PTR(ctl) ((ctl)->rx_buff)
    #define     TX_PDU_PTR(ctl) ((ctl)->tx_buff)
#endif
typedef struct
{
    uint16_t DO[DO_SIZE];
    uint16_t DI[DI_SIZE];
    uint16_t AI[AI_SIZE];
    uint16_t AO[AO_SIZE];
}Modbus_Register_Struct_TypeDef;


typedef struct
{
#ifdef  TCPIP
    uint8_t rx_buff[RX_BUFF_SIZE + MBAP_HEAD];
    uint8_t tx_buff[TX_BUFF_SIZE + MBAP_HEAD];
#else
    uint8_t rx_buff[RX_BUFF_SIZE];
    uint8_t tx_buff[TX_BUFF_SIZE];
#endif
    uint32_t recv_len;
    uint32_t send_len;
}Modbus_Ctl_TypeDef;


typedef struct
{
    uint32_t Error;
}Modbus_Error_TypeDef;


#pragma pack(1)
typedef struct _Firmware_Info_TypeDef
{
    uint16_t version;
    uint16_t sensor_add;
#ifdef  TCPIP
    uint8_t ip[4];
    uint8_t sn[4];
    uint8_t gw[4];
    uint8_t dns[4];
    uint16_t tcpip_port;
#endif
    int16_t tem_offset;
    int16_t tem_alarm_hi;
    int16_t tem_alarm_lo;
    uint16_t tem_alarm_hysteresis;
    int16_t humi_offset;
    uint16_t humi_alarm_hi;
    uint16_t humi_alarm_lo;
    uint16_t humi_alarm_hysteresis;
    int16_t CO2_altitude_comp;
    uint16_t CO2_abc_enable;
    uint16_t CO2_frc_ref_val;
    uint16_t CO2_alarm_hi;
    uint16_t CO2_alarm_lo;
    uint16_t CO2_alarm_hysteresis;
    uint16_t oled_brightness;
    uint16_t reserved;
    uint16_t Flash_CRC;
}Firmware_Info_TypeDef;
#pragma pack()


extern Modbus_Ctl_TypeDef  Modbus_Ctl;
extern Modbus_Register_Struct_TypeDef      Modbus_Register_Struct;
extern Firmware_Info_TypeDef Firmware_Info_Load_Struct;
extern Firmware_Info_TypeDef Firmware_Info_Default_Ram_Struct;


int8_t Modbus_Flash_Erase();
int8_t Modbus_Flash_Write(uint32_t add, uint8_t *data, uint32_t size);
Firmware_Info_TypeDef* Find_Firmware_Info(Firmware_Info_TypeDef *p_find, uint16_t *count);
void Modbus_Firmware_Reload(void);
int8_t Modbus_Init(void);
void Modbus_Process(Modbus_Ctl_TypeDef *modbus_ctl);
int8_t Modbus_Recv(uint8_t sock, Modbus_Ctl_TypeDef *modbus_ctl, uint16_t timeout);
int8_t Modbus_Send(uint8_t sock, Modbus_Ctl_TypeDef *modbus_ctl);
#endif
