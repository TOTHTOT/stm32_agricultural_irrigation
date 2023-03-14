/*
 * @Description: dht11 驱动文件
 * @Author: TOTHTOT
 * @Date: 2023-03-12 19:34:07
 * @LastEditTime: 2023-03-12 20:18:10
 * @LastEditors: TOTHTOT
 * @FilePath: \MDK-ARMe:\JieDan\stm32_agricultural_irrigation\CODE\STM32F103C8T6(HAL+FreeRTOS)\HARDWARE\DHT11\dht11.c
 */

/* 调用头文件 */
#include "dht11.h"
#include "delay.h"

/* 内部调用函数 */
void dht11_io_in(void);
void dht11_io_out(void);
void dht11_reset(void);
uint8_t dht11_read_bit(void);
uint8_t dht11_read_byte(void);
uint8_t dht11_check(struct dht11_device *device);

/* 内部宏定义 */
#define DHT11_INPUT_DQ HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin)
#define DHT11_OUTPUT_H HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET)
#define DHT11_OUTPUT_L HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_RESET)

/* 全局变量定义 */
struct dht11_device g_dht11_device_st = {
    .dht11_data_humidity = 0,
    .dht11_data_temprature = 0,
    .dht11_state_em = DHT11_NOT_FIND,
    .get_dat11_data = NULL
};    

/**
 * @name: dht11_io_in
 * @msg: IO口方向设置为输入
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年3月12日19:48:31
 */
void dht11_io_in(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin = DHT11_DATA_Pin;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStructure);
}

/**
 * @name: dht11_io_out
 * @msg: IO口方向设置为输出
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年3月12日19:48:31
 */
void dht11_io_out(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin = DHT11_DATA_Pin;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStructure);
}

/**
 * @name: dht11_reset
 * @msg: 复位 DHT11
 * @return {*} 无
 * @author: TOTHTOT
 * @date: 2023年3月12日19:54:08
 */
void dht11_reset(void)
{
    dht11_io_out(); // 设置为输出
    DHT11_OUTPUT_L; // 拉低
    delay_us(20);   // 至少18ms
    DHT11_OUTPUT_H; // 拉高
    delay_us(30);   // 至少20~40us
}

/**
 * @name: dht11_check
 * @msg: 检测dht11是否在线
 * @param {dht11_device} *device
 * @return {*} == 0 在线; == 1 不在线;
 * @author: TOTHTOT
 * @date:
 */
uint8_t dht11_check(struct dht11_device *device)
{
    uint8_t retry = 0;
    dht11_io_in();
    while (DHT11_INPUT_DQ && retry < DHT11_RETRY_TIMES)
    { // 拉低40~80us
        retry++;
        delay_us(1);
    };

    if (retry >= DHT11_RETRY_TIMES)
    {
        device->dht11_state_em = DHT11_OFFLINE;
        return 1;
    }

    retry = 0;
    while (!DHT11_INPUT_DQ && retry < DHT11_RETRY_TIMES)
    { // 拉高40~80us
        retry++;
        delay_us(1);
    };
    if (retry >= DHT11_RETRY_TIMES)
    {
        device->dht11_state_em = DHT11_OFFLINE;
        return 1;
    }
    device->dht11_state_em = DHT11_ONLINE;
    return 0; // 检测到DHT11返回0
}

/**
 * @name: dht11_read_bit
 * @msg: dht11 读取一位
 * @return {*}  读取到的数据
 * @author: TOTHTOT
 * @date: 2023年3月12日20:01:20
 */
uint8_t dht11_read_bit(void)
{
    uint8_t retry = 0;
    while (DHT11_INPUT_DQ && retry < 100)
    { // 等待变为低电平
        retry++;
        delay_us(1);
    }
    retry = 0;
    while (!DHT11_INPUT_DQ && retry < 100)
    { // 等待变为高电平
        retry++;
        delay_us(1);
    }
    delay_us(40); // 等待40us
    if (DHT11_INPUT_DQ)
        return 1;
    else
        return 0;
}

/**
 * @name: dht11_read_byte
 * @msg: dht11 读取一个字节
 * @return {*} 读取到的字节
 * @author: TOTHTOT
 * @date: 2023年3月12日20:00:51
 */
uint8_t dht11_read_byte(void)
{
    uint8_t i, dat;
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= dht11_read_bit();
    }
    return dat;
}

/**
 * @name: dht11_read_data
 * @msg: dht11 读取数据
 * @param {uint16_t} *temp 温度
 * @param {uint16_t} *humi 湿度
 * @return {*} == 1 失败; == 0 成功;
 * @author: TOTHTOT
 * @date: 2023年3月12日20:04:37
 */
uint8_t dht11_read_data(uint16_t *temp, uint16_t *humi)
{
    uint8_t buf[5];
    uint8_t i;
    dht11_reset();
    if (dht11_check(&g_dht11_device_st) == 0)
    {
        for (i = 0; i < 5; i++)
        {
            buf[i] = dht11_read_byte();
        }
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            *humi = (buf[0] << 8) + buf[1];
            *temp = (buf[2] << 8) + buf[3];
        }
    }
    else
        return 1;
    return 0;
}

/**
 * @name: dht11_init
 * @msg: dht11 的初始化
 * @return {*} == 1, 失败; == 0 成功
 * @author: TOTHTOT
 * @date: 2023年3月12日20:05:53
 */
uint8_t dht11_init(void)
{
    dht11_reset();
    
    g_dht11_device_st.dht11_state_em = DHT11_NOT_FIND;
    g_dht11_device_st.get_dat11_data = dht11_read_data;

    return dht11_check(&g_dht11_device_st);
}
