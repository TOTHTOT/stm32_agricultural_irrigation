/*
 * @Description: dht11 驱动文件
 * @Author: TOTHTOT
 * @Date: 2023-03-12 19:34:17
 * @LastEditTime: 2023-03-12 20:30:35
 * @LastEditors: TOTHTOT
 * @FilePath: \MDK-ARMe:\JieDan\stm32_agricultural_irrigation\CODE\STM32F103C8T6(HAL+FreeRTOS)\HARDWARE\DHT11\dht11.h
 */
#ifndef __DHT11_H__
#define __DHT11_H__

#include "main.h"

struct dht11_device
{
    enum dht11_state
    {
        DHT11_NOT_FIND, // 没找到设备, 默认状态
        DHT11_ONLINE,   // 设备在线, 在线才获取数据
        DHT11_OFFLINE,  // 设备掉线, 连接后被拔出后进入此装态
        DHT11_TOTAL_STATE
    } dht11_state_em;                                  // dht11 的状态
#define DHT11_RETRY_TIMES 100                          // 检测dht11 是否在线次数
    uint16_t dht11_data_temprature, dht11_data_humidity;   // dht11 温度和湿度
    uint8_t (*get_dat11_data)(uint16_t *, uint16_t *); // 获取数据函数指针, 外部调用, 返回值是执行结果
};

uint8_t dht11_init(void);
extern struct dht11_device g_dht11_device_st;    

#endif /* __DHT11_H__ */

