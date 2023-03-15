/*
 * @Description: oled 驱动程序 使用硬件i2c
 * @Author: TOTHTOT
 * @Date: 2023-03-14 19:06:07
 * @LastEditTime: 2023-03-15 19:40:16
 * @LastEditors: TOTHTOT
 * @FilePath: \MDK-ARMe:\JieDan\stm32_agricultural_irrigation\CODE\STM32F103C8T6(HAL+FreeRTOS)\HARDWARE\OLED\oled.c
 */

#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"
#include "delay.h"
#include "main.h"
#include "i2c.h"
#include "string.h"
/* 全局变量 */
struct oled_device g_oled_device_st = {0};

/**
 * @name: OLED_WR_CMD
 * @msg: 写命令, 最底层的函数, 修改其就能实现硬件或者软件i2c
 * @param {uint8_t} cmd 指令
 * @return {*} 无
 * @author: TOTHTOT
 * @date: 2023年3月14日23:01:49
 */
void OLED_WR_CMD(uint8_t cmd)
{
#ifdef I2C_OLED_HARDWARE
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDER, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 0x100);
#elif I2C_OLED_HARDWARE_DMA
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
    {
    };
    HAL_I2C_Mem_Write_DMA(&hi2c1, OLED_ADDER, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1);
#endif
}

/**
 * @name: OLED_WR_DATA
 * @msg: 写数据, 最底层的函数, 修改其就能实现硬件或者软件i2c
 * @param {uint8_t} data 数据
 * @return {*} 无
 * @author: TOTHTOT
 * @date: 2023年3月14日23:02:25
 */
void OLED_WR_DATA(uint8_t data)
{
#ifdef I2C_OLED_HARDWARE
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDER, 0x40, I2C_MEMADD_SIZE_8BIT, &data, 1, 0x100);
#elif I2C_OLED_HARDWARE_DMA
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
    {
    };
    HAL_I2C_Mem_Write_DMA(&hi2c1, OLED_ADDER, 0x40, I2C_MEMADD_SIZE_8BIT, &data, 1);
#endif
}

// 坐标设置
/**
 * @name:
 * @msg:
 * @param {uint8_t} x
 * @param {uint8_t} y
 * @return {*}
 * @author: TOTHTOT
 * @date:
 */
void OLED_Set_Pos(uint8_t x, uint8_t y)
{
    OLED_WR_CMD(0xb0 + y);
    OLED_WR_CMD(((x & 0xf0) >> 4) | 0x10);
    OLED_WR_CMD(x & 0x0f);
}

// 开启OLED显示
void OLED_Display_On(void)
{
    OLED_WR_CMD(0X8D); // SET DCDC命令
    OLED_WR_CMD(0X14); // DCDC ON
    OLED_WR_CMD(0XAF); // DISPLAY ON
}

// 关闭OLED显示
void OLED_Display_Off(void)
{
    OLED_WR_CMD(0X8D); // SET DCDC命令
    OLED_WR_CMD(0X10); // DCDC OFF
    OLED_WR_CMD(0XAE); // DISPLAY OFF
}

/**
 * @name:OLED_DrawPoint
 * @msg: oled 在显存中画点
 * @param {uint8_t} x   x坐标
 * @param {uint8_t} y   y坐标
 * @param {uint8_t} t   == 1, 点亮这个点; == 0, 熄灭这个点
 * @return {*} 无
 * @author: TOTHTOT
 * @date: 2023年3月15日10:58:53
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
#if USE_OLED_GRAM
    uint8_t pos, bx, temp = 0;
    if (x > 127 || y > 63)
        return; // 超出范围了.
    pos = y / 8;
    bx = y % 8;
    temp = 1 << bx;
    if (t)
        g_oled_device_st.oled_gram[pos][x] |= temp;
    else
        g_oled_device_st.oled_gram[pos][x] &= ~temp;
#endif /* USE_OLED_GRAM */
}

void OLED_Refresh_Gram(void)
{
	#if USE_OLED_GRAM
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_CMD(0xb0 + i); // 设置页地址（0~7）
        OLED_WR_CMD(0x00);     // 设置显示位置—列低地址
        OLED_WR_CMD(0x10);     // 设置显示位置—列高地址
        for (n = 0; n < 128; n++)
            OLED_WR_DATA(g_oled_device_st.oled_gram[i][n]);
    }
	#endif /* USE_OLED_GRAM */
}

// 清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
void OLED_Clear()
{

#if USE_OLED_GRAM
    uint8_t i, n;
    memset(g_oled_device_st.oled_gram, 0, sizeof(g_oled_device_st.oled_gram));
    OLED_Refresh_Gram();
#else
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_CMD(0xb0 + i);
        OLED_WR_CMD(0x00);
        OLED_WR_CMD(0x10);
        for (n = 0; n < 128; n++)
            OLED_WR_DATA(0x00);
    }
#endif /* USE_OLED_GRAM */
}

/**
 * @name: OLED_ShowChar
 * @msg: oled 显示一个字符
 * @param {uint8_t} x   x坐标
 * @param {uint8_t} y   y坐标
 * @param {uint8_t} chr 要显示的字符
 * @param {uint8_t} Char_Size   字体大小
 * @param {uint8_t} mode oled 点的亮灭
 * @return {*}  无
 * @author: TOTHTOT
 * @date: 2023年3月14日19:28:44
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size, uint8_t mode)
{
#if USE_OLED_GRAM
    uint8_t temp, t, t1;
    uint8_t x0 = x;
    uint8_t csize = (Char_Size / 8 + ((Char_Size % 8) ? 1 : 0)) * (Char_Size / 2); // 得到字体一个字符对应点阵集所占的字节数
    chr = chr - ' ';                                                               // 得到偏移后的值
    if (Char_Size == 8)
        Char_Size = 5;

    if (x > 128 - 1)
    {
        x = 0;
        y = y + 2;
    }
    if (Char_Size == 16)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            temp = F8X16[chr * 16 + t]; // 调用1608字体
            for (t1 = 0; t1 < 8; t1++)
            {
                if (temp & 0x80)
                    OLED_DrawPoint(x, y, mode);
                else
                    OLED_DrawPoint(x, y, !mode);
                temp <<= 1;
                x++;
            }
        }
        y++;
        x = x0;
        for (uint8_t i = 0; i < 8; i++)
        {
            temp = F8X16[chr * 16 + t + 8]; // 调用1608字体
            for (t1 = 0; t1 < 8; t1++)
            {
                if (temp & 0x80)
                    OLED_DrawPoint(x, y, mode);
                else
                    OLED_DrawPoint(x, y, !mode);
                temp <<= 1;
                x++;
            }
        }
    }
    OLED_Refresh_Gram();
/*     else
    {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WR_DATA(F6x8[c][i]);
    } */
#else
    unsigned char c = 0, i = 0;
    c = chr - ' '; // 得到偏移后的值
    if (x > 128 - 1)
    {
        x = 0;
        y = y + 2;
    }
    if (Char_Size == 16)
    {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 8; i++)
            OLED_WR_DATA(F8X16[c * 16 + i]);
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 8; i++)
            OLED_WR_DATA(F8X16[c * 16 + i + 8]);
    }
    else
    {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WR_DATA(F6x8[c][i]);
    }
#endif
}

/**
 * @name: OLED_ShowString
 * @msg: olde 显示一串字符串
 * @param {uint8_t} x   x坐标
 * @param {uint8_t} y   y坐标
 * @param {uint8_t} *chr    字符串
 * @param {uint8_t} Char_Size   字体大小
 * @return {*}  无
 * @author: TOTHTOT
 * @date: 2023年3月14日19:29:27
 */
void OLED_ShowString(uint8_t x, uint8_t y, const uint8_t *chr, uint8_t Char_Size)
{
    unsigned char j = 0;
    while (chr[j] != '\0')
    {
        OLED_ShowChar(x, y, chr[j], Char_Size, 1);
        x += 8;
        if (x > 120)
        {
            x = 0;
            y += 2;
        }
        j++;
    }
}

// m^n函数
unsigned int oled_pow(uint8_t m, uint8_t n)
{
    unsigned int result = 1;
    while (n--)
        result *= m;
    return result;
}

// 显示2个数字
// x,y :起点坐标
// len :数字的位数
// size:字体大小
// mode:模式	0,填充模式;1,叠加模式
// num:数值(0~4294967295);
/**
 * @name:
 * @msg:
 * @param {uint8_t} x
 * @param {uint8_t} y
 * @param {unsigned int} num
 * @param {uint8_t} len
 * @param {uint8_t} size2
 * @return {*}
 * @author: TOTHTOT
 * @date:
 */
void OLED_ShowNum(uint8_t x, uint8_t y, unsigned int num, uint8_t len, uint8_t size2)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                OLED_ShowChar(x + (size2 / 2) * t, y, ' ', size2, 1);
                continue;
            }
            else
                enshow = 1;
        }
        OLED_ShowChar(x + (size2 / 2) * t, y, temp + '0', size2, 1);
    }
}

/**
 * @name: OLED_ShowCHinese
 * @msg: olde 显示一个汉字
 * @param {uint8_t} x   x坐标
 * @param {uint8_t} y   y坐标
 * @param {uint8_t} no  第几个汉字, 从汉字库中提取
 * @return {*}  无
 * @author: TOTHTOT
 * @date: 2023年3月14日19:31:55
 */
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no)
{
    uint8_t t, adder = 0;
    OLED_Set_Pos(x, y);
    for (t = 0; t < 16; t++)
    {
        OLED_WR_DATA(Hzk[2 * no][t]);
        adder += 1;
    }
    OLED_Set_Pos(x, y + 1);
    for (t = 0; t < 16; t++)
    {
        OLED_WR_DATA(Hzk[2 * no + 1][t]);
        adder += 1;
    }
}

/**
 * @name: OLED_ver_scroll
 * @msg: 水平滚动
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年3月14日19:32:26
 */
void OLED_ver_scroll()
{
    OLED_WR_CMD(0x2e); // 关滚动
    OLED_WR_CMD(0x27); // 29向右，2a向左
    OLED_WR_CMD(0x00); // A:空字节
    OLED_WR_CMD(0x02); // B:水平起始页
    OLED_WR_CMD(0x07); // C:水平滚动速度
    OLED_WR_CMD(0x03); // D:水平结束页
    OLED_WR_CMD(0x00); // E:每次垂直滚动位移
    OLED_WR_CMD(0xFF); // E:每次垂直滚动位移
    OLED_WR_CMD(0x2f); // 开滚动
}

/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
/**
 * @name: OLED_DrawBMP
 * @msg: 显示一张图片
 * @param {unsigned char} x0    x起始坐标
 * @param {unsigned char} y0    y起始坐标
 * @param {unsigned char} x1    x结束坐标
 * @param {unsigned char} y1    y结束坐标
 * @param {unsigned char} BMP   图片
 * @return {*}  无
 * @author: TOTHTOT
 * @date: 2023年3月14日19:33:30
 */
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
    unsigned int j = 0;
    unsigned char x, y;

    if (y1 % 8 == 0)
        y = y1 / 8;
    else
        y = y1 / 8 + 1;
    for (y = y0; y < y1; y++)
    {
        OLED_Set_Pos(x0, y);
        for (x = x0; x < x1; x++)
        {
            OLED_WR_DATA(BMP[j++]);
        }
    }
}

/**
 * @name: main_page_data
 * @msg: 主页面得数据更新
 * @param {oled_device} *oled_st olde 结构体
 * @return {*}  无
 * @author: TOTHTOT
 * @date:   2023年3月14日20:27:51
 */
void main_page_data(struct oled_device *oled_st)
{
    /* 显示温湿度数据 */
    OLED_ShowNum(oled_st->wd_pos_st.x, oled_st->wd_pos_st.y, oled_st->oled_data_st.dht11_p->dht11_data_temprature, 3, oled_st->oled_data_st.char_size);
    OLED_ShowNum(oled_st->sd_pos_st.x, oled_st->sd_pos_st.y, oled_st->oled_data_st.dht11_p->dht11_data_humidity, 3, oled_st->oled_data_st.char_size);
    /* 显示光照数据 */
    // OLED_ShowNum(oled_st->sd_pos_st.x, oled_st->sd_pos_st.y, oled_st->oled_data_st.dht11_p->dht11_data_humidity, 3, oled_st->oled_data_st.char_size);
    OLED_Refresh_Gram();
}

/**
 * @name: main_page
 * @msg: 显示主页面内容
 * @param {oled_device} *oled_st    oled 结构体
 * @return {*}  无
 * @author: TOTHTOT
 * @date: 2023年3月14日20:27:24
 */
void main_page(struct oled_device *oled_st)
{
    /* 主页面显示的内容 */
    OLED_ShowString(0, 0, "WD:", 16);
    oled_st->wd_pos_st.x = 30;
    oled_st->wd_pos_st.y = 0;

    OLED_ShowString(64, 0, "SD:", 16);
    oled_st->sd_pos_st.x = 64 + 30;
    oled_st->sd_pos_st.y = 0;

    OLED_ShowString(0, 2, "GZ:", 16);
    oled_st->gz_pos_st.x = 30;
    oled_st->gz_pos_st.y = 2;

    main_page_data(oled_st);
    OLED_Refresh_Gram();
}

/**
 * @name: OLED_Init
 * @msg: 初始化 oled
 * @return {*}
 * @author: TOTHTOT
 * @date: 2023年3月14日19:34:09
 */
void OLED_Init(void)
{
    // 不注释RTOS报错
    delay_xms(400);
    OLED_WR_CMD(0xAE); //--turn off oled panel
    OLED_WR_CMD(0x00); //---set low column address
    OLED_WR_CMD(0x10); //---set high column address
    OLED_WR_CMD(0x40); //--set start line address  Set Mapping RAM Display Start Line
    OLED_WR_CMD(0x81); //--set contrast control register
    OLED_WR_CMD(0xCF); // Set SEG Output Current Brightness
    OLED_WR_CMD(0xA1); //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    OLED_WR_CMD(0xC8); // Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    OLED_WR_CMD(0xA6); //--set normal display
    OLED_WR_CMD(0xA8); //--set multiplex ratio(1 to 64)
    OLED_WR_CMD(0x3f); //--1/64 duty
    OLED_WR_CMD(0xD3); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_CMD(0x00); //-not offset
    OLED_WR_CMD(0xd5); //--set display clock divide ratio/oscillator frequency
    OLED_WR_CMD(0x80); //--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_CMD(0xD9); //--set pre-charge period
    OLED_WR_CMD(0xF1); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_CMD(0xDA); //--set com pins hardware configuration
    OLED_WR_CMD(0x12);
    OLED_WR_CMD(0xDB); //--set vcomh
    OLED_WR_CMD(0x40); // Set VCOM Deselect Level
    OLED_WR_CMD(0x20); //-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_CMD(0x02); //
    OLED_WR_CMD(0x8D); //--set Charge Pump enable/disable
    OLED_WR_CMD(0x14); //--set(0x10) disable
    OLED_WR_CMD(0xA4); // Disable Entire Display On (0xa4/0xa5)
    OLED_WR_CMD(0xA6); // Disable Inverse Display On (0xa6/a7)
    OLED_Clear();
    OLED_WR_CMD(0xAF);

    extern osSemaphoreId_t en_refresh_screenHandle;
    /* oled 初始化时更新设置相关数据 */
    g_oled_device_st.oled_show_char = OLED_ShowChar;
    g_oled_device_st.oled_show_num = OLED_ShowNum;
    g_oled_device_st.oled_show_string = OLED_ShowString;
    g_oled_device_st.oled_clear_screen = OLED_Clear;
    g_oled_device_st.updata_main_page = main_page;
    g_oled_device_st.updata_main_page_data = main_page_data;
    g_oled_device_st.en_refresh_screen_p = &en_refresh_screenHandle;
    g_oled_device_st.oled_data_st.dht11_p = &g_dht11_device_st;
    g_oled_device_st.oled_data_st.char_size = 16;
    g_oled_device_st.oled_refresh_type_em = SCREEN_NONE;
}
