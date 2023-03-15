/*
 * @Description: oled 的驱动代码, 使用硬件i2c
 * @Author: TOTHTOT
 * @Date: 2023-03-14 19:06:07
 * @LastEditTime: 2023-03-15 11:21:18
 * @LastEditors: TOTHTOT
 * @FilePath: \MDK-ARMe:\JieDan\stm32_agricultural_irrigation\CODE\STM32F103C8T6(HAL+FreeRTOS)\HARDWARE\OLED\oled.h
 */

#ifndef __OLED_H_
#define __OLED_H_

#include "stdlib.h"
#include "main.h"
#include "dht11.h"
#include "cmsis_os.h"
#include "delay.h"

#define OLED_ADDER 0x78

/* oled 显示结构体, 对屏幕的所有操作都封装在其中, 不需要去操作底层函数, 在创建变量时要对其进行初始化 */
struct oled_device
{
/* 使用的I2C模式, DMA或者硬件I2C */
#define I2C_OLED_HARDWARE
    // #define I2C_OLED_HARDWARE_DMA 1

#define USE_OLED_GRAM 0 // 使用oled显存
#if USE_OLED_GRAM
    uint8_t oled_gram[8][128]; // 定义模拟显存
#endif                         /* USE_OLED_GRAM */

    /* 以后使用这个库只需要改动这个结构图体和 data_pos 结构体 */
    struct oled_show_data
    {
        struct dht11_device *dht11_p; // dht11的数据结构体, 更新数据时使用
        uint8_t char_size;            // 字体大小
    } oled_data_st;

#ifdef SYSTEM_SUPPORT_OS
    osSemaphoreId_t *en_refresh_screen_p; // 更新屏幕, 使用FreeRTOS时使用
    /* 使用操作系统时 oled线程会根据这个状态机进行不同类型的操作 */
    enum os_refresh_screen_type
    {
        SCREEN_NONE,                   // 默认状态
        SCREEN_CLREAR,                 // 清屏
        SCREEN_REFRESH_MAIN_PAGE,      // 刷新主页面
        SCREEN_REFRESH_MAIN_PAGE_DATA, // 刷新数据
        SCREEN_TOTAL_TYPE              // 总共状态
    } oled_refresh_type_em;
#endif /* SYSTEM_SUPPORT_OS */

    /* 数据显示的位置, 在主页面里赋值 */
    struct data_pos
    {
        uint8_t x;
        uint8_t y;
    } wd_pos_st, sd_pos_st, gz_pos_st;

    /* 外部可以调用的函数 */
    void (*oled_show_char)(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);     // 显示一个字符
    void (*oled_show_string)(uint8_t, uint8_t, const uint8_t *, uint8_t);    // 显示一个字符串
    void (*oled_show_num)(uint8_t, uint8_t, unsigned int, uint8_t, uint8_t); // 显示一个数字
    void (*oled_clear_screen)(void);                                         // 清屏
    void (*updata_main_page)(struct oled_device *);                          // 更新主页面
    void (*updata_main_page_data)(struct oled_device *);                     // 更新主页面数据
};
extern struct oled_device g_oled_device_st;

void OLED_Init(void);

#endif /* __OLED_H_ */
